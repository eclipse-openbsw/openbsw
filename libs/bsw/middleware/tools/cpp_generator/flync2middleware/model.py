# *******************************************************************************
# Copyright (c) 2026 KPIT Technologies Ltd.
#
# This program and the accompanying materials are made available under the
# terms of the Apache License Version 2.0 which is available at
# https://www.apache.org/licenses/LICENSE-2.0
#
# SPDX-License-Identifier: Apache-2.0
# *******************************************************************************

"""Load the parts of a FLYNC model this generator reads.

    communication/someip/services/*.flync.yaml   members, ids, and their
                                                 datatypes inline
    apps/*.flync.yaml                            provider and consumer refs
    ecus/*/controllers/*/app_bindings.flync.yaml which controller runs an app
    communication/middleware/*.flync.yaml        clusters and the apps each
                                                 runs, pools, and optional
                                                 per-service overrides

The services to generate are the ones an app on a cluster provides or consumes.

A member carries its datatype inline rather than naming a declared one. This
module gives each distinct datatype a dataref named after its shape, which is
what `translate` and `cpp_types` refer to it by.

`sdk_validate` runs the FLYNC SDK's own validation over the whole model,
when the SDK is installed.
"""

import contextlib
import os
from pathlib import Path

import yaml

from .cpp_types import SCALAR_WIDTHS, SCALARS


class ModelError(Exception):
    """The model is missing something this generator needs."""


class SdkUnavailable(Exception):
    """The FLYNC SDK cannot be imported, so the model cannot be validated."""


#: Middleware RPC sizing of a service.
RPC_KEYS = ("request_timeout", "consumer_request_limit", "provider_request_limit")

#: Middleware settings of a service unless its `services` entry overrides them:
#: the `default` allocator profile, which must exist anyway, and the RPC sizing
#: the referenceApp's own deployment uses.
SERVICE_DEFAULTS = {
    "allocator_profile": "default",
    "request_timeout": 5000,
    "consumer_request_limit": 1,
    "provider_request_limit": 1,
}

#: The files this generator reads, relative to the model root. A validation
#: finding in one of them can change what is generated; one anywhere else cannot.
READ_PATTERNS = (
    "communication/someip/services/*.flync.yaml",
    "apps/*.flync.yaml",
    "ecus/*/controllers/*/app_bindings.flync.yaml",
    "communication/middleware/*.flync.yaml",
)


def sdk_validate(root: Path) -> tuple[str, list[dict]]:
    """Validate the whole model with the FLYNC SDK.

    Returns the SDK's workspace state and one finding per reported problem:
    the file, whether this generator reads it, whether it is only a warning,
    and the SDK's location and message. Raises SdkUnavailable when the SDK is
    not installed; anything else the SDK raises is passed on.
    """
    try:
        from flync.sdk.helpers.validation_helpers import validate_workspace
    except ImportError as exc:
        raise SdkUnavailable(str(exc)) from exc

    root = Path(root).resolve()
    # The SDK prints its own progress; only its findings are wanted here. The
    # sink must be a real file: the SDK needs file descriptors for its workers
    # and its fault handler.
    with open(os.devnull, "w") as sink, contextlib.redirect_stdout(sink), contextlib.redirect_stderr(sink):
        result = validate_workspace(root)

    findings = []
    for document, details in (result.errors or {}).items():
        path = _relative_to(str(document), root)
        for detail in details or []:
            detail = detail if isinstance(detail, dict) else {"msg": str(detail)}
            location = detail.get("loc") or ()
            findings.append(
                {
                    "file": path,
                    "read": _is_read(path),
                    "warning": detail.get("type") == "warning",
                    "kind": str(detail.get("type") or ""),
                    "location": ".".join(str(part) for part in location)
                    if isinstance(location, (list, tuple))
                    else str(location),
                    "message": str(detail.get("msg") or ""),
                }
            )
    state = getattr(result.state, "value", result.state)
    return str(state), findings


