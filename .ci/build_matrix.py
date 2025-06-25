from enum import Enum, auto
from pathlib import Path
from rich import box
from rich.console import Console
from rich.table import Table
import itertools
import os
import re
import shutil
import subprocess
import sys


def get_full_path(command):
    if (cmd := shutil.which(command)) is None:
        print(f"ERROR: Compiler command {command} not found!")
        sys.exit(1)
    return cmd


class CxxId(Enum):
    GNU = "GNU"
    Clang = "Clang"
    Both = "Undefined"

    def __str__(self) -> str:
        return self.value


class Platform(Enum):
    Embedded = "Embedded"
    x86 = "x86 / POSIX-compatible"

    def __str__(self) -> str:
        return self.value


class Config(Enum):
    NoConfig = auto()
    DebugRelease = auto()
    MultiConfig = auto()


# MATRIX
commands = [
    # workflow for unit tests (Debug)
    # cmake --workflow --preset ci-tests-dbg
    {
        "config": ["cmake", "--workflow", "--preset", "ci-tests-dbg"],
        "build": None,
        "test": None,
        "cxxid": CxxId.Both,
        "platform": Platform.x86,
        "build-config": Config.NoConfig,
    },
    # workflow for unit tests (Release)
    # cmake --workflow --preset ci-tests-rel
    {
        "config": ["cmake", "--workflow", "--preset", "ci-tests-rel"],
        "build": None,
        "test": None,
        "cxxid": CxxId.Both,
        "platform": Platform.x86,
        "build-config": Config.NoConfig,
    },
    # workflow for reference application for POSIX-compatibles
    # cmake --workflow --preset ci-posix
    {
        "config": ["cmake", "--workflow", "--preset", "ci-posix"],
        "build": None,
        "test": None,
        "cxxid": CxxId.Both,
        "platform": Platform.x86,
        "build-config": Config.NoConfig,
    },
    # workflow for s32k148 built with GNU compiler
    # cmake --workflow --preset ci-s32k148-gcc
    {
        "config": ["cmake", "--workflow", "--preset", "ci-s32k148-gcc"],
        "build": None,
        "test": None,
        "cxxid": CxxId.GNU,
        "platform": Platform.Embedded,
        "build-config": Config.NoConfig,
    },
    # workflow for s32k148 built with Clang
    # cmake --workflow --preset ci-s32k148-clang
    # {
    #     "config": ["cmake", "--workflow", "--preset", "ci-s32k148-clang"],
    #     "build": None,
    #     "test": None,
    #     "cxxid": CxxId.Clang,
    #     "platform": Platform.Embedded,
    #     "build-config": Config.NoConfig,
    # },
    # presets for unit tests (Debug)
    # CC=gcc/clang CXX=g++/clang++
    # cmake --preset tests-dbg
    # cmake --build --preset tests-dbg
    # ctest --preset tests-dbg
    {
        "config": ["cmake", "--preset", "tests-dbg"],
        "build": ["cmake", "--build", "--preset", "tests-dbg"],
        "test": ["ctest", "--preset", "tests-dbg"],
        "cxxid": CxxId.Both,
        "platform": Platform.x86,
        "build-config": Config.NoConfig,
    },
    # presets for reference application for POSIX-compatibles
    # CC=gcc/clang CXX=g++/clang++
    # cmake --preset tests-rel
    # cmake --build --preset tests-rel
    # ctest --preset tests-rel
    {
        "config": ["cmake", "--preset", "tests-rel"],
        "build": ["cmake", "--build", "--preset", "tests-rel"],
        "test": ["ctest", "--preset", "tests-rel"],
        "cxxid": CxxId.Both,
        "platform": Platform.x86,
        "build-config": Config.NoConfig,
    },
    # preset for reference application for POSIX-compatibles with
    # tracing disabled
    # CC=gcc/clang CXX=g++/clang++
    # cmake --preset posix
    # cmake --build --preset posix # --config Debug / --config Release
    {
        "config": ["cmake", "--preset", "posix"],
        "build": ["cmake", "--build", "--preset", "posix"],
        "test": None,
        "cxxid": CxxId.Both,
        "platform": Platform.x86,
        "build-config": Config.DebugRelease,
    },
    # preset for reference application for POSIX-compatibles with
    # tracing enabled
    # CC=gcc/clang CXX=g++/clang++
    # cmake --preset posix -DBUILD_TRACING=Yes
    # cmake --build --preset posix # --config Debug / --config Release
    {
        "config": ["cmake", "--preset", "posix", "-DBUILD_TRACING=Yes"],
        "build": ["cmake", "--build", "--preset", "posix"],
        "test": None,
        "cxxid": CxxId.Both,
        "platform": Platform.x86,
        "build-config": Config.DebugRelease,
    },
    # preset for reference application for s32k148 built with GNU compiler
    # cmake --preset s32k148-gcc
    # cmake --build --preset s32k148-gcc # --config Debug / --config Release / --config RelWithDebInfo
    {
        "config": ["cmake", "--preset", "s32k148-gcc"],
        "build": ["cmake", "--build", "--preset", "s32k148-gcc"],
        "test": None,
        "cxxid": CxxId.GNU,
        "platform": Platform.Embedded,
        "build-config": Config.MultiConfig,
    },
    # preset for reference application for s32k148 built with Clang
    # cmake --preset s32k148-clang
    # cmake --build --preset s32k148-clang # --config Debug / --config Release / --config RelWithDebInfo
    # {
    #     "config": ["cmake", "--preset", "s32k148-clang"],
    #     "build": ["cmake", "--build", "--preset", "s32k148-clang"],
    #     "test": None,
    #     "cxxid": CxxId.Clang,
    #     "platform": Platform.Embedded,
    #     "build-config": Config.MultiConfig,
    # },
]


