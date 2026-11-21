# *******************************************************************************
# Copyright (c) 2026 KPIT Technologies Ltd.
#
# This program and the accompanying materials are made available under the
# terms of the Apache License Version 2.0 which is available at
# https://www.apache.org/licenses/LICENSE-2.0
#
# SPDX-License-Identifier: Apache-2.0
# *******************************************************************************

"""flync datatype -> the SOME/IP wire encoding of one service's members.

The middleware moves a C++ object between two clusters and never serialises
it, so `translate` deliberately drops the wire rules: a length-prefix width,
a string encoding, an enum's range, a union's member table. A bridge onto
SOME/IP needs exactly those, and they are all in the model.

This builds one codec specification per distinct datatype a service uses.
The deployment says what each member's C++ type is called; the model says how
that type is written on the wire. The two are joined on the member name, so
the tag and the type it encodes cannot drift apart.

Not derived here, deliberately: E2E. The model declares a profile and a data
id per member, but states the header fields as ordinary parameters on one
member and leaves them implicit on another, so wrapping cannot be decided
from the model as it stands. An E2E-protected member needs a hand-written codec.
"""

import functools

from .cpp_types import SCALAR_WIDTHS, SCALARS, class_name, cpp_identifier
from .model import ModelError

#: Scalars the serializer streams directly, so they need no codec of their own.
_STREAMED = set(SCALAR_WIDTHS)


#: Where the codec framework -- SomeIpCodec, the scalar tag, the serializable
#: adapter, the per-kind encodings -- is included from.
DEFAULT_FRAMEWORK_INCLUDE = "middleware/bindings/someip/SomeIpCodec.h"


def build(model, deployment: dict, framework_include: str = DEFAULT_FRAMEWORK_INCLUDE) -> list:
    """One codec header's worth of specification per service."""
    # Members sharing a datatype share its spec, so each is worked out once.
    spec = functools.lru_cache(maxsize=None)(functools.partial(_spec, model))
    services = []
    for service in deployment.get("services") or []:
        flync_service = _flync_service(model, service)
        tags: dict = {}
        for member, direction, cpp_type in _members(model, service, flync_service):
            _collect(spec, tags, member, direction, cpp_type)
        services.append(
            {
                "namespace": service["namespace"],
                "name": service["name"],
                "header": service["headers"]["common"],
                "codec_framework": framework_include,
                "tags": [tags[name] for name in sorted(tags)],
            }
        )
    return services


def _flync_service(model, service: dict) -> dict:
    for name, candidate in model.services.items():
        if int(candidate["id"]) == int(service["id"]):
            return candidate
    raise ModelError(f"no service file declares id {service['id']:#06x}")


def _members(model, service: dict, flync_service: dict):
    """(member, direction, C++ type) for everything that crosses the wire.

    The deployment names members in C++ spelling and the model in its own, so
    they are matched through the same transformation the deployment used.
    """
    interactions = model.interactions_for(flync_service["name"])

    by_name = {entry["name"]: entry for entry in service.get("events") or []}
    for event in flync_service.get("events") or []:
        entry = by_name.get(class_name(event["name"]))
        if entry is not None:
            yield interactions[event["name"]], "value", entry["type"]

    by_name = {entry["name"]: entry for entry in service.get("attributes") or []}
    for field in flync_service.get("fields") or []:
        entry = by_name.get(class_name(field["name"]))
        if entry is not None:
            yield interactions[field["name"]], "value", entry["type"]

    methods = (service.get("methods") or []) + (service.get("methods_fire_and_forget") or [])
    by_name = {entry["name"]: entry for entry in methods}
    for method in flync_service.get("methods") or []:
        entry = by_name.get(class_name(method["name"]))
        if entry is None:
            continue
        if "input_param" in entry:
            yield interactions[method["name"]], "input", entry["input_param"]["type"]
        if "output_param" in entry:
            yield interactions[method["name"]], "output", entry["output_param"]["type"]


def _collect(spec, tags: dict, interaction: dict, direction: str, cpp_type: str) -> None:
    """Register the tag a member needs, and any tag its fields need."""
    datarefs = _datarefs(interaction, direction)
    if not datarefs:
        return

    if len(datarefs) == 1:
        member_spec = spec(datarefs[0][1], cpp_type)
        if member_spec is not None:
            tags[member_spec["tag"]] = member_spec
        return

    # Several datarefs are one struct on the wire, written in declaration
    # order with no length prefix of its own.
    fields = []
    for field_name, dataref in datarefs:
        field_spec = spec(dataref, None)
        if field_spec is not None:
            tags[field_spec["tag"]] = field_spec
        fields.append({"name": field_name, "tag": field_spec["tag"] if field_spec else None})
    tags[f"{cpp_type}Tag"] = {
        "tag": f"{cpp_type}Tag",
        "kind": "struct",
        "type": cpp_type,
        "fields": fields,
    }


