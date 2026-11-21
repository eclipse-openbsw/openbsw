# *******************************************************************************
# Copyright (c) 2026 KPIT Technologies Ltd.
#
# This program and the accompanying materials are made available under the
# terms of the Apache License Version 2.0 which is available at
# https://www.apache.org/licenses/LICENSE-2.0
#
# SPDX-License-Identifier: Apache-2.0
# *******************************************************************************

"""flync model -> the deployment structure the C++ templates already consume.

The output is exactly what a hand-written deployment.yaml would contain, so
it goes through the same schema validation and the same templates. Nothing
downstream knows flync exists.

What comes from where:

    service id, member ids and kinds   service file
    member payload types               the members' inline datatypes
    C++ namespace and class            the service file's own stem
    roles, instance ids, placement     the apps' provider and consumer refs
    RPC sizing                         the middleware configuration, per service
    clusters, pools, pool per service  the middleware configuration
    connections, cores, capacities     derived here, never stated

Derived rather than read is the point: a cluster connection exists because
one cluster provides a service another consumes, so stating it again would
be a second place to get it wrong.
"""

from .cpp_types import (
    TypeRegistry,
    class_name,
    namespace_from_stem,
    service_class_from_stem,
    variable_name,
)
from .model import ModelError, RPC_KEYS


def translate(model) -> dict:
    """Build the deployment dict from a loaded FlyncModel."""
    allocators = _allocators(model)
    clusters, cluster_index = _clusters(model)

    placements = model.placements()
    services = [
        _service(model, name, placements[name], allocators, cluster_index)
        for name in sorted(placements, key=lambda n: int(model.services[n]["id"]))
    ]

    return {
        "clusters": clusters,
        "allocators": allocators,
        "services": services,
        "connections": _connections(services, placements, model, cluster_index),
        "cores": _cores(model, cluster_index),
    }


# ---------------------------------------------------------------------------
# overlay sections
# ---------------------------------------------------------------------------


def _allocators(model) -> list[dict]:
    """Overlay allocator profiles, in the shape the deployment schema wants.

    Two spelling differences, both deliberate: `chunk_size` is `ChunkSize`
    there, and a profile id that is not already a bare identifier is CamelCased,
    because the generator builds a function name out of it.

    One profile must be called `default`: the allocator-selector template emits
    `shm::getDefaultAllocator()` in its fallback branch unconditionally, so
    without it the generated selector does not compile.
    """
    allocators = [
        {
            "name": _allocator_name(profile["profile_id"]),
            "pools": [
                {"NumberOfElements": int(pool["elements"]), "ChunkSize": int(pool["chunk_size"])}
                for pool in profile.get("pools") or []
            ],
        }
        for profile in model.overlay["allocator_profiles"]
    ]

    if not any(allocator["name"] == "default" for allocator in allocators):
        raise ModelError(
            "no allocator profile is called 'default'; the generated allocator "
            "selector always falls back to shm::getDefaultAllocator(), so one "
            "profile must carry that id"
        )
    return allocators


def _allocator_name(profile_id: str) -> str:
    return profile_id if profile_id.isalnum() else service_class_from_stem(profile_id)


def _clusters(model) -> tuple[list[dict], dict]:
    """Emit clusters stripped of the overlay-only keys the schema forbids."""
    clusters = []
    index = {}
    for entry in model.overlay["clusters"]:
        cluster = {
            "name": entry["name"],
            "id": int(entry["id"]),
            "queueSize": int(entry["queue_size"]),
        }
        if cluster["name"] in index:
            raise ModelError(f"cluster '{cluster['name']}' is declared twice")
        clusters.append(cluster)
        index[cluster["name"]] = cluster
    return clusters, index


# ---------------------------------------------------------------------------
# services
# ---------------------------------------------------------------------------


