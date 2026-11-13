# *******************************************************************************
# Copyright (c) 2026 BMW AG
#
# This program and the accompanying materials are made available under the
# terms of the Apache License Version 2.0 which is available at
# https://www.apache.org/licenses/LICENSE-2.0
#
# SPDX-License-Identifier: Apache-2.0
# *******************************************************************************

"""Loads the ``connections`` of the deployment model.

A connection is unidirectional: its ``consumers`` are the services the source
cluster requires from the target cluster and its ``providers`` are the services
the source cluster provides to it. Both lists name services of the ``services``
list and state how many endpoints of the service exist on that side.

:func:`check_connections` validates the connections and their references (run it
on the schema-validated model). :func:`load_connections` then gives every
consumer and provider the service data the templates read from it.
"""

_SIDES = ("consumers", "providers")


def _service_key(entry: dict) -> tuple[str, str]:
    return entry["namespace"], entry["name"]


def check_connections(data: dict) -> list[str]:
    """Return the errors found in the connections of *data*."""
    errors: list[str] = []
    cluster_names = {cluster["name"] for cluster in data["clusters"]}
    service_keys = {_service_key(service) for service in data["services"]}
    seen_pairs: set[tuple[str, str]] = set()

    for connection in data["connections"]:
        source = connection["sourceCluster"]["name"]
        target = connection["targetCluster"]["name"]
        label = f"connection {source} -> {target}"

        for role, name in (("sourceCluster", source), ("targetCluster", target)):
            if name not in cluster_names:
                errors.append(f"{label}: {role} '{name}' is not a declared cluster")
        if (source, target) in seen_pairs:
            errors.append(f"{label}: declared more than once")
        seen_pairs.add((source, target))

        for side in _SIDES:
            used: set[tuple[str, str]] = set()
            for usage in connection[side]:
                key = _service_key(usage)
                qualified = "::".join(key)
                if key not in service_keys:
                    errors.append(f"{label}: {side} names unknown service {qualified}")
                elif key in used:
                    errors.append(f"{label}: {side} lists service {qualified} twice")
                used.add(key)
        if not connection["consumers"] and not connection["providers"]:
            errors.append(f"{label}: must declare at least one consumer or provider")
    return errors


def load_connections(data: dict) -> None:
    """Add the data of the named service to every consumer and provider in place."""
    services = {_service_key(service): service for service in data["services"]}
    for connection in data["connections"]:
        for side in _SIDES:
            for usage in connection[side]:
                service = services[_service_key(usage)]
                usage["commonInclude"] = service["commonInclude"]
                usage["deployment"] = service["deployment"]