def _relative_to(document: str, root: Path) -> str:
    """A document the SDK reports, as a path relative to the model root."""
    path = Path(document[len("file://"):] if document.startswith("file://") else document)
    try:
        return path.resolve().relative_to(root).as_posix()
    except ValueError:
        return path.as_posix()


def _is_read(path: str) -> bool:
    return any(Path(path).match(pattern) for pattern in READ_PATTERNS)


def _load(path: Path) -> dict:
    with open(path, "r") as handle:
        data = yaml.safe_load(handle)
    if not isinstance(data, dict):
        raise ModelError(f"{path}: expected a YAML mapping at the top level")
    return data


class FlyncModel:
    def __init__(self, root: Path):
        self.root = root
        if not root.is_dir():
            raise ModelError(f"model root not found: {root}")

        self.datatypes: dict = {}
        self.wire_types: dict = {}
        self._interactions: dict = {}
        self.services = self._load_services()
        self._service_names = {int(service["id"]): name for name, service in self.services.items()}
        self.apps = self._load_apps()
        self.app_controllers = self._load_app_bindings()
        self.overlay = self._load_middleware()
        self.app_clusters = self._load_app_clusters()
        self.middleware_services = self._middleware_services()
        self._attach_rpc()

    # ---- communication/someip/services -------------------------------------

    def _load_services(self) -> dict:
        directory = self.root / "communication" / "someip" / "services"
        if not directory.is_dir():
            raise ModelError(f"no service directory at {directory}")

        services = {}
        for path in sorted(directory.glob("*.flync.yaml")):
            data = _load(path)
            name = data.get("name")
            if name is None:
                raise ModelError(f"{path}: service has no `name`")
            data["_stem"] = path.name[: -len(".flync.yaml")]
            services[name] = data
            self._interactions[name] = self._interactions_of(name, data)
        return services

    def _interactions_of(self, service_name: str, service: dict) -> dict:
        """One interaction per member, built from the member's own parameters."""
        interactions: dict = {}
        # Datatype names are scoped to the service, so two services can each
        # carry their own uint8 array without colliding.
        scope = service["_stem"]

        for event in service.get("events") or []:
            interactions[event["name"]] = {
                "dataref": [self._dataref(p, scope) for p in event.get("parameters") or []]
            }
        for field in service.get("fields") or []:
            interactions[field["name"]] = {
                "dataref": [self._dataref(p, scope) for p in field.get("parameters") or []]
            }
        for method in service.get("methods") or []:
            interactions[method["name"]] = {
                "input_parameters": [
                    {"name": p["name"], "dataref": self._dataref(p, scope)}
                    for p in method.get("input_parameters") or []
                ],
                "output_parameters": [
                    {"name": p["name"], "dataref": self._dataref(p, scope)}
                    for p in method.get("output_parameters") or []
                ],
            }

        for name, interaction in interactions.items():
            interaction["_source"] = f"{service_name} service file"
        return interactions

    # ---- inline datatype -> the dataref view the type registry reads -------

    def _dataref(self, parameter: dict, scope: str) -> str:
        datatype = parameter.get("datatype")
        if not isinstance(datatype, dict):
            raise ModelError(f"parameter '{parameter.get('name')}' carries no datatype")

        config = _config_of(datatype)
        name = f"{scope}_{_shape_name(datatype)}"

        # The name is a function of the shape, so two shapes reaching one name
        # is a defect in that function rather than something a model can state.
        earlier = self.datatypes.get(name)
        if earlier is not None and earlier["config"] != config:
            raise ModelError(
                f"naming is not injective: '{name}' is produced by two different shapes:\n"
                f"  {earlier['config']}\n  {config}"
            )
        self.datatypes[name] = {"name": name, "config": config}
        # The middleware view drops the wire rules -- a prefix width, an
        # encoding, an enum's range. The codec needs them, so the datatype is
        # kept as the model states it.
        self.wire_types[name] = datatype
        return name

    # ---- apps --------------------------------------------------------------

    def _load_apps(self) -> dict:
        directory = self.root / "apps"
        if not directory.is_dir():
            raise ModelError(f"no apps directory at {directory}")

        apps = {}
        for path in sorted(directory.glob("*.flync.yaml")):
            data = _load(path)
            apps[path.name[: -len(".flync.yaml")]] = data
        if not apps:
            raise ModelError(f"{directory}: no application declares a service reference")
        return apps

    def _load_app_bindings(self) -> dict:
        """app -> controller, from every controller's app_bindings file."""
        controllers: dict = {}
        for path in sorted(self.root.glob("ecus/*/controllers/*/app_bindings.flync.yaml")):
            controller = path.parent.name
            for app in _load(path).get("app_refs") or []:
                earlier = controllers.setdefault(app, controller)
                if earlier != controller:
                    raise ModelError(
                        f"app '{app}' is bound to both '{earlier}' and '{controller}'"
                    )
        if not controllers:
            raise ModelError("no controller binds an app; cluster placement cannot be resolved")
        return controllers

    # ---- communication/middleware -------------------------------------------

    def _load_middleware(self) -> dict:
        directory = self.root / "communication" / "middleware"
        if not directory.is_dir():
            raise ModelError(
                f"no middleware configuration at {directory} -- FLYNC alone cannot say "
                "which clusters exist, how they are sized, or which memory pools a service "
                "allocates from"
            )

        merged: dict = {"clusters": [], "allocator_profiles": [], "services": []}
        for path in sorted(directory.glob("*.flync.yaml")):
            data = _load(path)
            for section in merged:
                merged[section].extend(data.get(section) or [])

        if not merged["clusters"]:
            raise ModelError(f"{directory}: no clusters declared")
        return merged

    def _load_app_clusters(self) -> dict:
        """app -> cluster, from the apps each cluster lists."""
        clusters: dict = {}
        for cluster in self.overlay["clusters"]:
            for app in cluster.get("app_refs") or []:
                if app not in self.apps:
                    raise ModelError(
                        f"cluster '{cluster['name']}' lists app '{app}', which no file in apps declares"
                    )
                earlier = clusters.setdefault(app, cluster["name"])
                if earlier != cluster["name"]:
                    raise ModelError(
                        f"app '{app}' is listed by both cluster '{earlier}' and '{cluster['name']}'"
                    )
        return clusters

    def _middleware_services(self) -> dict:
        """Every service an app on a cluster provides or consumes, with its settings."""
        used: set = set()
        for app in self.app_clusters:
            for key in ("service_provider_refs", "service_consumer_refs"):
                for ref in self.apps[app].get(key) or []:
                    name = self._service_of(app, ref)
                    if name is not None:
                        used.add(name)
        if not used:
            raise ModelError(
                "no app listed in a cluster's app_refs provides or consumes a service "
                "declared in communication/someip/services"
            )

        overrides: dict = {}
        for entry in self.overlay["services"]:
            name = entry.get("service_ref")
            if name is None:
                raise ModelError("middleware configuration: a services entry has no service_ref")
            if name not in self.services:
                raise ModelError(
                    f"middleware configuration names service '{name}', which no file in "
                    "communication/someip/services declares"
                )
            if name not in used:
                raise ModelError(
                    f"middleware configuration overrides service '{name}', which no app on a "
                    "cluster provides or consumes"
                )
            unknown = sorted(set(entry) - set(SERVICE_DEFAULTS) - {"service_ref"})
            if unknown:
                raise ModelError(
                    f"middleware configuration: service '{name}' has unknown key(s) "
                    f"{', '.join(unknown)}; allowed are {', '.join(SERVICE_DEFAULTS)}"
                )
            overrides[name] = {key: value for key, value in entry.items() if key != "service_ref"}

        # In the order the service files are read, so the output does not depend
        # on which app happens to reference a service first.
        return {
            name: {**SERVICE_DEFAULTS, **overrides.get(name, {})}
            for name in self.services
            if name in used
        }

    def _attach_rpc(self) -> None:
        """RPC sizing is stated per service, so every member of it inherits it."""
        for name, entry in self.middleware_services.items():
            rpc = {key: entry[key] for key in RPC_KEYS if entry.get(key) is not None}
            for interaction in self._interactions[name].values():
                interaction.update(rpc)

    # ---- joins --------------------------------------------------------------

    def interactions_for(self, service_name: str) -> dict:
        return self._interactions.get(service_name, {})

    def _service_of(self, app: str, ref: dict):
        """The service a reference names, by id and major version as FLYNC resolves it."""
        service_id = ref.get("service_id")
        if service_id is None:
            raise ModelError(f"app '{app}' has a service reference with no service_id")
        name = self._service_names.get(int(service_id))
        if name is None:
            return None
        declared = self.services[name]["major_version"]
        if int(ref["major_version"]) != int(declared):
            raise ModelError(
                f"app '{app}' references service 0x{int(service_id):04x} major version "
                f"{ref['major_version']}, but the service declares {declared}"
            )
        return name

    def placements(self) -> dict:
        """Per middleware service: which cluster provides or consumes which instance."""
        placements: dict = {
            name: {"providers": set(), "consumers": set(), "instance_ids": set()}
            for name in self.middleware_services
        }

        for app, data in self.apps.items():
            for role, key in (("providers", "service_provider_refs"), ("consumers", "service_consumer_refs")):
                for ref in data.get(key) or []:
                    service = self._service_of(app, ref)
                    if service not in placements:
                        continue
                    where = self.app_clusters.get(app)
                    if where is None:
                        raise ModelError(
                            f"app '{app}' references middleware service '{service}', but no "
                            "cluster lists it in app_refs"
                        )
                    instance_id = int(ref["instance_id"])
                    placements[service][role].add((where, instance_id))
                    placements[service]["instance_ids"].add(instance_id)

        for service, placement in placements.items():
            if not placement["instance_ids"]:
                raise ModelError(
                    f"middleware configuration names service '{service}', but no app places "
                    "it on a cluster"
                )
        return placements

    def cluster_controllers(self) -> dict:
        """cluster -> controller, through the apps the cluster runs."""
        controllers: dict = {}

        for app, data in self.apps.items():
            for key in ("service_provider_refs", "service_consumer_refs"):
                for ref in data.get(key) or []:
                    if self._service_of(app, ref) not in self.middleware_services:
                        continue
                    where = self.app_clusters.get(app)
                    controller = self.app_controllers.get(app)
                    if controller is None:
                        raise ModelError(f"no controller binds app '{app}'")
                    earlier = controllers.setdefault(where, controller)
                    if earlier != controller:
                        raise ModelError(
                            f"cluster '{where}' is claimed by apps on '{earlier}' and "
                            f"'{controller}'; a cluster belongs to one controller"
                        )
        return controllers


