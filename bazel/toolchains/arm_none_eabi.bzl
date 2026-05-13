# ARM GCC cross-compilation toolchain for bare-metal Cortex-M4 targets.
#
# Uses arm-gnu-toolchain 14.3 installed at /opt/arm-gnu-toolchain/
# (see docker/development/Dockerfile).
#
# To use: bazel build --config=s32k148-freertos //executables/referenceApp/application:app

load("@rules_cc//cc:cc_toolchain_config_lib.bzl", "tool_path")

_GCC_ARM_PATH = "/opt/arm-gnu-toolchain"

def _arm_none_eabi_impl(ctx):
    tool_paths = [
        tool_path(name = "gcc", path = _GCC_ARM_PATH + "/bin/arm-none-eabi-gcc"),
        tool_path(name = "g++", path = _GCC_ARM_PATH + "/bin/arm-none-eabi-g++"),
        tool_path(name = "ld", path = _GCC_ARM_PATH + "/bin/arm-none-eabi-ld"),
        tool_path(name = "ar", path = _GCC_ARM_PATH + "/bin/arm-none-eabi-ar"),
        tool_path(name = "nm", path = _GCC_ARM_PATH + "/bin/arm-none-eabi-nm"),
        tool_path(name = "objcopy", path = _GCC_ARM_PATH + "/bin/arm-none-eabi-objcopy"),
        tool_path(name = "objdump", path = _GCC_ARM_PATH + "/bin/arm-none-eabi-objdump"),
        tool_path(name = "strip", path = _GCC_ARM_PATH + "/bin/arm-none-eabi-strip"),
        tool_path(name = "cpp", path = _GCC_ARM_PATH + "/bin/arm-none-eabi-cpp"),
        tool_path(name = "gcov", path = _GCC_ARM_PATH + "/bin/arm-none-eabi-gcov"),
    ]

    return cc_common.create_cc_toolchain_config_info(
        ctx = ctx,
        toolchain_identifier = "arm-none-eabi-gcc",
        host_system_name = "x86_64-linux-gnu",
        target_system_name = "arm-none-eabi",
        target_cpu = "armv7e-m",
        target_libc = "newlib",
        compiler = "gcc",
        abi_version = "eabi",
        abi_libc_version = "newlib",
        tool_paths = tool_paths,
        cxx_builtin_include_directories = [
            _GCC_ARM_PATH + "/arm-none-eabi/include",
            _GCC_ARM_PATH + "/arm-none-eabi/include/c++/14.3.1",
            _GCC_ARM_PATH + "/arm-none-eabi/include/c++/14.3.1/arm-none-eabi",
            _GCC_ARM_PATH + "/lib/gcc/arm-none-eabi/14.3.1/include",
            _GCC_ARM_PATH + "/lib/gcc/arm-none-eabi/14.3.1/include-fixed",
        ],
    )

arm_none_eabi_toolchain_config = rule(
    implementation = _arm_none_eabi_impl,
    attrs = {},
    provides = [CcToolchainConfigInfo],
)
