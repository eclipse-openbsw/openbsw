# *******************************************************************************
# Copyright (c) 2026 BMW AG
#
# This program and the accompanying materials are made available under the
# terms of the Apache License Version 2.0 which is available at
# https://www.apache.org/licenses/LICENSE-2.0
#
# SPDX-License-Identifier: Apache-2.0
# *******************************************************************************

"""Bazel rule for generating routing and blob configuration headers."""

load("@rules_cc//cc/common:cc_common.bzl", "cc_common")
load("@rules_cc//cc/common:cc_info.bzl", "CcInfo")

def _generate_routing_impl(ctx):
    generated_dir = "/".join([
        part
        for part in [
            ctx.bin_dir.path,
            ctx.label.workspace_root,
            ctx.label.package,
            ctx.label.name,
            "generated",
        ]
        if part != ""
    ])
    blob_headers = [
        ctx.actions.declare_file(ctx.attr.name + "/generated/blob/ConfigType.h"),
        ctx.actions.declare_file(ctx.attr.name + "/generated/blob/configuration.h"),
    ]
    routing_headers = [
        ctx.actions.declare_file(ctx.attr.name + "/generated/routing/channelId.h"),
        ctx.actions.declare_file(ctx.attr.name + "/generated/routing/constants.h"),
    ]
    headers = blob_headers + routing_headers

    args = ctx.actions.args()
    args.add(ctx.file.jsonl)
    args.add(generated_dir + "/blob")
    args.add(generated_dir + "/routing")

    ctx.actions.run(
        inputs = [ctx.file.jsonl],
        tools = [ctx.attr._generator[DefaultInfo].files_to_run],
        outputs = headers,
        executable = ctx.executable._generator,
        arguments = [args],
        mnemonic = "GenerateRouting",
        progress_message = "Generating routing configuration for %{label}",
    )

    compilation_context = cc_common.create_compilation_context(
        headers = depset(headers),
        includes = depset([generated_dir]),
    )

    return [
        DefaultInfo(files = depset(headers)),
        CcInfo(compilation_context = compilation_context),
    ]

generate_routing = rule(
    implementation = _generate_routing_impl,
    doc = "Generates blob and routing C++ headers from a routing JSONL file.",
    attrs = {
        "jsonl": attr.label(
            mandatory = True,
            allow_single_file = [".jsonl"],
            doc = "Routing configuration input.",
        ),
        "_generator": attr.label(
            default = Label("//tools/blob:generate_routing"),
            cfg = "exec",
            executable = True,
        ),
    },
)
