import argparse
import sys
from buildoperations import BuildOpTpl
from buildoperations import run_builds

# MATRIX
commands = {
    "wf-tests-debug": BuildOpTpl(
        config_cmd="cmake --workflow --preset wf-tests-debug",
        platforms=["linux"],
        cxxstds=[0],
        build_dir="build/tests/Debug",
    ),
    "wf-tests-release": BuildOpTpl(
        config_cmd="cmake --workflow --preset wf-tests-release",
        platforms=["linux"],
        cxxstds=[0],
        build_dir="build/tests/Release",
    ),
    "wf-posix-freertos": BuildOpTpl(
        config_cmd="cmake --workflow --preset wf-posix-freertos",
        platforms=["linux"],
        cxxstds=[0],
        build_dir="build/posix-freertos",
    ),
    "wf-posix-threadx": BuildOpTpl(
        config_cmd="cmake --workflow --preset wf-posix-threadx",
        platforms=["linux"],
        cxxstds=[0],
        build_dir="build/posix-threadx",
    ),
    "wf-s32k148-freertos-gcc": BuildOpTpl(
        config_cmd="cmake --workflow --preset wf-s32k148-freertos-gcc",
        platforms=["arm"],
        cxxstds=[0],
        build_dir="build/s32k148-freertos-gcc",
    ),
    "wf-s32k148-threadx-gcc": BuildOpTpl(
        config_cmd="cmake --workflow --preset wf-s32k148-threadx-gcc",
        platforms=["arm"],
        cxxstds=[0],
        build_dir="build/s32k148-threadx-gcc",
    ),
    # "wf-s32k148-freertos-clang": BuildOpTpl(
    #     config_cmd="cmake --workflow --preset wf-s32k148-freertos-clang",
    #     platforms=["arm"],
    #     cxxstds=[0],
    #     build_dir="build/s32k148-freertos-clang",
    # ),
    # "wf-s32k148-threadx-clang": BuildOpTpl(
    #     config_cmd="cmake --workflow --preset wf-s32k148-threadx-clang",
    #     platforms=["arm"],
    #     cxxstds=[0],
    #     build_dir="build/s32k148-threadx-clang",
    # ),
    "tests-debug": BuildOpTpl(
        config_cmd="cmake --preset tests-debug",
        build_cmd="cmake --build --preset tests-debug",
        test_cmd="ctest --preset tests-debug",
        configs=["Debug"],
        platforms=["linux"],
        build_dir="build/tests/Debug",
    ),
    "tests-release": BuildOpTpl(
        config_cmd="cmake --preset tests-release",
        build_cmd="cmake --build --preset tests-release",
        test_cmd="ctest --preset tests-release",
        configs=["Release"],
        platforms=["linux"],
        build_dir="build/tests/Release",
    ),
    "posix-freertos": BuildOpTpl(
        config_cmd="cmake --preset posix-freertos",
        build_cmd="cmake --build --preset posix-freertos",
        configs=["Debug", "Release"],
        platforms=["linux"],
        build_dir="build/posix-freertos",
    ),
    "posix-threadx": BuildOpTpl(
        config_cmd="cmake --preset posix-threadx",
        build_cmd="cmake --build --preset posix-threadx",
        configs=["Debug", "Release"],
        platforms=["linux"],
        build_dir="build/posix-threadx",
    ),
    "posix-freertos-with-tracing": BuildOpTpl(
        config_cmd="cmake --preset posix-freertos -DBUILD_TRACING=Yes",
        build_cmd="cmake --build --preset posix-freertos",
        configs=["Debug", "Release"],
        platforms=["linux"],
        build_dir="build/posix-freertos",
    ),
    "s32k148-freertos-gcc": BuildOpTpl(
        config_cmd="cmake --preset s32k148-freertos-gcc",
        build_cmd="cmake --build --preset s32k148-freertos-gcc",
        configs=["Debug", "Release", "RelWithDebInfo"],
        platforms=["arm"],
        cxxids=["gcc"],
        build_dir="build/s32k148-freertos-gcc",
    ),
    "s32k148-threadx-gcc": BuildOpTpl(
        config_cmd="cmake --preset s32k148-threadx-gcc",
        build_cmd="cmake --build --preset s32k148-threadx-gcc",
        configs=["Debug", "Release", "RelWithDebInfo"],
        platforms=["arm"],
        cxxids=["gcc"],
        build_dir="build/s32k148-threadx-gcc",
    ),
    # "s32k148-freertos-clang": BuildOpTpl(
    #     config_cmd="cmake --preset s32k148-freertos-clang",
    #     build_cmd="cmake --build --preset s32k148-freertos-clang",
    #     platforms=["arm"],
    #     cxxids=["clang"],
    # ),
    # "s32k148-threadx-clang": BuildOpTpl(
    #     config_cmd="cmake --preset s32k148-threadx-clang",
    #     build_cmd="cmake --build --preset s32k148-threadx-clang",
    #     platforms=["arm"],
    #     cxxids=["clang"],
    # ),
}


def main(argv: list[str] | None = None) -> int:
    pargs = argparse.ArgumentParser(description="Build runner")

    pargs.add_argument("--preset", default="", help="preset name (optional)")

    pargs.add_argument(
        "--cxxid", default="", help="C++ compiler id (clang or gcc - optional)"
    )

    pargs.add_argument("--cxxstd", type=int, default=0, help="C++ standard (optional)")

    pargs.add_argument(
        "--config",
        default="",
        help='Build config ("Debug", "Release", "RelWithDebInfo" - optional)',
    )

    pargs.add_argument(
        "--platform", default="", help='Target platform ("arm" or "linux" - optional)'
    )

    args = pargs.parse_args(argv)

    run_builds(args, commands)
    return 0


if __name__ == "__main__":
    sys.exit(main())
