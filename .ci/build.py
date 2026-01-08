import argparse
import sys
from buildoperations import BuildOpTpl
from buildoperations import run_builds

# MATRIX
commands = {
    "tests-posix-debug": BuildOpTpl(
        config_cmd="cmake --preset tests-posix-debug",
        build_cmd="cmake --build --preset tests-posix-debug",
        test_cmd="ctest --preset tests-posix-debug --output-on-failure",
        configs=["Debug"],
        platforms=["linux"],
        build_dir="build/tests/posix/Debug",
    ),
    "tests-posix-release": BuildOpTpl(
        config_cmd="cmake --preset tests-posix-release",
        build_cmd="cmake --build --preset tests-posix-release",
        test_cmd="ctest --preset tests-posix-release --output-on-failure",
        configs=["Release"],
        platforms=["linux"],
        build_dir="build/tests/posix/Release",
    ),
    "tests-s32k1xx-debug": BuildOpTpl(
        config_cmd="cmake --preset tests-s32k1xx-debug",
        build_cmd="cmake --build --preset tests-s32k1xx-debug",
        test_cmd="ctest --preset tests-s32k1xx-debug --output-on-failure",
        configs=["Debug"],
        platforms=["linux"],
        build_dir="build/tests/s32k1xx/Debug",
    ),
    "tests-s32k1xx-release": BuildOpTpl(
        config_cmd="cmake --preset tests-s32k1xx-release",
        build_cmd="cmake --build --preset tests-s32k1xx-release",
        test_cmd="ctest --preset tests-s32k1xx-release --output-on-failure",
        configs=["Release"],
        platforms=["linux"],
        build_dir="build/tests/s32k1xx/Release",
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
    "s32k148-freertos-clang": BuildOpTpl(
        config_cmd="cmake --preset s32k148-freertos-clang",
        build_cmd="cmake --build --preset s32k148-freertos-clang",
        configs=["Debug", "Release", "RelWithDebInfo"],
        platforms=["arm"],
        cxxids=["clang"],
        build_dir="build/s32k148-freertos-clang",
    ),
    "s32k148-threadx-clang": BuildOpTpl(
        config_cmd="cmake --preset s32k148-threadx-clang",
        build_cmd="cmake --build --preset s32k148-threadx-clang",
        configs=["Debug", "Release", "RelWithDebInfo"],
        platforms=["arm"],
        cxxids=["clang"],
        build_dir="build/s32k148-threadx-clang",
    )
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
