# *******************************************************************************
# Copyright (c) 2026 KPIT Technologies Ltd.
#
# This program and the accompanying materials are made available under the
# terms of the Apache License Version 2.0 which is available at
# https://www.apache.org/licenses/LICENSE-2.0
#
# SPDX-License-Identifier: Apache-2.0
# *******************************************************************************

"""Merge a host deployment with one derived from a FLYNC model.

    merge_deployment.py HOST_YAML MODEL_YAML OUTPUT_YAML

An executable can have services of its own that live in no FLYNC model, next
to the ones derived from it. Generating the two separately would produce two
memory layouts and two sets of cluster-connection globals, so they are merged
into one deployment first.

Every fact keeps exactly one owner:

    services      host and model each contribute; one id in both is an error
    typeModules   host and model each contribute; one header in both is an error
    components    union; a name in both must be identical
    clusters      union; a name in both must be identical apart from the
                  component instances, which both contribute
    allocators    union; a name in both with other pools is renamed on the
                  model's side to flync<name>, so each side keeps its own sizing;
                  the host's default allocator stays the only default
    connections   host plus derived, merged by cluster pair
    cores         union; a cluster placed on two cores is an error
    anything else the host's own (e.g. pdus), passed through
"""

import sys

import yaml


class MergeError(Exception):
    """The host deployment and the model disagree."""


#: Sections both sides may contribute; any other section is the host's alone.
_MERGED = (
    "components",
    "clusters",
    "allocators",
    "typeModules",
    "services",
    "connections",
    "cores",
)


def merge(host: dict, model: dict) -> dict:
    """Return one deployment holding the host's and the model's services."""
    unknown = sorted(set(model) - set(_MERGED))
    if unknown:
        raise MergeError(f"the FLYNC model's deployment has unexpected sections: {unknown}")

    clusters = _clusters(host.get("clusters"), model.get("clusters"))
    by_name = {cluster["name"]: cluster for cluster in clusters}

    merged = {
        "components": _union_named("component", host.get("components"), model.get("components")),
        "clusters": clusters,
        "allocators": _union_allocators(host.get("allocators"), model.get("allocators"), model),
    }
    type_modules = _type_modules(host.get("typeModules"), model.get("typeModules"))
    if type_modules:
        merged["typeModules"] = type_modules
    merged["services"] = _services(host.get("services"), model.get("services"))
    merged["connections"] = _connections(host.get("connections"), model.get("connections"), by_name)

    cores = _cores(host.get("cores"), model.get("cores"), by_name)
    if cores:
        merged["cores"] = cores

    for key, value in host.items():
        if key not in _MERGED:
            merged[key] = value
    return merged


def _service_id(service: dict) -> int:
    return int(service["deployment"]["ServiceID"])


def _services(host: list | None, model: list | None) -> list:
    host, model = host or [], model or []
    host_ids = {_service_id(service): service["name"] for service in host}
    for service in model:
        clash = host_ids.get(_service_id(service))
        if clash is not None:
            raise MergeError(
                f"service {_service_id(service):#06x} is defined by the host deployment "
                f"('{clash}') and by the FLYNC model ('{service['name']}'); remove it "
                "from one of them"
            )
    return list(host) + list(model)


def _type_modules(host: list | None, model: list | None) -> list:
    host, model = host or [], model or []
    host_headers = {module["header"] for module in host}
    for module in model:
        if module["header"] in host_headers:
            raise MergeError(
                f"type module {module['header']} is defined by the host deployment and "
                "by the FLYNC model"
            )
    return list(host) + list(model)


def _union_named(kind: str, host: list | None, model: list | None) -> list:
    """Host entries in their order, then any the model adds; shared ones must agree."""
    merged = list(host or [])
    by_name = {entry["name"]: entry for entry in merged}
    for entry in model or []:
        existing = by_name.get(entry["name"])
        if existing is None:
            merged.append(entry)
            by_name[entry["name"]] = entry
        elif _plain(existing) != _plain(entry):
            raise MergeError(
                f"{kind} '{entry['name']}' is defined differently by the host deployment "
                f"and the FLYNC model:\n  host : {_plain(existing)}\n  model: {_plain(entry)}"
            )
    return merged


def _clusters(host: list | None, model: list | None) -> list:
    """Clusters of both sides; one in both hosts the component instances of both."""
    merged = [dict(cluster) for cluster in host or []]
    by_name = {cluster["name"]: cluster for cluster in merged}
    for entry in model or []:
        existing = by_name.get(entry["name"])
        if existing is None:
            copy = dict(entry)
            merged.append(copy)
            by_name[copy["name"]] = copy
            continue
        if _without_instances(existing) != _without_instances(entry):
            raise MergeError(
                f"cluster '{entry['name']}' is defined differently by the host deployment "
                f"and the FLYNC model:\n  host : {_without_instances(existing)}\n"
                f"  model: {_without_instances(entry)}"
            )
        instances = list(existing.get("componentInstances") or [])
        names = {instance["instanceName"] for instance in instances}
        for instance in entry.get("componentInstances") or []:
            if instance["instanceName"] in names:
                raise MergeError(
                    f"cluster '{entry['name']}' hosts component instance "
                    f"'{instance['instanceName']}' in both the host deployment and the FLYNC model"
                )
            instances.append(instance)
        if instances:
            existing["componentInstances"] = instances
    return merged


