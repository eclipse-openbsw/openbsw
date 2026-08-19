# *******************************************************************************
# Copyright (c) 2026 BMW AG
#
# This program and the accompanying materials are made available under the
# terms of the Apache License Version 2.0 which is available at
# https://www.apache.org/licenses/LICENSE-2.0
#
# SPDX-License-Identifier: Apache-2.0
# *******************************************************************************

"""Minimal configurable wrapper for the middleware library.

This wrapper forwards CcInfo from a dependency while allowing a controlled set
of label_flag overrides to be applied on the dependency edge.
"""

load("@rules_cc//cc/common:cc_info.bzl", "CcInfo")

MIDDLEWARE_LABEL_FLAGS = [
    "//libs/3rdparty/etl:etl_profile",
    "//libs/bsw/middleware/interfaces:concurrency_impl",
    "//libs/bsw/middleware/interfaces:logger_impl",
    "//libs/bsw/middleware/interfaces:os_impl",
    "//libs/bsw/middleware/interfaces:time_impl",
]

def _middleware_transition_impl(settings, attr):
    outputs = {
        label_flag: settings[label_flag]
        for label_flag in MIDDLEWARE_LABEL_FLAGS
    }

    for label_flag, value in attr.config.items():
        outputs[label_flag] = value

    return outputs

_middleware_transition = transition(
    implementation = _middleware_transition_impl,
    inputs = MIDDLEWARE_LABEL_FLAGS,
    outputs = MIDDLEWARE_LABEL_FLAGS,
)

def _middleware_configured_library_impl(ctx):
    dep = ctx.attr.dep[0]
    providers = [dep[CcInfo], dep[DefaultInfo]]

    if OutputGroupInfo in dep:
        providers.append(dep[OutputGroupInfo])

    return providers

_middleware_configured_library = rule(
    implementation = _middleware_configured_library_impl,
    attrs = {
        "dep": attr.label(
            cfg = _middleware_transition,
            mandatory = True,
            providers = [CcInfo],
        ),
        "config": attr.string_keyed_label_dict(
            cfg = _middleware_transition,
        ),
        "_allowlist_function_transition": attr.label(
            default = "@bazel_tools//tools/allowlists/function_transition_allowlist",
        ),
    },
)

def middleware_config(name, dep, config = None, **kwargs):
    """Wrap a middleware dependency and optionally override middleware label_flags.

    Args:
        name: Name of the wrapper target.
        dep: Middleware-like dependency that provides CcInfo.
        config: Optional dict mapping supported label_flag labels to targets.
        **kwargs: Forwarded to the underlying rule.
    """
    normalized_config = config or {}

    for key in normalized_config.keys():
        if key not in MIDDLEWARE_LABEL_FLAGS:
            fail("Unsupported middleware config flag: {}".format(key))

    _middleware_configured_library(
        name = name,
        dep = dep,
        config = normalized_config,
        **kwargs
    )
