# *******************************************************************************
# Copyright (c) 2026 BMW AG
#
# This program and the accompanying materials are made available under the
# terms of the Apache License Version 2.0 which is available at
# https://www.apache.org/licenses/LICENSE-2.0
#
# SPDX-License-Identifier: Apache-2.0
# *******************************************************************************

"""Bazel rule for generating middleware C++ sources from deployment models."""

def _run_codegen_impl(ctx):
    outputs = [ctx.actions.declare_file(path) for path in ctx.attr.outs]
    generated_prefix = ctx.attr.generated_files_prefix
    if generated_prefix and not generated_prefix.endswith("/"):
        generated_prefix += "/"

    output_directory = ctx.bin_dir.path + "/" + ctx.label.package + "/" + generated_prefix
    ctx.actions.run(
        executable = ctx.executable.tool,
        arguments = [
            "--output",
            output_directory,
            "--deployment-yaml",
            ctx.file.deployment.path,
            "--clang-format-config",
            ctx.file.clang_format_config.path,
        ],
        tools = [ctx.executable.tool],
        inputs = depset([ctx.file.deployment, ctx.file.clang_format_config]),
        outputs = outputs,
        mnemonic = "RunCodegen",
        progress_message = "Running code generation for {}".format(ctx.label.name),
    )

    generated_shm_hdrs = [
        output
        for output in outputs
        if output.extension == "h" and "/shm/" in output.short_path
    ]
    generated_shm_srcs = [
        output
        for output in outputs
        if output.extension == "cpp" and "/shm/" in output.short_path
    ] + [
        output
        for output in outputs
        if output.basename == "AllocatorSelectorDefinitions.cpp"
    ]
    return [
        DefaultInfo(files = depset(outputs)),
        OutputGroupInfo(
            generated_headers = depset([
                output
                for output in outputs
                if output.extension == "h"
            ]),
            generated_sources = depset([
                output
                for output in outputs
                if output.extension == "cpp"
            ]),
            generated_service_hdrs = depset([
                output
                for output in outputs
                if output.extension == "h" and output not in generated_shm_hdrs
            ]),
            generated_service_srcs = depset([
                output
                for output in outputs
                if output.extension == "cpp" and output not in generated_shm_srcs
            ]),
            generated_shm_hdrs = depset(generated_shm_hdrs),
            generated_shm_srcs = depset(generated_shm_srcs),
        ),
    ]

run_codegen = rule(
    implementation = _run_codegen_impl,
    attrs = {
        "clang_format_config": attr.label(
            allow_single_file = True,
            default = Label("//:clang_format_config"),
        ),
        "deployment": attr.label(
            mandatory = True,
            allow_single_file = True,
        ),
        "generated_files_prefix": attr.string(default = ""),
        "outs": attr.string_list(mandatory = True),
        "tool": attr.label(
            cfg = "exec",
            executable = True,
            mandatory = True,
        ),
    },
)