def get_environment(cxxid: CxxId, platform: Platform):
    env = {}

    threads = os.cpu_count() - 1
    if threads is None:
        threads = 1

    env["CTEST_PARALLEL_LEVEL"] = str(threads)
    env["CMAKE_BUILD_PARALLEL_LEVEL"] = str(threads)

    if platform == Platform.Embedded:
        if cxxid == CxxId.GNU:
            env["CC"] = get_full_path("arm-none-eabi-gcc")
            env["CXX"] = get_full_path("arm-none-eabi-g++")
            return env
        if cxxid == CxxId.Clang:
            env["CC"] = get_full_path("/usr/bin/llvm-arm/bin/clang")
            env["CXX"] = get_full_path("/usr/bin/llvm-arm/bin/clang++")
            return env

    if platform == Platform.x86:
        if cxxid == CxxId.GNU:
            env["CC"] = get_full_path("gcc-11")
            env["CXX"] = get_full_path("g++-11")
            return env
        if cxxid == CxxId.Clang:
            env["CC"] = get_full_path("clang")
            env["CXX"] = get_full_path("clang++")
            return env

    raise RuntimeError("Wrong platform or compiler ID")


def commands_to_table(title: str, data: list) -> Table:
    table = Table(
        title=title,
        title_style="bold dim",
        show_header=False,
        box=box.DOUBLE,
        style="dim",
    )
    table.add_column(justify="left", style="italic green")
    for v in data:
        if v is not None:
            table.add_row(str(v))
    return table


def dict_to_table(title: str, data: dict) -> Table:
    table = Table(
        title=title,
        title_style="bold dim",
        show_header=False,
        box=box.DOUBLE,
        style="dim",
    )
    table.add_column(justify="right", style="bold dim")
    table.add_column(justify="left", style="italic green")
    for k, v in data.items():
        table.add_row(str(k), "" if v is None else str(v))
    return table


def all_builds():
    cxxstds = ["14", "17", "20", "23"]
    build_dir = Path("build")
    console = Console()

    for cmd in commands:
        cxxids = [""]
        if cmd["cxxid"] == CxxId.Both:
            cxxids = [CxxId.GNU, CxxId.Clang]
        else:
            cxxids = [cmd["cxxid"]]

        cfgs = [[""]]
        if cmd["build-config"] != Config.NoConfig:
            cfgs = [["--config", "Debug"], ["--config", "Release"]]
            if cmd["build-config"] == Config.MultiConfig:
                cfgs.append(["--config", "RelWithDebInfo"])

        cxxstdvars = [""]
        if cmd["build"]:
            cxxstdvars = [f"-DCMAKE_CXX_STANDARD={std}" for std in cxxstds]

        for cxxid, cxxstdvar, cfg in itertools.product(cxxids, cxxstdvars, cfgs):
            parameters = {
                "compiler ID": str(cxxid),
                "configuration": f"{cfg[1] if len(cfg) > 1 else 'N/A'}",
                "platform": f"{cmd['platform']}",
            }

            if cxxstdvar != "":
                parameters["C++ standard"] = list(
                    map(int, re.findall(r"\d+", f"{cxxstdvar}"))
                )[0]

            console.print(dict_to_table("Iteration", parameters))

            cli_commands = [" ".join(cmd["config"] + [f"{cxxstdvar}"])]
            if cmd["build"] is not None:
                build_cmd = cmd["build"].copy()

                cfg_str = " ".join(cfg)
                if cfg_str != "":
                    build_cmd.append(cfg_str)

                cli_commands = cli_commands + [" ".join(build_cmd)]

            if cmd["test"] is not None:
                cli_commands = cli_commands + [" ".join(cmd["test"])]

            console.print(commands_to_table("Commands", cli_commands))

            env_addition = get_environment(cxxid, cmd["platform"])
            console.print(dict_to_table("Environment", env_addition))
            env = dict(os.environ)
            env |= env_addition

            if build_dir.exists():
                shutil.rmtree(build_dir)

            if cmd["config"]:
                cfg_cmd = cmd["config"] + [f"{cxxstdvar}"]
                cfg_cmd = [x for x in cfg_cmd if x != ""]
                subprocess.run(cfg_cmd, check=True, env=env)

            if cmd["build"]:
                build_cmd = cmd["build"] + cfg
                build_cmd = [x for x in build_cmd if x != ""]
                subprocess.run(build_cmd, check=True, env=env)

            if cmd["test"]:
                test_cmd = [x for x in cmd["test"] if x != ""]
                subprocess.run(test_cmd, check=True, env=env)

    print("All combinations have been processed.")


def main():
    all_builds()


if __name__ == "__main__":
    sys.exit(main())
