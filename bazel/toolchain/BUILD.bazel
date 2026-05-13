load(":arm_none_eabi/gcc/arm_none_eabi_gcc_cc_toolchain_config.bzl", "cc_toolchain_config")

package(default_visibility = ["//visibility:public"])

filegroup(name = "empty")

cc_toolchain_config(name = "arm_none_eabi_gcc_toolchain_config")

cc_toolchain(
    name = "arm_none_eabi_gcc_toolchain",
    all_files = ":empty",
    compiler_files = ":empty",
    dwp_files = ":empty",
    linker_files = ":empty",
    objcopy_files = ":empty",
    strip_files = ":empty",
    supports_param_files = 0,
    toolchain_config = ":arm_none_eabi_gcc_toolchain_config",
)

toolchain(
    name = "cc_toolchain_arm_none_eabi_gcc",
    target_compatible_with = [
        "@platforms//os:none",
        "@platforms//cpu:armv7e-m",
    ],
    toolchain = ":arm_none_eabi_gcc_toolchain",
    toolchain_type = "@bazel_tools//tools/cpp:toolchain_type",
)
