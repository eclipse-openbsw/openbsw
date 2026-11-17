# *******************************************************************************
# Copyright (c) 2026 BMW AG
#
# This program and the accompanying materials are made available under the
# terms of the Apache License Version 2.0 which is available at
# https://www.apache.org/licenses/LICENSE-2.0
#
# SPDX-License-Identifier: Apache-2.0
# *******************************************************************************

"""Loads the ``typeModules`` of the deployment model.

:func:`check_type_modules` validates the modules and the references to them
(run it on the schema-validated model). :func:`load_type_modules` then applies
the defaults the datatypes template relies on and resolves every type reference
of the modules to a :class:`TypeRef`.

A reference is resolved like a C++ name: relative to the namespace it is used
in, then to each enclosing namespace, and finally as a fully qualified name. A
reference that names no declared datatype is a builtin or a C++ type that is
written as is.
"""

import re
from dataclasses import dataclass

from filters import CPP_TYPE_MAP

# Order in which datatypes.h.jinja emits the kinds; a type may only use earlier ones.
EMISSION_ORDER = (
    "AliasTypeDef",
    "EnumTypeDef",
    "StringTypeDef",
    "StructTypeDef",
    "ArrayTypeDef",
)

# C++ primitive spelling -> IDL name (first IDL name wins, e.g. float -> Float).
_CPP_TO_IDL: dict[str, str] = {}
for _idl, _cpp in CPP_TYPE_MAP.items():
    _CPP_TO_IDL.setdefault(_cpp, _idl)

_QUALIFIED_IDENTIFIER = re.compile(r"(::)?[A-Za-z_]\w*(::[A-Za-z_]\w*)*")


@dataclass(frozen=True)
class TypeRef:
    """Reference to a type as the templates see it."""

    name: str
    namespace: str = ""
    is_builtin: bool = False
    # Verbatim C++ spelling for types the YAML writes as C++ (see filters.cpp_type).
    cpp: str | None = None


def _parts(name: str) -> tuple[str, ...]:
    return tuple(part for part in name.split("::") if part)


class TypeRegistry:
    """The datatypes declared in the type modules, by qualified name."""

    def __init__(self, modules: list) -> None:
        self.errors: list[str] = []
        self._entries: dict[tuple[str, ...], tuple[TypeRef, dict, dict]] = {}
        for module in modules:
            namespace = _parts(module["namespace"])
            for type_def in module["types"]:
                key = namespace + (type_def["name"],)
                if key in self._entries:
                    self.errors.append(f"typeModules: duplicate type {'::'.join(key)}")
                    continue
                ref = TypeRef(name=type_def["name"], namespace="::".join(namespace))
                self._entries[key] = (ref, module, type_def)

    def find(self, reference: str, scope: str) -> tuple[TypeRef, dict, dict] | None:
        """Return (type, module, definition) of the datatype *reference* names in *scope*."""
        if not _QUALIFIED_IDENTIFIER.fullmatch(reference):
            return None
        parts = _parts(reference)
        if reference.startswith("::"):
            candidates = [parts]
        else:
            scope_parts = _parts(scope)
            candidates = [
                scope_parts[:depth] + parts for depth in range(len(scope_parts), -1, -1)
            ]
        for candidate in candidates:
            if candidate in self._entries:
                return self._entries[candidate]
        return None

    def resolve(self, reference: str, scope: str) -> TypeRef | None:
        """Return the datatype *reference* names in *scope*, or None."""
        found = self.find(reference, scope)
        return found[0] if found else None


def make_type_ref(
    cpp: str | None,
    fallback_name: str = "",
    registry: TypeRegistry | None = None,
    scope: str = "",
) -> TypeRef:
    """Build a :class:`TypeRef` from a type string of the deployment YAML.

    ``name`` is always usable as an identifier root because the templates derive
    parameter names from it; ``fallback_name`` is used for types that are not a
    plain (qualified) identifier, e.g. ``::etl::array<uint8_t, 4>``.
    """
    if not cpp:
        return TypeRef(name="void")
    if cpp in _CPP_TO_IDL:
        return TypeRef(name=_CPP_TO_IDL[cpp], is_builtin=True)
    if registry is not None:
        declared = registry.resolve(cpp, scope)
        if declared is not None:
            return declared
    if _QUALIFIED_IDENTIFIER.fullmatch(cpp):
        return TypeRef(name=cpp.rsplit("::", 1)[-1], cpp=cpp)
    return TypeRef(name=fallback_name, cpp=cpp)


