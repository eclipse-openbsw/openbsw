# *******************************************************************************
# Copyright (c) 2026 KPIT Technologies Ltd.
#
# This program and the accompanying materials are made available under the
# terms of the Apache License Version 2.0 which is available at
# https://www.apache.org/licenses/LICENSE-2.0
#
# SPDX-License-Identifier: Apache-2.0
# *******************************************************************************

"""flync names and datatypes -> C++ names and types, plus the typedefs the
deployment needs.

A dataref names a datatype declared inline in a service file (see model.py).
Scalars map straight onto a C++ fundamental type. Anything with a shape -- an
array, a string, a union -- becomes an etl container, which the deployment
cannot spell inline because the middleware templates use the type name as an
identifier; those get a typedef, registered here and emitted into the
service's `typedefs`.

Deliberately NOT represented: the wire rules that ride along with a type
(an enum's valid range, a union's member table, a dynamic array's
length-prefix width, a string's encoding). Those belong to the SOME/IP codec,
not to the middleware, which only ever moves a C++ object between two
clusters. They stay in flync and are read by codecs.py instead.
"""

import re

# ---------------------------------------------------------------------------
# flync name -> C++ identifier
#
# flync names members the way the wire model does (readUINT8Array,
# WindowPOSITION). The deployment needs two spellings of each: `name`, which
# the generator uses for C++ class members and template ids, and `variable`,
# the lowerCamel form used for the generated data member.
#
# `name` keeps the flync spelling with the first letter capitalised, so an id
# stays recognisable against the wire model. `variable` de-shouts it:
# UINT8 -> Uint8, POSITION -> Position.
# ---------------------------------------------------------------------------

#: One word of a camelCase name. The first alternative takes an all-caps run
#: with any trailing digits, but only up to a letter that starts a new word:
#: without the lookahead, `readUINT8Array` would be read as one run `UINT8A`
#: and come out `readUint8array`.
_WORD = re.compile(r"[A-Z]+[0-9]*(?![a-z])|[A-Za-z][a-z0-9]*")


def class_name(flync_name: str) -> str:
    """`readVIN` -> `ReadVIN`; an already-capitalised name is unchanged."""
    if not flync_name:
        return flync_name
    return flync_name[0].upper() + flync_name[1:]


def variable_name(flync_name: str) -> str:
    """`readUINT8Array` -> `readUint8Array`, `WindowPOSITION` -> `windowPosition`.

    E2E survives without a special case: it splits into the words E2 and E,
    each already title-case, and rejoins unchanged.
    """
    if not flync_name:
        return flync_name

    words = _WORD.findall(flync_name)
    if not words:
        return flync_name

    # Only an all-caps word is shouting; a word that is already mixed case was
    # written that way on purpose.
    titled = [word[0].upper() + word[1:].lower() if word.isupper() else word for word in words]
    joined = "".join(titled)
    return joined[0].lower() + joined[1:]


def cpp_identifier(flync_name: str) -> str:
    """`DOOR_lock_state` -> `DoorLockState`.

    Datatype names are snake_case in flync and become C++ type names in the
    deployment, where the schema allows letters and digits only.
    """
    parts = [part for part in re.split(r"[^A-Za-z0-9]+", flync_name) if part]
    return "".join(part[0].upper() + part[1:].lower() if part.isupper() else part[0].upper() + part[1:]
                   for part in parts)


def namespace_from_stem(stem: str) -> str:
    """`door_control_service` -> `door_control_service`.

    The service file's own name is the only C++ identity flync carries.
    """
    return stem


def service_class_from_stem(stem: str) -> str:
    """`door_control_service` -> `DoorControlService`."""
    return "".join(part[:1].upper() + part[1:] for part in stem.split("_") if part)


# ---------------------------------------------------------------------------
# flync datatype -> C++ type
# ---------------------------------------------------------------------------

#: flync scalar datatype -> C++ type. `bool` rather than `uint8_t` for
#: boolean: the middleware serialises by object, so the C++ type is the model.
SCALARS = {
    "boolean": "bool",
    "uint8": "uint8_t",
    "uint16": "uint16_t",
    "uint32": "uint32_t",
    "uint64": "uint64_t",
    "int8": "int8_t",
    "int16": "int16_t",
    "int32": "int32_t",
    "int64": "int64_t",
    "float32": "float",
    "float64": "double",
}

#: Byte width of each flync scalar on the wire.
SCALAR_WIDTHS = {
    "boolean": 1,
    "uint8": 1,
    "int8": 1,
    "uint16": 2,
    "int16": 2,
    "uint32": 4,
    "int32": 4,
    "float32": 4,
    "uint64": 8,
    "int64": 8,
    "float64": 8,
}

#: Character width of a string encoding, in bytes. A fixed_length_string's
#: `length` counts characters, the etl::array counts bytes.
_ENCODING_WIDTH = {
    "UTF-8": 1,
    "UTF-16BE": 2,
    "UTF-16LE": 2,
}

#: Capacity given to a dynamic array or string that names no upper_limit.
#: flync leaves those unbounded; the middleware is allocation-free and needs
#: a number, so one is supplied here rather than silently guessed per call
#: site. Override per datatype with `max_elements` in the middleware overlay.
DEFAULT_DYNAMIC_CAPACITY = 64


class TypeError_(Exception):
    """A flync datatype this translator cannot express as a C++ type."""