def _service(model, service_name, placement, allocators, cluster_index) -> dict:
    flync_service = model.services[service_name]
    interactions = model.interactions_for(service_name)
    if not interactions:
        raise ModelError(f"service '{service_name}' declares no events, fields or methods")

    stem = flync_service["_stem"]
    namespace = namespace_from_stem(stem)
    name = service_class_from_stem(stem)

    profile = model.middleware_services[service_name]["allocator_profile"]
    allocator = _named(allocators, _allocator_name(profile))
    registry = TypeRegistry(model.datatypes)

    events = _events(flync_service, interactions, registry)
    attributes = _attributes(flync_service, interactions, registry)
    methods, fire_and_forget = _methods(flync_service, interactions, registry)

    return {
        "namespace": namespace,
        "name": name,
        "id": int(flync_service["id"]),
        "allocator": allocator,
        # Registered while walking the members above, so this must come after.
        "typedefs": registry.typedefs(),
        "datatypes": registry.structs(),
        "attributes": attributes,
        "methods": methods,
        "methods_fire_and_forget": fire_and_forget,
        "events": events,
        "headers": {
            "common": f"{namespace}/{name}Common.h",
            "proxy": f"{namespace}/{name}Proxy.h",
            "skeleton": f"{namespace}/{name}Skeleton.h",
            # Required only in mock mode, and harmless otherwise.
            "proxy_mock": f"{namespace}/{name}ProxyMock.h",
            "skeleton_mock": f"{namespace}/{name}SkeletonMock.h",
        },
        "deployment": _deployment(placement, cluster_index),
    }


def _events(flync_service, interactions, registry) -> list[dict]:
    events = []
    for event in flync_service.get("events") or []:
        name = event["name"]
        events.append(
            {
                "name": class_name(name),
                "variable": variable_name(name),
                "id": int(event["id"]),
                "type": _payload_type(name, interactions, registry, suffix="Value"),
            }
        )
    return events


def _attributes(flync_service, interactions, registry) -> list[dict]:
    """A flync field becomes one middleware attribute.

    readonly follows from the absence of a setter_id, and noSubscriptions from
    the absence of a notifier_id -- flync states neither as a flag, and a flag
    would be a second place for them to disagree.

    setAsMethod follows from the presence of a setter_id: a SOME/IP field setter
    is request/response on the wire, carrying the value applied, so the skeleton
    needs a response buffer for it. Without the flag the generator emits a
    setter that returns void and cannot answer.

    A getter or a setter expects an answer, so its interface states the RPC
    sizing; a field that only notifies states none.
    """
    attributes = []
    for field in flync_service.get("fields") or []:
        name = field["name"]
        rpc = _rpc(
            interactions.get(name), name, required=("getter_id" in field or "setter_id" in field)
        )
        attribute = {
            "name": class_name(name),
            "variable": variable_name(name),
            "type": _payload_type(name, interactions, registry, suffix="Value"),
            "readonly": "setter_id" not in field,
            "noSubscriptions": "notifier_id" not in field,
            "setAsMethod": "setter_id" in field,
            "consumer_request_limit": rpc["consumer_request_limit"],
            "provider_request_limit": rpc["provider_request_limit"],
        }
        if "notifier_id" in field:
            attribute["event"] = {"id": int(field["notifier_id"])}
        if "getter_id" in field:
            attribute["getter"] = {"id": int(field["getter_id"]), "timeout": rpc["request_timeout"]}
        if "setter_id" in field:
            attribute["setter"] = {"id": int(field["setter_id"]), "timeout": rpc["request_timeout"]}
        attributes.append(attribute)
    return attributes


#: Sizing for a member that expects no answer: a fire-and-forget method, or a
#: field that only notifies. One in flight, and no timeout to wait out.
_NO_RPC = {"request_timeout": 0, "consumer_request_limit": 1, "provider_request_limit": 1}


def _rpc(interaction, member_name, required) -> dict:
    """The RPC sizing an interface states, read off one of its interactions."""
    if not required:
        return dict(_NO_RPC)

    missing = [key for key in RPC_KEYS if (interaction or {}).get(key) is None]
    if missing:
        raise ModelError(
            f"'{member_name}' expects a response, but its interface states no "
            f"{', '.join(missing)}"
        )
    return {key: int(interaction[key]) for key in RPC_KEYS}


