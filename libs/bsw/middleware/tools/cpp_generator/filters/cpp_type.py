# *******************************************************************************
# Copyright (c) 2026 BMW AG
#
# This program and the accompanying materials are made available under the
# terms of the Apache License Version 2.0 which is available at
# https://www.apache.org/licenses/LICENSE-2.0
#
# SPDX-License-Identifier: Apache-2.0
# *******************************************************************************

"""Jinja filter ``cppType``."""

# Platform-neutral IDL type names -> C++ types.
CPP_TYPE_MAP = {
    "UInt8": "uint8_t",
    "UInt16": "uint16_t",
    "UInt32": "uint32_t",
    "UInt64": "uint64_t",
    "Int8": "int8_t",
    "Int16": "int16_t",
    "Int32": "int32_t",
    "Int64": "int64_t",
    "Boolean": "bool",
    "Float": "float",
    "Float16": "float",
    "Float32": "float",
    "Float64": "float",
    "Double": "double",
    "ByteBuffer": "etl::span<uint8_t>",
}


def _is_type_ref(value):
    return hasattr(value, "name") and hasattr(value, "namespace")


def _is_builtin(type_ref):
    return bool(getattr(type_ref, "is_builtin", getattr(type_ref, "isBuiltin", False)))


def _namespace_parts(namespace):
    return [part for part in (namespace or "").split("::") if part]


def relative_name(name, namespace, current_namespace):
    """Name of ``namespace::name`` as written from code inside ``current_namespace``.

    The common leading namespaces are dropped because C++ finds a name in any
    enclosing namespace, e.g. ``org::bar::foo::T`` seen from ``org::bar`` is
    ``foo::T``.
    """
    type_parts = _namespace_parts(namespace)
    current_parts = _namespace_parts(current_namespace)
    common = 0
    while (
        common < min(len(type_parts), len(current_parts))
        and type_parts[common] == current_parts[common]
    ):
        common += 1
    return "::".join(type_parts[common:] + [name])


def cpp_type(value, current_namespace=""):
    """Convert an IDL type to its C++ representation.

    A type reference (object with ``name``, ``namespace`` and optional
    ``is_builtin``) is mapped to a C++ primitive if it is a builtin, otherwise
    written relative to ``current_namespace`` (see :func:`relative_name`). A
    non-empty ``cpp`` attribute is returned verbatim instead. Any other value is
    looked up by its string form in ``CPP_TYPE_MAP`` and returned unchanged when
    absent. ``None`` yields ``None``.
    """
    if value is None:
        return None

    if _is_type_ref(value):
        if getattr(value, "cpp", None):
            return value.cpp
        if _is_builtin(value) and value.name in CPP_TYPE_MAP:
            return CPP_TYPE_MAP[value.name]
        return relative_name(value.name, value.namespace, current_namespace)

    name = str(value)
    return CPP_TYPE_MAP.get(name, name)
