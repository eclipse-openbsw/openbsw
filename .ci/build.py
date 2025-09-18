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
    "wf-posix": BuildOpTpl(
        config_cmd="cmake --workflow --preset wf-posix",
        platforms=["linux"],
        cxxstds=[0],
        build_dir="build/posix",
    ),
    "wf-s32k148-gcc": BuildOpTpl(
        config_cmd="cmake --workflow --preset wf-s32k148-gcc",
        platforms=["arm"],
        cxxstds=[0],
        build_dir="build/s32k148-gcc",
    ),
    # "wf-s32k148-clang": BuildOpTpl(
    #     config_cmd="cmake --workflow --preset wf-s32k148-clang",
    #     platforms=["arm"],
    #     cxxstds=[0],
    #     build_dir="build/s32k148-clang",
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
    "posix": BuildOpTpl(
        config_cmd="cmake --preset posix",
        build_cmd="cmake --build --preset posix",
        configs=["Debug", "Release"],
        platforms=["linux"],
        build_dir="build/posix",
    ),
    "posix-with-tracing": BuildOpTpl(
        config_cmd="cmake --preset posix -DBUILD_TRACING=Yes",
        build_cmd="cmake --build --preset posix",
        configs=["Debug", "Release"],
        platforms=["linux"],
        build_dir="build/posix",
    ),
    "s32k148-gcc": BuildOpTpl(
        config_cmd="cmake --preset s32k148-gcc",
        build_cmd="cmake --build --preset s32k148-gcc",
        configs=["Debug", "Release", "RelWithDebInfo"],
        platforms=["arm"],
        cxxids=["gcc"],
        build_dir="build/s32k148-gcc",
    ),
    # "s32k148-clang": BuildOpTpl(
    #     config_cmd="cmake --preset s32k148-clang",
    #     build_cmd="cmake --build --preset s32k148-clang",
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
