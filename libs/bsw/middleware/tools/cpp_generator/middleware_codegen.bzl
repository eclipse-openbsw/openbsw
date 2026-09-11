# *******************************************************************************
# Copyright (c) 2026 Accenture
#
# This program and the accompanying materials are made available under the
# terms of the Apache License Version 2.0 which is available at
# https://www.apache.org/licenses/LICENSE-2.0
#
# SPDX-License-Identifier: Apache-2.0
# *******************************************************************************

"""Middleware C++ code generation macro.

`middleware_codegen` runs `jinja2cpp.py` over a deployment YAML model and exposes
the generated C++ as a `cc_library`. Callers provide `generated_outputs`: the list
of output paths as printed by `jinja2cpp.py --list-outputs`, which is checked in
and verified against actual generator output at build time.
"""

load("@bazel_skylib//lib:shell.bzl", "shell")
load("@bazel_skylib//rules:diff_test.bzl", "diff_test")
load("@bazel_skylib//rules:write_file.bzl", "write_file")
load("@rules_cc//cc:cc_library.bzl", "cc_library")

_GENERATOR_TOOL = "//libs/bsw/middleware/tools/cpp_generator:jinja2cpp"

# Templates and schemas staged as inputs so jinja2cpp.py can find them
# at their source-tree paths inside the sandbox.
_GENERATOR_TEMPLATES = "//libs/bsw/middleware/tools/cpp_generator:generator_templates"

# Path passed to --input, which must match the package path of the generator so
# jinja2cpp.py can locate templates/jinja/ and templates/schemas/ below it.
_GENERATOR_INPUT = "libs/bsw/middleware/tools/cpp_generator"

def _codegen_srcs_impl(ctx):
    outs = ctx.outputs.outs
    output_dir = "/".join([ctx.bin_dir.path, ctx.label.package, ctx.attr.gen_root])

    args = ctx.actions.args()
    args.add("--input", _GENERATOR_INPUT)
    args.add("--output", output_dir)
    args.add("--deployment-yaml", ctx.file.deployment_yaml)

    ctx.actions.run(
        executable = ctx.executable._jinja2cpp,
        inputs = depset([ctx.file.deployment_yaml] + ctx.files._generator_templates),
        outputs = outs,
        arguments = [args],
        mnemonic = "MiddlewareCodegen",
        progress_message = "Generating middleware C++ from " + ctx.file.deployment_yaml.short_path,
    )
    return [DefaultInfo(files = depset(outs))]

_codegen_srcs = rule(
    implementation = _codegen_srcs_impl,
    attrs = {
        "_jinja2cpp": attr.label(
            default = _GENERATOR_TOOL,
            executable = True,
            cfg = "exec",
        ),
        "_generator_templates": attr.label(default = _GENERATOR_TEMPLATES),
        "deployment_yaml": attr.label(allow_single_file = True, mandatory = True),
        "gen_root": attr.string(mandatory = True),
        "outs": attr.output_list(),
    },
)

def _list_outputs_impl(ctx):
    out = ctx.outputs.out

    ctx.actions.run_shell(
        # Stages the py_binary runfiles (interpreter, pip packages) in the sandbox.
        tools = [ctx.attr._jinja2cpp[DefaultInfo].files_to_run],
        inputs = depset([ctx.file.deployment_yaml] + ctx.files._generator_templates),
        outputs = [out],
        command = "{tool} --input {input} --deployment-yaml {deployment} --list-outputs > {out}".format(
            tool = shell.quote(ctx.executable._jinja2cpp.path),
            input = shell.quote(_GENERATOR_INPUT),
            deployment = shell.quote(ctx.file.deployment_yaml.path),
            out = shell.quote(out.path),
        ),
        progress_message = "Listing codegen outputs for " + ctx.file.deployment_yaml.short_path,
    )
    return [DefaultInfo(files = depset([out]))]

_list_outputs = rule(
    implementation = _list_outputs_impl,
    attrs = {
        "_jinja2cpp": attr.label(
            default = _GENERATOR_TOOL,
            executable = True,
            cfg = "exec",
        ),
        "_generator_templates": attr.label(default = _GENERATOR_TEMPLATES),
        "deployment_yaml": attr.label(allow_single_file = True, mandatory = True),
        "out": attr.output(),
    },
)

def middleware_codegen(
        name,
        deployment_yaml,
        generated_outputs,
        deps = [],
        visibility = None):
    """Generate middleware C++ from `deployment_yaml` and expose it as a cc_library.

    Args:
      name: Name of the generated `cc_library`.
      deployment_yaml: Label of the deployment YAML model (single source of truth).
      generated_outputs: List of output paths relative to the generation output
        base (as emitted by `jinja2cpp.py --list-outputs`). `.h` entries become
        `hdrs`, `.cpp` entries become `srcs`.
      deps: Extra `cc_library` deps the generated code needs to compile
        (e.g. `//libs/bsw/middleware`).
      visibility: Visibility of the generated `cc_library`.
    """
    gen_root = name + "_generated"
    outs = [gen_root + "/" + path for path in generated_outputs]
    hdrs = [f for f in outs if f.endswith(".h")]
    srcs = [f for f in outs if f.endswith(".cpp")]

    _codegen_srcs(
        name = name + "_srcs",
        deployment_yaml = deployment_yaml,
        gen_root = gen_root,
        outs = outs,
    )

    cc_library(
        name = name,
        srcs = srcs,
        hdrs = hdrs,
        strip_include_prefix = gen_root + "/include/generated_code",
        # alwayslink is required because the generated .cpp files implement
        # symbols declared inside //libs/bsw/middleware, e.g.
        # AllocatorSelectorDefinitions.cpp implements middleware::memory::getAllocFunction().
        alwayslink = True,
        deps = deps,
        visibility = visibility,
    )

    write_file(
        name = name + "_expected_outputs",
        out = name + "_expected_outputs.txt",
        content = sorted(generated_outputs) + [""],
        newline = "unix",
    )

    _list_outputs(
        name = name + "_actual_outputs",
        deployment_yaml = deployment_yaml,
        out = name + "_actual_outputs.txt",
    )

    # Catches stale generated_outputs.bzl at build time.
    diff_test(
        name = name + "_drift_test",
        failure_message = (
            name + ": generated_outputs.bzl is stale. " +
            "See " + native.package_name() + "/generated_outputs.bzl for regeneration instructions."
        ),
        file1 = name + "_expected_outputs.txt",
        file2 = name + "_actual_outputs.txt",
    )
