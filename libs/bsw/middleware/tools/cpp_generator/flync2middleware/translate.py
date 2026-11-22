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
    components, ports, placement       the apps' provider and consumer refs
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
    cpp_identifier,
    namespace_from_stem,
    service_class_from_stem,
)
from .model import ModelError, RPC_KEYS


def translate(model) -> dict:
    """Build the deployment dict from a loaded FlyncModel."""
    allocators = _allocators(model)
    placements = model.placements()

    services, type_modules = [], []
    for name in sorted(placements, key=lambda n: int(model.services[n]["id"])):
        service, type_module = _service(model, name)
        services.append(service)
        if type_module is not None:
            type_modules.append(type_module)

    components, instances = _components(model, placements)
    clusters, cluster_index = _clusters(model, instances)

    deployment = {
        "components": components,
        "clusters": clusters,
        "allocators": allocators,
    }
    if type_modules:
        deployment["typeModules"] = type_modules
    deployment["services"] = services
    deployment["connections"] = _connections(placements, model, cluster_index)
    deployment["cores"] = _cores(model, cluster_index)
    return deployment


def instance_enumerator(cluster: str, app: str, service_class: str, instance_id: int) -> str:
    """The `InstanceId` enumerator the templates generate for one provided instance.

    The templates join cluster, component instance and port names; the port
    name carries the flync instance id, so each enumerator names one instance.
    """
    return _pascal(cluster) + _pascal(_instance_name(app)) + _pascal(
        _port_name(service_class, instance_id)
    )


def _pascal(name: str) -> str:
    return name[:1].upper() + name[1:]


def _instance_name(app: str) -> str:
    name = cpp_identifier(app)
    return name[:1].lower() + name[1:]


def _port_name(service_class: str, instance_id: int) -> str:
    return f"{service_class[:1].lower()}{service_class[1:]}{instance_id}"


# ---------------------------------------------------------------------------
# overlay sections
# ---------------------------------------------------------------------------


def _allocators(model) -> list[dict]:
    """Overlay allocator profiles, in the shape the deployment schema wants.

    `chunk_size` is spelled `ChunkSize` there, and a profile id that is not
    already a bare identifier is CamelCased, because the generator builds a
    function name out of it. The profile called `default` is the default
    allocator.
    """
    allocators = []
    for profile in model.overlay["allocator_profiles"]:
        allocator = {"name": _allocator_name(profile["profile_id"])}
        if allocator["name"] == "default":
            allocator["default"] = True
        allocator["pools"] = [
            {"NumberOfElements": int(pool["elements"]), "ChunkSize": int(pool["chunk_size"])}
            for pool in profile.get("pools") or []
        ]
        allocators.append(allocator)

    if not any(allocator.get("default") for allocator in allocators):
        raise ModelError(
            "no allocator profile is called 'default'; the deployment needs exactly one "
            "default allocator"
        )
    return allocators


def _allocator_name(profile_id: str) -> str:
    return profile_id if profile_id.isalnum() else service_class_from_stem(profile_id)


def _components(model, placements) -> tuple[list[dict], dict]:
    """One component per app, with one port per service instance it provides or consumes.

    Returns the components and, per cluster, the component instances it hosts.
    """
    ports: dict[str, dict] = {}
    cluster_of: dict[str, str] = {}
    provided: dict[tuple[str, int], str] = {}

    for service_name, placement in sorted(placements.items()):
        stem = model.services[service_name]["_stem"]
        service_class = service_class_from_stem(stem)
        for role, app, cluster, instance_id in sorted(placement["apps"]):
            if role == "providers":
                earlier = provided.setdefault((service_name, instance_id), cluster)
                if earlier != cluster:
                    raise ModelError(
                        f"instance {instance_id} of '{service_name}' is provided on both "
                        f"'{earlier}' and '{cluster}'"
                    )
            key = "providedPorts" if role == "providers" else "requiredPorts"
            ports.setdefault(app, {"providedPorts": [], "requiredPorts": []})[key].append(
                {
                    "name": _port_name(service_class, instance_id),
                    "interfaceName": service_class,
                    "interfaceNamespace": namespace_from_stem(stem),
                }
            )
            cluster_of[app] = cluster

    components = []
    instances: dict[str, list] = {}
    for app in sorted(ports):
        component = {"name": cpp_identifier(app)}
        component.update({key: value for key, value in ports[app].items() if value})
        components.append(component)
        instances.setdefault(cluster_of[app], []).append(
            {"instanceName": _instance_name(app), "component": component}
        )
    return components, instances


