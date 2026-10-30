# *******************************************************************************
# Copyright (c) 2026 Accenture
#
# This program and the accompanying materials are made available under the
# terms of the Apache License Version 2.0 which is available at
# https://www.apache.org/licenses/LICENSE-2.0
#
# SPDX-License-Identifier: Apache-2.0
# *******************************************************************************

""" Bazel CC toolchain configuration for ARM Cortex-M4 bare-metal targets using arm-none-eabi-gcc."""

load("@bazel_tools//tools/build_defs/cc:action_names.bzl", "ACTION_NAMES")
load(
    "@bazel_tools//tools/cpp:cc_toolchain_config_lib.bzl",
    "feature",
    "feature_set",
    "flag_group",
    "flag_set",
    "tool_path",
)
load("@rules_cc//cc/common:cc_common.bzl", "cc_common")

_ALL_C_COMPILE_ACTIONS = [
    ACTION_NAMES.c_compile,
    ACTION_NAMES.preprocess_assemble,
    ACTION_NAMES.assemble,
]

_ALL_CXX_COMPILE_ACTIONS = [
    ACTION_NAMES.cpp_compile,
    ACTION_NAMES.cpp_header_parsing,
    ACTION_NAMES.cpp_module_compile,
    ACTION_NAMES.cpp_module_codegen,
]

_ALL_COMPILE_ACTIONS = _ALL_C_COMPILE_ACTIONS + _ALL_CXX_COMPILE_ACTIONS

_ALL_LINK_ACTIONS = [
    ACTION_NAMES.cpp_link_executable,
    ACTION_NAMES.cpp_link_dynamic_library,
    ACTION_NAMES.cpp_link_nodeps_dynamic_library,
]

_ALL_ASSEMBLE_ACTIONS = [
    ACTION_NAMES.assemble,
    ACTION_NAMES.preprocess_assemble,
]

_ARM_GNU_TOOLCHAIN_ROOT = "/opt/arm-gnu-toolchain"