# ---------------------------------------------------------------------------
# inline datatype -> config + name
# ---------------------------------------------------------------------------


def _config_of(datatype: dict) -> dict:
    """The dataref `config` the type registry reads, from an inline datatype."""
    kind = datatype.get("type")

    if datatype.get("dimensions"):
        element = (datatype.get("element_type") or {}).get("type")
        return {
            "datatype": element,
            "dimensions": [dict(dimension) for dimension in datatype["dimensions"]],
        }
    if kind == "enum":
        # Carried as its base scalar; the type registry emits no typedef for
        # it. Kept distinct from a plain scalar because its range is a wire
        # rule the codec has to enforce.
        return {"datatype": "enum", "base_type": {"datatype": (datatype.get("base_type") or {}).get("type")}}
    if kind == "bitfield":
        return {"datatype": "bitfield", "length": datatype.get("length")}
    if kind == "union":
        # Member names are a codec concern, and request and response spell them
        # differently; the middleware only needs the widest member.
        return {
            "datatype": "union",
            "members": [
                {"datatype": member.get("type"), "index": member.get("index")}
                for member in datatype.get("members") or []
            ],
        }
    if kind in ("fixed_length_string", "dynamic_length_string"):
        config = {"datatype": kind, "encoding": datatype.get("encoding")}
        if "length" in datatype:
            config["length"] = datatype["length"]
        return config
    return {"datatype": kind}