def _module_references(module: dict):
    """Yield (definition, container, key) for each type reference in *module*."""
    for type_def in module["types"]:
        kind = type_def["typeName"]
        if kind == "AliasTypeDef":
            yield type_def, type_def, "targetType"
        elif kind == "ArrayTypeDef":
            yield type_def, type_def, "elementType"
        elif kind == "StructTypeDef":
            for element in type_def["elements"]:
                yield type_def, element, "type"


def _service_references(service: dict):
    for attribute in service.get("attributes") or []:
        yield attribute["type"]
    for method in service.get("methods") or []:
        for arg in (method.get("inArgs") or []) + (method.get("outArgs") or []):
            yield arg["type"]
    for broadcast in service.get("broadcasts") or []:
        if broadcast.get("type"):
            yield broadcast["type"]


def check_type_modules(data: dict) -> list[str]:
    """Check the type modules and the references to them; return the errors."""
    modules = data.get("typeModules") or []
    registry = TypeRegistry(modules)
    errors = list(registry.errors)

    headers = [module["header"] for module in modules]
    for header in sorted({h for h in headers if headers.count(h) > 1}):
        errors.append(f"typeModules: header {header} is used by several modules")

    for module in modules:
        header = module["header"]
        rank = {
            id(type_def): (EMISSION_ORDER.index(type_def["typeName"]), position)
            for position, type_def in enumerate(module["types"])
        }
        for owner, container, key in _module_references(module):
            found = registry.find(container[key], module["namespace"])
            if found is None:
                continue
            _, target_module, target = found
            if target_module is module:
                if rank[id(target)] >= rank[id(owner)]:
                    errors.append(
                        f"typeModule {header}: {owner['name']} uses {target['name']}, "
                        f"which is emitted after it (order: {', '.join(EMISSION_ORDER)})"
                    )
            elif target_module["header"] not in (module.get("includes") or []):
                errors.append(
                    f"typeModule {header}: {owner['name']} uses {target['name']} "
                    f"but does not include {target_module['header']}"
                )

    for service in data.get("services") or []:
        includes = service.get("typeIncludes") or []
        for reference in _service_references(service):
            found = registry.find(reference, service["namespace"])
            if found is not None and found[1]["header"] not in includes:
                errors.append(
                    f"service {service['name']}: uses {reference} but does not "
                    f"list {found[1]['header']} in typeIncludes"
                )
    return errors


def _default_min_size(entry: dict) -> None:
    # A missing minSize means a fixed size; the templates compare it with maxSize.
    entry.setdefault("maxSize", None)
    if entry.get("minSize") is None:
        entry["minSize"] = entry["maxSize"]


def _apply_defaults(module: dict) -> None:
    for type_def in module["types"]:
        kind = type_def["typeName"]
        if kind == "EnumTypeDef":
            for enumerator in type_def["enumerators"]:
                enumerator.setdefault("value", None)
        elif kind == "ArrayTypeDef":
            _default_min_size(type_def)
        elif kind == "StructTypeDef":
            for element in type_def["elements"]:
                element.setdefault("array", False)
                if element["array"]:
                    _default_min_size(element)


def load_type_modules(data: dict) -> TypeRegistry:
    """Prepare the type modules of *data* in place and return their registry."""
    modules = data.get("typeModules") or []
    registry = TypeRegistry(modules)
    for module in modules:
        _apply_defaults(module)
        for _, container, key in _module_references(module):
            container[key] = make_type_ref(
                container[key], registry=registry, scope=module["namespace"]
            )
    return registry