def _clusters(model, instances) -> tuple[list[dict], dict]:
    """Emit clusters stripped of the overlay-only keys the schema forbids."""
    clusters = []
    index = {}
    for entry in model.overlay["clusters"]:
        cluster = {
            "name": entry["name"],
            "id": int(entry["id"]),
            "queueSize": int(entry["queue_size"]),
        }
        if cluster["name"] in instances:
            cluster["componentInstances"] = instances[cluster["name"]]
        if cluster["name"] in index:
            raise ModelError(f"cluster '{cluster['name']}' is declared twice")
        clusters.append(cluster)
        index[cluster["name"]] = cluster
    return clusters, index


# ---------------------------------------------------------------------------
# services
# ---------------------------------------------------------------------------


def _service(model, service_name) -> tuple[dict, dict | None]:
    """The service and the type module holding the types its members use."""
    flync_service = model.services[service_name]
    interactions = model.interactions_for(service_name)
    if not interactions:
        raise ModelError(f"service '{service_name}' declares no events, fields or methods")

    stem = flync_service["_stem"]
    namespace = namespace_from_stem(stem)
    name = service_class_from_stem(stem)
    types_namespace = f"{namespace}::{name}"

    profile = model.middleware_services[service_name]["allocator_profile"]
    registry = TypeRegistry(model.datatypes)

    def qualify(cpp_type: str) -> str:
        # Declared types live in the type module's namespace, scalars are builtins.
        return f"{types_namespace}::{cpp_type}" if registry.declares(cpp_type) else cpp_type

    broadcasts = _broadcasts(flync_service, interactions, registry, qualify)
    attributes = _attributes(flync_service, interactions, registry, qualify)
    methods = _methods(flync_service, interactions, registry, qualify)

    # Registered while walking the members above, so this must come after.
    types = [
        {"typeName": "AliasTypeDef", "name": typedef["name"], "targetType": typedef["type"]}
        for typedef in registry.typedefs()
    ] + [
        {
            "typeName": "StructTypeDef",
            "name": struct["name"],
            "elements": [
                {"name": parameter["name"], "type": parameter["type"]}
                for parameter in struct["parameters"]
            ],
        }
        for struct in registry.structs()
    ]
    type_module = None
    service = {
        "namespace": namespace,
        "name": name,
        "deployment": {
            "ServiceID": int(flync_service["id"]),
            "MemoryAllocator": _allocator_name(profile),
        },
    }
    if types:
        type_module = {
            "namespace": types_namespace,
            "header": f"{namespace}/{name}Types.h",
            "types": types,
        }
        service["typeIncludes"] = [type_module["header"]]

    service.update(
        {
            "attributes": attributes,
            "methods": methods,
            "broadcasts": broadcasts,
            "commonInclude": f"{namespace}/{name}Common.h",
            "proxyInclude": f"{namespace}/{name}Proxy.h",
            "skeletonInclude": f"{namespace}/{name}Skeleton.h",
            # Required only in mock mode, and harmless otherwise.
            "proxyMockInclude": f"{namespace}/{name}ProxyMock.h",
            "skeletonMockInclude": f"{namespace}/{name}SkeletonMock.h",
        }
    )
    return service, type_module


def _broadcasts(flync_service, interactions, registry, qualify) -> list[dict]:
    broadcasts = []
    for event in flync_service.get("events") or []:
        name = event["name"]
        broadcasts.append(
            {
                "name": class_name(name),
                "type": qualify(_payload_type(name, interactions, registry, suffix="Value")),
                "deployment": {"NotificationID": int(event["id"])},
            }
        )
    return broadcasts


def _attributes(flync_service, interactions, registry, qualify) -> list[dict]:
    """A flync field becomes one middleware attribute.

    Getter, setter and notifier follow from the presence of their ids; flync
    states none of them as a flag, and a flag would be a second place for them
    to disagree.

    A SOME/IP field setter is request/response on the wire, carrying the value
    applied, so every setter is one with a response.

    A getter or a setter expects an answer, so its interface states the RPC
    sizing; a field that only notifies states none.
    """
    attributes = []
    for field in flync_service.get("fields") or []:
        name = field["name"]
        has_setter = "setter_id" in field
        rpc = _rpc(
            interactions.get(name), name, required=("getter_id" in field or has_setter)
        )
        deployment = {
            "Timeout": rpc["request_timeout"],
            "SetterWithResponse": has_setter,
            "MaxProxyRequests": rpc["consumer_request_limit"],
            "MaxSkeletonRequests": rpc["provider_request_limit"],
        }
        if "getter_id" in field:
            deployment["GetterID"] = int(field["getter_id"])
        if has_setter:
            deployment["SetterID"] = int(field["setter_id"])
        if "notifier_id" in field:
            deployment["NotificationID"] = int(field["notifier_id"])

        attributes.append(
            {
                "name": class_name(name),
                "type": qualify(_payload_type(name, interactions, registry, suffix="Value")),
                "hasGetter": "getter_id" in field,
                "hasSetter": has_setter,
                "hasNotifier": "notifier_id" in field,
                "deployment": deployment,
            }
        )
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


