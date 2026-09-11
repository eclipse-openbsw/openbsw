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

`middleware_codegen` runs `jinja2cpp.py` over a deployment YAML model and wraps
the generated C++ in a `cc_library`. It models the `add_custom_command`
+ `add_library` pair in `middlewareConfiguration/CMakeLists.txt` on the Cmake side.

The set of generated files is data-dependent and varies by the deployment
model file, so callers pass `generated_outputs`: the list of output paths relative
to the generation output base, exactly as printed by
`jinja2cpp.py --list-outputs`. That list is checked in (e.g. a
`generated_outputs.bzl`) and verified against the generator output at build time.
"""

load("@bazel_skylib//rules:diff_test.bzl", "diff_test")
load("@bazel_skylib//rules:write_file.bzl", "write_file")
load("@rules_cc//cc:cc_library.bzl", "cc_library")

# The generator package: script, templates and schemas are staged via this
# filegroup, and the script/input dir are referenced by their execroot paths.
_GENERATOR = "//libs/bsw/middleware/tools/cpp_generator:generator"
_GENERATOR_INPUT = "libs/bsw/middleware/tools/cpp_generator"
_GENERATOR_SCRIPT = _GENERATOR_INPUT + "/jinja2cpp.py"

# Dev-container venv interpreter (has jinja2/jsonschema/PyYAML per
# docker/development/files/requirements.lock). Absolute path by design.
_PYTHON = "/opt/venv/bin/python3"

# Dev-container clang-format-17 install (no unversioned `clang-format` symlink
# exists, so a PATH lookup fails). Absolute path by design, for the same
# hermeticity reason as _PYTHON: this keeps the genrule's tool resolution
# independent of the invoking shell's PATH.
_CLANG_FORMAT = "/usr/bin/clang-format-17"

def middleware_codegen(
        name,
        deployment_yaml,
        generated_outputs,
        deps = None,
        visibility = None):
    """Generate middleware C++ from `deployment_yaml` and expose it as a cc_library.

    Args:
      name: Name of the generated `cc_library`. The backing genrule is
        `<name>_srcs`.
      deployment_yaml: Label of the deployment YAML model (single source of truth).
      generated_outputs: List of output paths relative to the generation output
        base (as emitted by `jinja2cpp.py --list-outputs`). `.h`/`.hpp` entries
        become `hdrs`, `.cpp` entries become `srcs`.
      deps: Extra `cc_library` deps the generated code needs to compile
        (e.g. `//libs/bsw/middleware`).
      visibility: Visibility of the generated `cc_library`.
    """
    gen_root = name + "_generated"
    outs = [gen_root + "/" + path for path in generated_outputs]

    native.genrule(
        name = name + "_srcs",
        srcs = [deployment_yaml, _GENERATOR],
        outs = outs,
        cmd = ("{python} {script} --input {input} --output $(RULEDIR)/{gen_root}" +
               " --deployment-yaml $(execpath {deployment}) --clang-format-binary {clang_format}").format(
            python = _PYTHON,
            script = _GENERATOR_SCRIPT,
            input = _GENERATOR_INPUT,
            gen_root = gen_root,
            deployment = deployment_yaml,
            clang_format = _CLANG_FORMAT,
        ),
        message = "Generating middleware C++ code from " + deployment_yaml,
    )

    hdrs = [out for out in outs if out.endswith(".h") or out.endswith(".hpp")]
    srcs = [out for out in outs if out.endswith(".cpp")]

    cc_library(
        name = name,
        srcs = srcs,
        hdrs = hdrs,
        strip_include_prefix = gen_root + "/include/generated_code",
        # alwayslink is required because the generated .cpp files implement
        # symbols declared inside //libs/bsw/middleware, e.g.
        # AllocatorSelectorDefinitions.cpp implements middleware::memory::getAllocFunction().
        alwayslink = True,
        deps = deps or [],
        visibility = visibility,
    )

    write_file(
        name = name + "_expected_outputs",
        out = name + "_expected_outputs.txt",
        content = sorted(generated_outputs) + [""],
        newline = "unix",
    )

    native.genrule(
        name = name + "_actual_outputs",
        srcs = [deployment_yaml, _GENERATOR],
        outs = [name + "_actual_outputs.txt"],
        cmd = ("{python} {script} --input {input}" +
               " --deployment-yaml $(execpath {deployment})" +
               " --list-outputs > $@").format(
            python = _PYTHON,
            script = _GENERATOR_SCRIPT,
            input = _GENERATOR_INPUT,
            deployment = deployment_yaml,
        ),
    )

    diff_test(
        name = name + "_drift_test",
        failure_message = (
            name + ": generated_outputs.bzl is stale. " +
            "See " + native.package_name() + "/generated_outputs.bzl for regeneration instructions."
        ),
        file1 = name + "_expected_outputs.txt",
        file2 = name + "_actual_outputs.txt",
    )