def _datarefs(interaction: dict, direction: str) -> list:
    if direction == "value":
        refs = interaction.get("dataref") or []
        if isinstance(refs, str):
            refs = [refs]
        return [(f"field{index}", ref) for index, ref in enumerate(refs)]

    key = "input_parameters" if direction == "input" else "output_parameters"
    return [(p["name"], p["dataref"]) for p in interaction.get(key) or []]


def _spec(model, dataref: str, cpp_type: str | None) -> dict | None:
    """The codec one datatype needs, or None when the serializer streams it."""
    datatype = model.wire_types.get(dataref)
    if datatype is None:
        raise ModelError(f"no datatype was recorded for '{dataref}'")

    kind = datatype.get("type")
    # The same spelling the typedef got, so a tag and the type it encodes are
    # recognisably one pair.
    name = cpp_identifier(dataref)
    tag = f"{name}Tag"
    value = cpp_type or name

    dimensions = datatype.get("dimensions")
    if dimensions:
        element = (datatype.get("element_type") or {}).get("type")
        if element not in _STREAMED:
            raise ModelError(f"array '{dataref}' has non-scalar element {element!r}")
        outer = dimensions[0]
        if len(dimensions) > 2:
            raise ModelError(f"array '{dataref}' has {len(dimensions)} dimensions; at most 2 are supported")
        if len(dimensions) == 2:
            inner = dimensions[1]
            return {
                "tag": tag, "kind": "array2d", "type": value, "element": _cpp(element),
                "outer_prefix": _prefix(outer, dataref), "inner_prefix": _prefix(inner, dataref),
                "inner_min": inner.get("lower_limit") or 0,
            }
        if outer.get("kind") == "fixed":
            return {
                "tag": tag, "kind": "array_fixed", "type": value,
                "element": _cpp(element), "length": outer.get("length"),
            }
        return {
            "tag": tag, "kind": "array_dynamic", "type": value, "element": _cpp(element),
            "prefix": _prefix(outer, dataref), "min": outer.get("lower_limit") or 0,
        }

    if kind == "fixed_length_string":
        return {
            "tag": tag, "kind": "string_fixed", "type": value,
            "length": datatype.get("length"), "encoding": _encoding(datatype, dataref),
        }
    if kind == "dynamic_length_string":
        return {
            "tag": tag, "kind": "string_dynamic", "type": value,
            "prefix": _prefix(datatype, dataref), "encoding": _encoding(datatype, dataref),
        }
    if kind == "union":
        members = []
        for member in datatype.get("members") or []:
            width = SCALAR_WIDTHS.get(member.get("type"))
            if width is None:
                raise ModelError(f"union '{dataref}' has member of unknown width {member.get('type')!r}")
            members.append({"index": member.get("index"), "width": width})
        if not members:
            raise ModelError(f"union '{dataref}' declares no members")
        # One member per width, for correcting an unknown type tag. The first
        # declared wins, so the choice is the model's order, not an accident.
        widths: dict = {}
        for member in members:
            widths.setdefault(member["width"], member["index"])
        return {
            "tag": tag,
            "kind": "union",
            "type": value,
            "members": members,
            "widths": [{"width": width, "index": index} for width, index in sorted(widths.items())],
        }
    if kind == "enum":
        entries = datatype.get("entries") or []
        if not entries:
            raise ModelError(f"enum '{dataref}' declares no entries")
        base = (datatype.get("base_type") or {}).get("type")
        return {
            "tag": tag, "kind": "enum", "type": _cpp(base),
            "max": max(int(entry["value"]) for entry in entries),
        }
    if kind == "bitfield" or kind in _STREAMED:
        return None

    raise ModelError(f"datatype kind {kind!r} of '{dataref}' has no codec rule")


#: The framework's codec::Encoding for each encoding the model can declare.
_ENCODINGS = {
    "UTF-8": "Utf8",
    "UTF-16BE": "Utf16Be",
    "UTF-16LE": "Utf16Le",
}


def _encoding(datatype: dict, dataref: str) -> str:
    encoding = datatype.get("encoding")
    if encoding not in _ENCODINGS:
        raise ModelError(f"string '{dataref}' has unsupported encoding {encoding!r}")
    return _ENCODINGS[encoding]


def _prefix(dimension: dict, dataref: str) -> str:
    bits = dimension.get("length_of_length_field")
    prefix = {8: "uint8_t", 16: "uint16_t", 32: "uint32_t"}.get(bits)
    if prefix is None:
        raise ModelError(f"'{dataref}' has an unsupported length_of_length_field {bits!r}")
    return prefix


def _cpp(scalar: str | None) -> str:
    if scalar not in SCALARS:
        raise ModelError(f"{scalar!r} is not a scalar")
    return SCALARS[scalar]