def _bitfield_scalar(datatype: dict) -> str:
    """The scalar a bitfield of this width is carried in."""
    width = {8: "uint8", 16: "uint16", 32: "uint32", 64: "uint64"}.get(int(datatype.get("length", 0)))
    if width is None:
        raise ModelError(f"bitfield of width {datatype.get('length')!r} has no scalar to carry it")
    return width


def _shape_name(datatype: dict) -> str:
    """A deterministic name for a datatype that has none of its own.

    FLYNC states a datatype where it is used, so two members of the same
    shape are two anonymous declarations of one type. The name is a function of
    the shape alone -- never of the member that declared it, nor of declaration
    order -- so adding a member never renames an existing type.

    Total over the datatype grammar: an unsupported construct raises here
    rather than reaching the templates under a name that means nothing.
    """
    kind = datatype.get("type")

    dimensions = datatype.get("dimensions")
    if dimensions:
        element = (datatype.get("element_type") or {}).get("type")
        if element is None:
            raise ModelError("an array datatype names no element_type")
        parts = [element, "array"]
        # One segment per dimension, outer to inner, so a 2-dimensional array
        # cannot collide with a 1-dimensional one of the same element.
        for dimension in dimensions:
            parts.extend(_dimension_name(dimension))
        return "_".join(str(part) for part in parts)

    if kind in SCALARS:
        return str(kind)
    if kind == "enum":
        base = (datatype.get("base_type") or {}).get("type")
        if base not in SCALARS:
            raise ModelError(f"enum datatype has no scalar base_type: {base!r}")
        entries = datatype.get("entries") or []
        if not entries:
            raise ModelError("an enum datatype declares no entries")
        return f"enum_0to{max(int(entry['value']) for entry in entries)}"
    if kind == "bitfield":
        # Carried in a scalar of that width, but distinct from one: the wire
        # rule is the width, not the C++ type.
        _bitfield_scalar(datatype)
        return f"bitfield{datatype.get('length')}"
    if kind == "union":
        # The C++ type is an opaque buffer, so its width is its identity. A
        # member list would rename the type whenever a member is added that
        # does not widen it.
        members = datatype.get("members") or []
        widest = max((SCALAR_WIDTHS.get(member.get("type"), 0) for member in members), default=0)
        if widest == 0:
            raise ModelError("a union datatype declares no member with a known width")
        return f"union{widest + 8}"
    if kind == "fixed_length_string":
        return f"{_encoding_tag(datatype)}_fixed{datatype.get('length')}"
    if kind == "dynamic_length_string":
        return f"{_encoding_tag(datatype)}_dynamic"

    raise ModelError(f"datatype kind {kind!r} has no naming rule")


def _dimension_name(dimension: dict) -> list:
    """The segments one array dimension contributes to a name."""
    if dimension.get("kind") == "fixed":
        return [f"fixed{dimension.get('length')}"]

    parts = [f"dyn{dimension.get('length_of_length_field')}"]
    if dimension.get("lower_limit"):
        parts.append(f"min{dimension['lower_limit']}")
    if dimension.get("upper_limit"):
        parts.append(f"max{dimension['upper_limit']}")
    return parts


def _encoding_tag(datatype: dict) -> str:
    encoding = str(datatype.get("encoding", "")).lower().replace("-", "")
    return encoding or "string"