def _methods(flync_service, interactions, registry, qualify) -> list[dict]:
    """Each method carries at most one argument per direction: the templates read one."""
    methods = []
    for method in flync_service.get("methods") or []:
        name = method["name"]
        interaction = interactions.get(name)
        if interaction is None:
            raise ModelError(
                f"method '{name}' of '{flync_service['name']}' has no parameter description"
            )

        is_fire_and_forget = method.get("type") == "fire_and_forget"
        rpc = _rpc(interaction, name, required=not is_fire_and_forget)
        entry = {"name": class_name(name)}
        if is_fire_and_forget:
            entry["fireAndForget"] = True

        request = _parameter(name, interaction.get("input_parameters"), registry, "Request")
        if request is not None:
            entry["inArgs"] = [{"name": request["name"], "type": qualify(request["type"])}]
        if not is_fire_and_forget:
            response = _parameter(
                name, interaction.get("output_parameters"), registry, "Response"
            )
            if response is not None:
                entry["outArgs"] = [{"name": response["name"], "type": qualify(response["type"])}]

        entry["deployment"] = {
            "MethodID": int(method["id"]),
            "Timeout": rpc["request_timeout"],
            "MaxProxyRequests": rpc["consumer_request_limit"],
            "MaxSkeletonRequests": rpc["provider_request_limit"],
        }
        methods.append(entry)
    return methods


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


#: Name given to a synthesised struct parameter, per direction.
_PARAMETER_NAME = {"Request": "request", "Response": "result"}


def _parameter(method_name, parameters, registry, suffix) -> dict | None:
    """One method direction: absent, a single value, or a synthesised struct."""
    parameters = parameters or []
    if not parameters:
        return None
    if len(parameters) == 1:
        only = parameters[0]
        return {"name": only["name"], "type": registry.resolve(only["dataref"])}

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


# ---------------------------------------------------------------------------
# derived sections
# ---------------------------------------------------------------------------


def _connections(placements, model, cluster_index) -> list[dict]:
    """One connection per ordered cluster pair, carrying every service on it.

    A consumer is paired with the cluster providing the instance it consumes,
    not with every provider of the service. Both directions are emitted: the
    provider side lists the service under providers, the consumer side under
    consumers.

    Capacity follows the pairing: one skeleton per instance provided across the
    pair, one proxy per consuming cluster.
    """
    service_keys = {}
    providers: dict[tuple[str, str], dict[str, set]] = {}
    consumers: dict[tuple[str, str], dict[str, int]] = {}

    for service_name, placement in sorted(placements.items()):
        stem = model.services[service_name]["_stem"]
        service_keys[service_name] = (namespace_from_stem(stem), service_class_from_stem(stem))
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
            providers.setdefault((provider, consumer), {}).setdefault(service_name, set()).add(
                instance_id
            )
            per_pair = consumers.setdefault((consumer, provider), {})
            per_pair[service_name] = per_pair.get(service_name, 0) + 1

    def _order(pair: tuple[str, str]) -> tuple[int, int]:
        return (cluster_index[pair[0]]["id"], cluster_index[pair[1]]["id"])

    def _usage(service_name: str, max_instances: int) -> dict:
        namespace, name = service_keys[service_name]
        return {"namespace": namespace, "name": name, "maxInstances": max_instances}

    connections = []
    for source, target in sorted(set(providers) | set(consumers), key=_order):
        connections.append(
            {
                "sourceCluster": cluster_index[source],
                "targetCluster": cluster_index[target],
                "consumers": [
                    _usage(service_name, count)
                    for service_name, count in sorted(consumers.get((source, target), {}).items())
                ],
                "providers": [
                    _usage(service_name, len(instances))
                    for service_name, instances in sorted(
                        providers.get((source, target), {}).items()
                    )
                ],
            }
        )
    return connections


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
