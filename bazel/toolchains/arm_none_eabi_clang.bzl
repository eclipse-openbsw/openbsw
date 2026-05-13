# ARM Clang (LLVM Embedded Toolchain for Arm) cross-compilation toolchain
# for bare-metal Cortex-M4 targets.
#
# Uses LLVM-ET-Arm 19.1.1 installed at /opt/llvm-et-arm/
# (see docker/development/Dockerfile).

load("@rules_cc//cc:cc_toolchain_config_lib.bzl", "tool_path")

_CLANG_ARM_PATH = "/opt/llvm-et-arm"

def _arm_none_eabi_clang_impl(ctx):
    tool_paths = [
        tool_path(name = "gcc", path = _CLANG_ARM_PATH + "/bin/clang"),
        tool_path(name = "g++", path = _CLANG_ARM_PATH + "/bin/clang++"),
        tool_path(name = "ld", path = _CLANG_ARM_PATH + "/bin/ld.lld"),
        tool_path(name = "ar", path = _CLANG_ARM_PATH + "/bin/llvm-ar"),
        tool_path(name = "nm", path = _CLANG_ARM_PATH + "/bin/llvm-nm"),
        tool_path(name = "objcopy", path = _CLANG_ARM_PATH + "/bin/llvm-objcopy"),
        tool_path(name = "objdump", path = _CLANG_ARM_PATH + "/bin/llvm-objdump"),
        tool_path(name = "strip", path = _CLANG_ARM_PATH + "/bin/llvm-strip"),
        tool_path(name = "cpp", path = _CLANG_ARM_PATH + "/bin/clang-cpp"),
        tool_path(name = "gcov", path = "/bin/false"),
    ]

    return cc_common.create_cc_toolchain_config_info(
        ctx = ctx,
        toolchain_identifier = "arm-none-eabi-clang",
        host_system_name = "x86_64-linux-gnu",
        target_system_name = "arm-none-eabi",
        target_cpu = "armv7e-m",
        target_libc = "newlib",
        compiler = "clang",
        abi_version = "eabi",
        abi_libc_version = "newlib",
        tool_paths = tool_paths,
        cxx_builtin_include_directories = [
            _CLANG_ARM_PATH + "/lib/clang-runtimes/arm-none-eabi/armv7em_hard_fpv4_sp_d16/include",
            _CLANG_ARM_PATH + "/lib/clang/19/include",
        ],
    )

arm_none_eabi_clang_toolchain_config = rule(
    implementation = _arm_none_eabi_clang_impl,
    attrs = {},
    provides = [CcToolchainConfigInfo],
)