def _without_instances(cluster: dict) -> dict:
    return _plain({key: value for key, value in cluster.items() if key != "componentInstances"})


def _union_allocators(host: list | None, model: list | None, model_deployment: dict) -> list:
    """Host allocators, then the model's; a name both use for other pools is renamed.

    The model's services name their allocator, so only the host's default stays
    the default when the host has one.
    """
    merged = list(host or [])
    by_name = {entry["name"]: entry for entry in merged}
    host_has_default = any(entry.get("default") for entry in merged)
    for entry in model or []:
        entry = dict(entry)
        if host_has_default:
            entry.pop("default", None)
        existing = by_name.get(entry["name"])
        if existing is not None and _pools(existing) == _pools(entry):
            continue
        if existing is not None:
            renamed = f"flync{entry['name']}".lower()
            if renamed in by_name:
                raise MergeError(
                    f"allocator '{entry['name']}' is defined differently by the host "
                    f"deployment and the FLYNC model, and '{renamed}' is taken too"
                )
            _rename_allocator(model_deployment, entry["name"], renamed)
            entry["name"] = renamed
        merged.append(entry)
        by_name[entry["name"]] = entry
    return merged


def _rename_allocator(model_deployment: dict, old: str, new: str) -> None:
    """Rename the allocator in every model service that selects it."""
    for service in model_deployment.get("services") or []:
        deployment = service["deployment"]
        if deployment.get("MemoryAllocator") == old:
            deployment["MemoryAllocator"] = new


def _connections(host: list | None, model: list | None, clusters: dict) -> list:
    """One connection per ordered cluster pair, carrying both sides' services.

    Each connection refers to the merged cluster, so it sees the component
    instances both sides placed on it.
    """
    merged: list = []
    by_pair: dict = {}

    def add(connection: dict) -> None:
        pair = (connection["sourceCluster"]["name"], connection["targetCluster"]["name"])
        existing = by_pair.get(pair)
        if existing is None:
            copy = dict(connection)
            copy["sourceCluster"] = clusters[pair[0]]
            copy["targetCluster"] = clusters[pair[1]]
            copy["consumers"] = list(connection.get("consumers") or [])
            copy["providers"] = list(connection.get("providers") or [])
            merged.append(copy)
            by_pair[pair] = copy
        else:
            existing["consumers"].extend(connection.get("consumers") or [])
            existing["providers"].extend(connection.get("providers") or [])

    for connection in host or []:
        add(connection)
    for connection in model or []:
        add(connection)
    return merged


def _cores(host: list | None, model: list | None, clusters: dict) -> list:
    """Host cores, then the model's; each cluster stays on exactly one core."""
    merged = [
        dict(core, clusters=[clusters[cluster["name"]] for cluster in core.get("clusters") or []])
        for core in host or []
    ]
    by_name = {core["name"]: core for core in merged}
    placed = {
        cluster["name"]: core["name"] for core in merged for cluster in core["clusters"]
    }
    for core in model or []:
        target = by_name.get(core["name"])
        if target is None:
            target = dict(core, clusters=[])
            merged.append(target)
            by_name[core["name"]] = target
        for cluster in core.get("clusters") or []:
            owner = placed.get(cluster["name"])
            if owner is not None and owner != target["name"]:
                raise MergeError(
                    f"cluster '{cluster['name']}' is placed on core '{owner}' by the host "
                    f"deployment and on core '{target['name']}' by the FLYNC model"
                )
            if owner is None:
                target["clusters"].append(clusters[cluster["name"]])
                placed[cluster["name"]] = target["name"]
    return merged


def _pools(entry: dict) -> list:
    """Pools are a set of chunk sizes; their order in the file is not meaningful."""
    return sorted((int(p["ChunkSize"]), int(p["NumberOfElements"])) for p in entry.get("pools") or [])


def _plain(obj):
    """Compare by content only: dict ordering and anchor identity do not matter."""
    if isinstance(obj, dict):
        return {k: _plain(v) for k, v in sorted(obj.items())}
    if isinstance(obj, list):
        return [_plain(v) for v in obj]
    return obj


class _NoAliases(yaml.SafeDumper):
    def ignore_aliases(self, data):
        return True


def main(argv: list[str]) -> int:
    if len(argv) != 4:
        print(__doc__, file=sys.stderr)
        return 2
    with open(argv[1]) as handle:
        host = yaml.safe_load(handle)
    with open(argv[2]) as handle:
        model = yaml.safe_load(handle)
    try:
        merged = merge(host, model)
    except MergeError as exc:
        print(f"error: {exc}", file=sys.stderr)
        return 1
    with open(argv[3], "w") as handle:
        yaml.dump(merged, handle, Dumper=_NoAliases, sort_keys=False)
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
