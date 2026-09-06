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
the generated C++ in a `cc_library`. It replaces the CMake `add_custom_command`
+ `add_library` pair in `middlewareConfiguration/CMakeLists.txt`.

The set of generated files is data-dependent (it varies with the deployment
model), so callers pass `generated_outputs`: the list of output paths relative
to the generation output base, exactly as printed by
`jinja2cpp.py --list-outputs`. That list is checked in (e.g. a
`generated_outputs.bzl`) and verified against the generator output at build time.

Non-parity notes (vs the CMake baseline):
  * The genrule invokes `/opt/venv/bin/python3` by absolute path, the fixed venv
    location in the dev-container image.
  * `alwayslink = True` mirrors the CMake `-Wl,--whole-archive` link of the
    generated configuration into the application.
  * `target_compatible_with = ["@platforms//os:linux"]` mirrors the CMake
    `if(NOT UNIX) return()` guard.
"""

load("@rules_cc//cc:cc_library.bzl", "cc_library")

# The generator package: script + templates + schemas are staged via this
# filegroup, and the script/input dir are referenced by their execroot paths.
_GENERATOR = "//libs/bsw/middleware/tools/cpp_generator:generator"
_GENERATOR_INPUT = "libs/bsw/middleware/tools/cpp_generator"
_GENERATOR_SCRIPT = _GENERATOR_INPUT + "/jinja2cpp.py"

# Dev-container venv interpreter (has jinja2/jsonschema/PyYAML per
# docker/development/files/requirements.lock). Absolute path by design.
_PYTHON = "/opt/venv/bin/python3"

_LINUX_ONLY = ["@platforms//os:linux"]

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
               " --deployment-yaml $(execpath {deployment})").format(
            python = _PYTHON,
            script = _GENERATOR_SCRIPT,
            input = _GENERATOR_INPUT,
            gen_root = gen_root,
            deployment = deployment_yaml,
        ),
        message = "Generating middleware C++ code from " + deployment_yaml,
        target_compatible_with = _LINUX_ONLY,
    )

    hdrs = [o for o in outs if o.endswith(".h") or o.endswith(".hpp")]
    srcs = [o for o in outs if o.endswith(".cpp")]

    cc_library(
        name = name,
        srcs = srcs,
        hdrs = hdrs,
        strip_include_prefix = gen_root + "/include/generated_code",
        alwayslink = True,
        target_compatible_with = _LINUX_ONLY,
        deps = deps or [],
        visibility = visibility,
    )