def _methods(flync_service, interactions, registry) -> tuple[list, list]:
    methods, fire_and_forget = [], []
    for method in flync_service.get("methods") or []:
        name = method["name"]
        interaction = interactions.get(name)
        if interaction is None:
            raise ModelError(
                f"method '{name}' of '{flync_service['name']}' has no parameter description"
            )

        is_fire_and_forget = method.get("type") == "fire_and_forget"
        rpc = _rpc(interaction, name, required=not is_fire_and_forget)
        entry = {
            "name": class_name(name),
            "variable": variable_name(name),
            "id": int(method["id"]),
            "timeout": rpc["request_timeout"],
            "consumer_request_limit": rpc["consumer_request_limit"],
            "provider_request_limit": rpc["provider_request_limit"],
        }

        request = _parameter(name, interaction.get("input_parameters"), registry, "Request")
        if request is not None:
            entry["input_param"] = request

        if is_fire_and_forget:
            fire_and_forget.append(entry)
            continue

        response = _parameter(name, interaction.get("output_parameters"), registry, "Response")
        if response is not None:
            entry["output_param"] = response
        methods.append(entry)

    return methods, fire_and_forget


def _deployment(placement, cluster_index) -> dict:
    """One entry per cluster, whichever roles it plays for the service.

    The instances database is generated per providing cluster, and a proxy
    finds its provider by instance id alone, so an instance provided twice is
    ambiguous.
    """
    by_cluster: dict[str, dict] = {}

    def _entry(cluster: str) -> dict:
        if cluster not in by_cluster:
            by_cluster[cluster] = {
                "cluster": cluster_index[cluster],
                "consumers": 0,
                "providers": {"instance_ids": []},
            }
        return by_cluster[cluster]

    provided: dict[int, str] = {}
    for cluster, instance_id in sorted(placement["providers"]):
        if instance_id in provided:
            raise ModelError(
                f"instance {instance_id} is provided on both '{provided[instance_id]}' "
                f"and '{cluster}'"
            )
        provided[instance_id] = cluster
        _entry(cluster)["providers"]["instance_ids"].append(instance_id)
    for cluster, _ in sorted(placement["consumers"]):
        _entry(cluster)["consumers"] += 1

    return {
        "instance_ids": sorted(placement["instance_ids"]),
        "clusters": list(by_cluster.values()),
    }


# ---------------------------------------------------------------------------
# payload types
# ---------------------------------------------------------------------------


def _payload_type(member_name, interactions, registry, suffix) -> str:
    """The C++ type of a pub-sub member (an event, or a field's value).

    One dataref is the type itself; several become a struct, because the
    middleware moves exactly one object per message.
    """
    interaction = interactions.get(member_name)
    if interaction is None:
        raise ModelError(f"'{member_name}' has no parameter description")

    datarefs = interaction.get("dataref") or []
    if isinstance(datarefs, str):
        datarefs = [datarefs]
    if not datarefs:
        raise ModelError(f"interaction '{member_name}' names no dataref")
    if len(datarefs) == 1:
        return registry.resolve(datarefs[0])

    parameters = [
        {"name": _member_field_name(dataref, index), "type": registry.resolve(dataref)}
        for index, dataref in enumerate(datarefs)
    ]
    return registry.struct(class_name(member_name) + suffix, parameters)


#: Name given to a synthesised struct parameter, per direction. Not "response":
#: the generated skeleton already has a parameter of that name, and two
#: parameters sharing one name is a redeclaration, not an overload.
_PARAMETER_NAME = {"Request": "request", "Response": "result"}


def _parameter(method_name, parameters, registry, suffix) -> dict | None:
    """One method direction: absent, a single value, or a synthesised struct."""
    parameters = parameters or []
    if not parameters:
        return None
    if len(parameters) == 1:
        only = parameters[0]
        name = only["name"]
        # Same collision as above, for a name the model chose itself.
        if name == "response":
            name = _PARAMETER_NAME[suffix]
        return {"name": name, "type": registry.resolve(only["dataref"])}

    fields = [
        {"name": parameter["name"], "type": registry.resolve(parameter["dataref"])}
        for parameter in parameters
    ]
    struct = registry.struct(class_name(method_name) + suffix, fields)
    return {"name": _PARAMETER_NAME[suffix], "type": struct}


