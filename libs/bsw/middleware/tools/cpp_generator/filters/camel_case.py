# *******************************************************************************
# Copyright (c) 2026 BMW AG
#
# This program and the accompanying materials are made available under the
# terms of the Apache License Version 2.0 which is available at
# https://www.apache.org/licenses/LICENSE-2.0
#
# SPDX-License-Identifier: Apache-2.0
# *******************************************************************************

"""Jinja filter ``camelCase``."""


def camel_case(value):
    """Lower-case the first character, leaving the rest unchanged.

    ``"GetSpeed"`` -> ``"getSpeed"``, ``"ABC"`` -> ``"aBC"``, ``""`` -> ``""``.
    ``None`` is returned unchanged.
    """
    if value is None:
        return None
    s = str(value)
    return s[:1].lower() + s[1:]