class TypeRegistry:
    """Collects the typedefs and structs one service needs.

    Typedefs are keyed by datatype, so each member's type names the model
    datatype it came from. Structs are merged by content instead, because a
    struct has no model name of its own.
    """

    def __init__(self, datatypes: dict, capacity_overrides: dict | None = None):
        self._datatypes = datatypes
        self._capacities = capacity_overrides or {}
        self._typedefs: dict[str, str] = {}  # typedef name -> cpp spelling
        self._structs: dict[str, list] = {}  # struct name -> parameters

    # ---- emitted sections -------------------------------------------------

    def typedefs(self) -> list[dict]:
        return [{"name": name, "type": spelling} for name, spelling in sorted(self._typedefs.items())]

    def structs(self) -> list[dict]:
        return [
            {"name": name, "type": "struct", "parameters": parameters}
            for name, parameters in self._structs.items()
        ]

    # ---- resolution -------------------------------------------------------

    def resolve(self, dataref: str) -> str:
        """Return the C++ type name for one flync dataref."""
        entry = self._datatypes.get(dataref)
        if entry is None:
            raise TypeError_(f"dataref '{dataref}' is not defined in data.flync.yaml")

        config = entry.get("config") or {}
        datatype = config.get("datatype")

        dimensions = config.get("dimensions")
        if dimensions:
            return self._array(dataref, config, dimensions)

        if datatype in SCALARS:
            return SCALARS[datatype]
        if datatype == "enum":
            # The middleware carries the base type; the 0..3 range is a codec
            # rule, enforced on the wire, not by the C++ type.
            base = (config.get("base_type") or {}).get("datatype")
            if base not in SCALARS:
                raise TypeError_(f"enum '{dataref}' has no scalar base_type")
            return SCALARS[base]
        if datatype == "bitfield":
            length = int(config.get("length", 0))
            width = {8: "uint8_t", 16: "uint16_t", 32: "uint32_t", 64: "uint64_t"}
            if length not in width:
                raise TypeError_(f"bitfield '{dataref}' has unsupported length {length}")
            return width[length]
        if datatype == "union":
            # An opaque buffer: the middleware moves the bytes, the codec
            # interprets the member table. Width = the widest member plus the
            # 4-byte length and 4-byte type tag SOME/IP puts in front.
            widest = max(
                (_scalar_width(member.get("datatype")) for member in config.get("members", [])),
                default=0,
            )
            if widest == 0:
                raise TypeError_(f"union '{dataref}' declares no members")
            return self._typedef(dataref, f"::etl::vector<uint8_t, {widest + 8}>")
        if datatype == "dynamic_length_string":
            capacity = self._capacity(dataref)
            return self._typedef(dataref, f"::etl::vector<uint8_t, {capacity}>")
        if datatype == "fixed_length_string":
            # `length` counts bytes on the wire, BOM and terminator included, not
            # characters: a UTF-16 fixed-64 string occupies 64 bytes. The encoding
            # is checked only so an unknown one fails here rather than on the wire.
            if config.get("encoding") not in _ENCODING_WIDTH:
                raise TypeError_(
                    f"string '{dataref}' has unsupported encoding {config.get('encoding')!r}"
                )
            return self._typedef(dataref, f"::etl::array<uint8_t, {int(config['length'])}>")

        raise TypeError_(f"datatype '{datatype}' of '{dataref}' is not supported")

    def struct(self, name: str, parameters: list[dict]) -> str:
        """Register a struct for a multi-parameter member and return its name.

        A struct identical in every member name and type to one already
        registered is not emitted again; the first registration's name is
        returned. Five trigger methods taking the same (delay, duration,
        debounceTime) share one type rather than producing five that the
        service code would have to handle separately.
        """
        for existing_name, existing in self._structs.items():
            if existing == parameters:
                return existing_name
        if name in self._structs:
            raise TypeError_(f"struct '{name}' defined twice with different parameters")
        self._structs[name] = parameters
        return name

    # ---- internals --------------------------------------------------------

    def _array(self, dataref: str, config: dict, dimensions: list) -> str:
        element = config.get("datatype")
        if element not in SCALARS:
            raise TypeError_(f"array '{dataref}' has non-scalar element type {element!r}")

        spelling = SCALARS[element]
        # Innermost dimension first: flync lists outer-to-inner, and each
        # wrapping is applied around what is already built.
        for dimension in reversed(dimensions):
            kind = str(dimension.get("kind", "")).lower()
            if kind == "fixed":
                spelling = f"::etl::array<{spelling}, {int(dimension['length'])}>"
            elif kind == "dynamic":
                capacity = dimension.get("upper_limit")
                capacity = int(capacity) if capacity else self._capacity(dataref)
                spelling = f"::etl::vector<{spelling}, {capacity}>"
            else:
                raise TypeError_(f"array '{dataref}' has unknown dimension kind {kind!r}")

        return self._typedef(dataref, spelling)

    def _capacity(self, dataref: str) -> int:
        return int(self._capacities.get(dataref, DEFAULT_DYNAMIC_CAPACITY))

    def _typedef(self, dataref: str, spelling: str) -> str:
        """One alias per model datatype, named after it.

        Two datatypes of the same C++ shape get two aliases of one type, which
        C++ allows. Merging them would name every member after whichever
        datatype happened to be registered first: an 8-bit-prefix array would
        be called ...Dyn32, and a UTF-8 string ...Utf16.
        """
        name = cpp_identifier(dataref)
        self._typedefs[name] = spelling
        return name


def _scalar_width(datatype: str | None) -> int:
    return SCALAR_WIDTHS.get(datatype or "", 0)
