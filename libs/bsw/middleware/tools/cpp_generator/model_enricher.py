# *******************************************************************************
# Copyright (c) 2026 BMW AG
#
# This program and the accompanying materials are made available under the
# terms of the Apache License Version 2.0 which is available at
# https://www.apache.org/licenses/LICENSE-2.0
#
# SPDX-License-Identifier: Apache-2.0
# *******************************************************************************

"""Derives what the templates need beyond what the deployment YAML states.

Runs after schema validation: type references, the ports of a cluster, and
defaults for optional deployment values.
"""

from loaders import TypeRegistry, load_type_modules, make_type_ref


def _arg_type(args: list | None, registry: TypeRegistry, scope: str):
    if not args:
        return make_type_ref(None)
    return make_type_ref(args[0]["type"], args[0]["name"], registry, scope)


def _enrich_method(method: dict, registry: TypeRegistry, scope: str) -> None:
    method["inType"] = _arg_type(method.get("inArgs"), registry, scope)
    method["outType"] = _arg_type(method.get("outArgs"), registry, scope)
    deployment = method["deployment"]
    deployment.setdefault("MaxProxyRequests", 1)
    deployment.setdefault("MaxSkeletonRequests", 1)


def _enrich_attribute(attribute: dict, registry: TypeRegistry, scope: str) -> None:
    attribute["type"] = make_type_ref(
        attribute["type"], attribute["name"], registry, scope
    )
    deployment = attribute["deployment"]
    # The templates test the ids with `is not none`.
    deployment.setdefault("GetterID", None)
    deployment.setdefault("SetterID", None)
    deployment.setdefault("MaxSkeletonRequests", 1)


def _enrich_service(service: dict, registry: TypeRegistry) -> None:
    scope = service["namespace"]
    for attribute in service.get("attributes") or []:
        _enrich_attribute(attribute, registry, scope)
    for method in service.get("methods") or []:
        _enrich_method(method, registry, scope)
    for broadcast in service.get("broadcasts") or []:
        broadcast["type"] = make_type_ref(
            broadcast.get("type"), broadcast["name"], registry, scope
        )


def _enrich_cluster(cluster: dict) -> None:
    if "providedPorts" in cluster:
        return

    provided, required = [], []
    for instance in cluster.get("componentInstances") or []:
        component = instance["component"]
        for target, ports in (
            (provided, component.get("providedPorts")),
            (required, component.get("requiredPorts")),
        ):
            target.extend(
                {
                    "portName": port["name"],
                    "interfaceName": port["interfaceName"],
                    "interfaceNamespace": port["interfaceNamespace"],
                }
                for port in ports or []
            )
    cluster["providedPorts"] = provided
    cluster["requiredPorts"] = required


def enrich_model(data: dict) -> None:
    """Add the derived fields to *data* (the loaded deployment model) in place."""
    registry = load_type_modules(data)
    for cluster in data["clusters"]:
        _enrich_cluster(cluster)
    for service in data["services"]:
        _enrich_service(service, registry)
    for connection in data["connections"]:
        _enrich_cluster(connection["source_cluster"])
        _enrich_cluster(connection["target_cluster"])
        connection["sourceCluster"] = connection["source_cluster"]
        connection["targetCluster"] = connection["target_cluster"]
        connection["providers"] = connection["skeletons"]
        connection["consumers"] = connection["proxies"]