def _member_field_name(dataref: str, index: int) -> str:
    """A pub-sub member's datarefs are unnamed, so name them by position.

    A method's parameters carry their own names and never come through here.
    """
    return f"field{index}"


def _named(entries, name):
    for entry in entries:
        if entry["name"] == name:
            return entry
    raise ModelError(f"no allocator profile '{name}'")


# ---------------------------------------------------------------------------
# derived sections
# ---------------------------------------------------------------------------


def _connections(services, placements, model, cluster_index) -> list[dict]:
    """One connection per ordered cluster pair, carrying every service on it.

    A consumer is paired with the cluster providing the instance it consumes,
    not with every provider of the service. Both directions are emitted: the
    skeleton registers on provider->consumer, the proxy on consumer->provider.

    Capacity follows the pairing: one skeleton per instance provided across the
    pair, one proxy per consuming cluster.
    """
    by_service = {int(service["id"]): service for service in services}
    skeletons: dict[tuple[str, str], dict[int, set]] = {}
    proxies: dict[tuple[str, str], dict[int, int]] = {}

    for service_name, placement in sorted(placements.items()):
        service_id = int(model.services[service_name]["id"])
        provider_of = {instance_id: cluster for cluster, instance_id in placement["providers"]}
        for consumer, instance_id in sorted(placement["consumers"]):
            provider = provider_of.get(instance_id)
            if provider is None:
                raise ModelError(
                    f"'{consumer}' consumes instance {instance_id} of service "
                    f"0x{service_id:04x}, which no cluster provides"
                )
            if provider == consumer:
                raise ModelError(
                    f"'{consumer}' both provides and consumes instance {instance_id} of "
                    f"service 0x{service_id:04x}; there is no connection within a cluster"
                )
            skeletons.setdefault((provider, consumer), {}).setdefault(service_id, set()).add(
                instance_id
            )
            per_pair = proxies.setdefault((consumer, provider), {})
            per_pair[service_id] = per_pair.get(service_id, 0) + 1

    def _order(pair: tuple[str, str]) -> tuple[int, int]:
        return (cluster_index[pair[0]]["id"], cluster_index[pair[1]]["id"])

    connections = []
    for source, target in sorted(set(skeletons) | set(proxies), key=_order):
        connections.append(
            {
                "source_cluster": cluster_index[source],
                "target_cluster": cluster_index[target],
                "proxies": [
                    _endpoint(by_service[service_id], count)
                    for service_id, count in sorted(proxies.get((source, target), {}).items())
                ],
                "skeletons": [
                    _endpoint(by_service[service_id], len(instances))
                    for service_id, instances in sorted(
                        skeletons.get((source, target), {}).items()
                    )
                ],
            }
        )
    return connections


def _endpoint(service: dict, max_instances: int) -> dict:
    """A connection endpoint is the service plus its transceiver capacity.

    The schema forbids unknown keys here, so only what it accepts is copied --
    the same set a `<<:` merge of the service anchor would have produced.
    """
    allowed = (
        "namespace", "name", "filename", "id", "allocator", "enums", "datatypes",
        "typedefs", "attributes", "methods", "methods_fire_and_forget", "events",
        "headers", "deployment",
    )
    endpoint = {key: service[key] for key in allowed if key in service}
    endpoint["max_instances"] = max_instances
    return endpoint


def _cores(model, cluster_index) -> list[dict]:
    """Group clusters by the controller they run on.

    A cluster pair split across two controllers is what makes its connection
    a cross-core one, which is why the middleware needs this grouping at all.
    """
    controllers = model.cluster_controllers()

    cores: dict[str, list] = {}
    for entry in model.overlay["clusters"]:
        controller = controllers.get(entry["name"])
        if controller is None:
            raise ModelError(
                f"no app places a service on cluster '{entry['name']}', so nothing "
                "says which controller runs it"
            )
        cores.setdefault(controller, []).append(cluster_index[entry["name"]])

    return [{"name": controller, "clusters": clusters} for controller, clusters in cores.items()]
