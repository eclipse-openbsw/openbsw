# *******************************************************************************
# Copyright (c) 2026 BMW AG
#
# This program and the accompanying materials are made available under the
# terms of the Apache License Version 2.0 which is available at
# https://www.apache.org/licenses/LICENSE-2.0
#
# SPDX-License-Identifier: Apache-2.0
# *******************************************************************************

"""Custom Jinja filters used by the middleware templates."""

from jinja2 import Environment

from .camel_case import camel_case
from .cpp_type import CPP_TYPE_MAP, cpp_type
from .pascal_case import pascal_case

__all__ = ["CPP_TYPE_MAP", "camel_case", "cpp_type", "pascal_case", "register_filters"]


def register_filters(env: Environment) -> None:
    """Register all custom filters on *env*."""
    env.filters["camelCase"] = camel_case
    env.filters["pascalCase"] = pascal_case
    env.filters["cppType"] = cpp_type