def _impl(ctx):
    tool_paths = [
        tool_path(
            name = "gcc",
            path = _ARM_GNU_TOOLCHAIN_ROOT + "/bin/arm-none-eabi-gcc",
        ),
        tool_path(
            name = "ld",
            path = _ARM_GNU_TOOLCHAIN_ROOT + "/bin/arm-none-eabi-ld",
        ),
        tool_path(
            name = "ar",
            path = _ARM_GNU_TOOLCHAIN_ROOT + "/bin/arm-none-eabi-ar",
        ),
        tool_path(
            name = "cpp",
            path = _ARM_GNU_TOOLCHAIN_ROOT + "/bin/arm-none-eabi-cpp",
        ),
        tool_path(
            name = "gcov",
            path = _ARM_GNU_TOOLCHAIN_ROOT + "/bin/arm-none-eabi-gcov",
        ),
        tool_path(
            name = "nm",
            path = _ARM_GNU_TOOLCHAIN_ROOT + "/bin/arm-none-eabi-nm",
        ),
        tool_path(
            name = "objdump",
            path = _ARM_GNU_TOOLCHAIN_ROOT + "/bin/arm-none-eabi-objdump",
        ),
        tool_path(
            name = "strip",
            path = _ARM_GNU_TOOLCHAIN_ROOT + "/bin/arm-none-eabi-strip",
        ),
    ]

    features = [
        feature(
            name = "architecture_flags",
            enabled = True,
            flag_sets = [
                flag_set(
                    actions = _ALL_COMPILE_ACTIONS,
                    flag_groups = [
                        flag_group(
                            flags = [
                                "-mcpu=cortex-m4",
                                "-mfloat-abi=hard",
                                "-mfpu=fpv4-sp-d16",
                                "-fmessage-length=0",
                            ],
                        ),
                    ],
                ),
            ],
        ),
        feature(
            name = "common_compile_flags",
            enabled = True,
            flag_sets = [
                flag_set(
                    actions = _ALL_COMPILE_ACTIONS,
                    flag_groups = [
                        flag_group(
                            flags = [
                                "-Wno-psabi",
                                "-fdata-sections",
                                "-ffunction-sections",
                                "-fno-asynchronous-unwind-tables",
                                "-fno-builtin",
                                "-fno-common",
                                "-fshort-enums",
                                "-fstack-usage",
                                "-mno-unaligned-access",
                                "-mthumb",
                                # GCC canonicalises paths by default, resolving symlinks to
                                # their real location. Bazel header tracking relies on paths
                                # staying in their symlinked form.
                                "-fno-canonical-system-headers",
                            ],
                        ),
                    ],
                ),
            ],
        ),
        feature(
            name = "c_compile_flags",
            enabled = True,
            flag_sets = [
                flag_set(
                    actions = _ALL_C_COMPILE_ACTIONS,
                    flag_groups = [
                        flag_group(
                            flags = [
                                "-ffreestanding",
                                "-funsigned-bitfields",
                            ],
                        ),
                    ],
                ),
            ],
        ),
        feature(
            name = "cxx_compile_flags",
            enabled = True,
            flag_sets = [
                flag_set(
                    actions = _ALL_CXX_COMPILE_ACTIONS,
                    flag_groups = [
                        flag_group(
                            flags = [
                                "-std=gnu++17",
                                "-fno-exceptions",
                                "-fno-non-call-exceptions",
                                "-fno-rtti",
                                "-fno-threadsafe-statics",
                                "-fno-use-cxa-atexit",
                                "-femit-class-debug-always",
                                "-funsigned-bitfields",
                            ],
                        ),
                    ],
                ),
            ],
        ),
        feature(
            name = "assembler_flags",
            enabled = True,
            flag_sets = [
                flag_set(
                    actions = _ALL_ASSEMBLE_ACTIONS,
                    flag_groups = [
                        flag_group(
                            flags = [
                                "-g",
                            ],
                        ),
                    ],
                ),
            ],
        ),
        feature(
            name = "warning_flags",
            enabled = True,
            flag_sets = [
                flag_set(
                    # C compilation only. Assembly actions are excluded because
                    # CMake does not pass warning flags to the assembler.
                    actions = [ACTION_NAMES.c_compile],
                    flag_groups = [flag_group(flags = ["-Wall", "-Wextra"])],
                ),
                flag_set(
                    actions = _ALL_CXX_COMPILE_ACTIONS,
                    flag_groups = [flag_group(flags = [
                        "-Wall",
                        "-Wextra",
                        "-Wvla",
                        "-Wno-deprecated-volatile",
                    ])],
                ),
            ],
        ),
        feature(
            name = "treat_warnings_as_errors",
            enabled = True,
            requires = [feature_set(features = ["warning_flags"])],
            flag_sets = [
                flag_set(
                    # Applied to C and C++ compilation only, excluding assembly.
                    actions = [ACTION_NAMES.c_compile] + _ALL_CXX_COMPILE_ACTIONS,
                    flag_groups = [flag_group(flags = ["-Werror"])],
                ),
            ],
        ),
        # Mode stubs for Bazel's built-in compilation modes. Bazel activates
        # one of these based on --compilation_mode. Declaring them here lets
        # mode-specific compile flag features gate on them.
        feature(name = "fastbuild"),
        feature(name = "dbg"),
        feature(name = "opt"),
        feature(
            name = "fastbuild_compile_flags",
            enabled = True,
            requires = [feature_set(features = ["fastbuild"])],
            flag_sets = [
                flag_set(
                    # Matches the CMake ARM RelWithDebInfo build type, the default for ARM targets.
                    actions = [ACTION_NAMES.c_compile] + _ALL_CXX_COMPILE_ACTIONS,
                    flag_groups = [flag_group(flags = ["-g3", "-O2", "-DNDEBUG"])],
                ),
                flag_set(
                    # Assembly carries debug-info only in this mode, matching CMake.
                    actions = _ALL_ASSEMBLE_ACTIONS,
                    flag_groups = [flag_group(flags = ["-g3"])],
                ),
            ],
        ),
        feature(
            name = "dbg_compile_flags",
            enabled = True,
            requires = [feature_set(features = ["dbg"])],
            flag_sets = [
                flag_set(
                    # Matches the CMake Debug build type. Assembly has no extra flags
                    # in Debug mode, so only C and C++ compilation receives the flag.
                    actions = [ACTION_NAMES.c_compile] + _ALL_CXX_COMPILE_ACTIONS,
                    flag_groups = [flag_group(flags = ["-g3"])],
                ),
            ],
        ),
        feature(
            name = "opt_compile_flags",
            enabled = True,
            requires = [feature_set(features = ["opt"])],
            flag_sets = [
                flag_set(
                    # Matches the CMake Release build type. Assembly has no extra flags
                    # in Release mode, so only C and C++ compilation receives the flags.
                    actions = [ACTION_NAMES.c_compile] + _ALL_CXX_COMPILE_ACTIONS,
                    flag_groups = [flag_group(flags = ["-O2", "-DNDEBUG"])],
                ),
            ],
        ),
        feature(
            name = "realtime_os_define",
            enabled = True,
            flag_sets = [
                flag_set(
                    actions = _ALL_COMPILE_ACTIONS,
                    flag_groups = [
                        flag_group(
                            flags = ["-DREALTIME_OS=1"],
                        ),
                    ],
                ),
            ],
        ),
        feature(
            name = "default_linker_flags",
            enabled = True,
            flag_sets = [
                flag_set(
                    actions = _ALL_LINK_ACTIONS,
                    flag_groups = [
                        flag_group(
                            flags = [
                                "-mcpu=cortex-m4",
                                "-mfloat-abi=hard",
                                "-mfpu=fpv4-sp-d16",
                                "-static",
                                "-Wl,--gc-sections",
                                "-specs=nano.specs",
                                "-specs=nosys.specs",
                                # Newlib-nano requires explicit standard library flags. The g++ link
                                # driver adds them automatically in CMake but Bazel uses the raw linker.
                                "-lm",
                                "-lstdc++",
                            ],
                        ),
                    ],
                ),
            ],
        ),
        feature(name = "supports_pic", enabled = False),
        feature(name = "supports_dynamic_linker", enabled = False),
    ]

    return cc_common.create_cc_toolchain_config_info(
        ctx = ctx,
        features = features,
        cxx_builtin_include_directories = [
            _ARM_GNU_TOOLCHAIN_ROOT + "/lib/gcc/arm-none-eabi",
            _ARM_GNU_TOOLCHAIN_ROOT + "/arm-none-eabi/include",
        ],
        toolchain_identifier = "arm_none_eabi_gcc_toolchain",
        host_system_name = "local",
        target_system_name = "arm-none-eabi",
        target_cpu = "cortex-m4",
        target_libc = "newlib-nano",
        compiler = "gcc",
        abi_version = "eabi",
        abi_libc_version = "newlib",
        tool_paths = tool_paths,
    )

cc_toolchain_config = rule(
    implementation = _impl,
    attrs = {},
    provides = [CcToolchainConfigInfo],
)
