load("@bazel_tools//tools/build_defs/cc:action_names.bzl", "ACTION_NAMES")
load(
    "@bazel_tools//tools/cpp:cc_toolchain_config_lib.bzl",
    "feature",
    "flag_group",
    "flag_set",
    "tool_path",
)

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

def _impl(ctx):
    tool_paths = [
        tool_path(
            name = "gcc",
            path = "/usr/bin/arm-none-eabi-gcc",
        ),
        tool_path(
            name = "ld",
            path = "/usr/bin/arm-none-eabi-ld",
        ),
        tool_path(
            name = "ar",
            path = "/usr/bin/arm-none-eabi-ar",
        ),
        tool_path(
            name = "cpp",
            path = "/usr/bin/arm-none-eabi-cpp",
        ),
        tool_path(
            name = "gcov",
            path = "/usr/bin/arm-none-eabi-gcov",
        ),
        tool_path(
            name = "nm",
            path = "/usr/bin/arm-none-eabi-nm",
        ),
        tool_path(
            name = "objdump",
            path = "/usr/bin/arm-none-eabi-objdump",
        ),
        tool_path(
            name = "strip",
            path = "/usr/bin/arm-none-eabi-strip",
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
                            ],
                        ),
                    ],
                ),
            ],
        ),
        feature(
            name = "c_flags",
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
            name = "cxx_flags",
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
                            ],
                        ),
                    ],
                ),
            ],
        ),

        # TODO: Verify if assumption is correct: bare-metal targets do not use PIC or dynamic linking
        feature(name = "supports_pic", enabled = False),
        feature(name = "supports_dynamic_linker", enabled = False),
    ]

    return cc_common.create_cc_toolchain_config_info(
        ctx = ctx,
        features = features,
        cxx_builtin_include_directories = [
            "/usr/lib/gcc/arm-none-eabi",
            "/usr/include/newlib",
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
