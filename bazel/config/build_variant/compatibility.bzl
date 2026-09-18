# *******************************************************************************
# Copyright (c) 2026 Accenture
#
# This program and the accompanying materials are made available under the
# terms of the Apache License Version 2.0 which is available at
# https://www.apache.org/licenses/LICENSE-2.0
#
# SPDX-License-Identifier: Apache-2.0
# *******************************************************************************

"""Build variant compatibility helpers.

Analogous to Pigweed's incompatible_with_mcu() pattern. Each helper wraps a
select() so call sites stay on one line regardless of which variant they target.
"""

def unit_test_only():
    """Compatible only with the unit_test build variant."""
    return select({
        "//bazel/config/build_variant:unit_test": [],
        "//conditions:default": ["@platforms//:incompatible"],
    })

def reference_app_only():
    """Compatible only with the reference_app build variant."""
    return select({
        "//bazel/config/build_variant:reference_app": [],
        "//conditions:default": ["@platforms//:incompatible"],
    })

def reference_app_posix_only():
    """Compatible only with the reference_app build variant on a posix host."""
    return select({
        "@platforms//os:none": ["@platforms//:incompatible"],
        "//conditions:default": [],
    }) + reference_app_only()
