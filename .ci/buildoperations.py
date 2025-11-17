import argparse
import os
import shlex
import shutil
import subprocess
import sys

from dataclasses import dataclass, field
from pathlib import Path
from rich import box
from rich.console import Console
from rich.table import Table
from typing import Final
from itertools import product


def get_full_path(command):
    if (cmd := shutil.which(command)) is None:
        print(f"ERROR: Compiler command {command} not found!")
        sys.exit(1)
    return cmd


def get_environment(cxxid: str, platform: str):
    env = {}

    threads = os.cpu_count() - 1
    if threads is None:
        threads = 1

    env["CTEST_PARALLEL_LEVEL"] = str(threads)
    env["CMAKE_BUILD_PARALLEL_LEVEL"] = str(threads)

    if platform == "arm":
        if cxxid == "gcc":
            env["CC"] = get_full_path("/opt/arm-gnu-toolchain/bin/arm-none-eabi-gcc")
            env["CXX"] = get_full_path("/opt/arm-gnu-toolchain/bin/arm-none-eabi-g++")
            return env
        if cxxid == "clang":
            env["CC"] = get_full_path("/opt/llvm-et-arm/bin/clang")
            env["CXX"] = get_full_path("/opt/llvm-et-arm/bin/clang++")
            return env

    if platform == "linux":
        if cxxid == "gcc":
            env["CC"] = get_full_path("gcc")
            env["CXX"] = get_full_path("g++")
            return env
        if cxxid == "clang":
            env["CC"] = get_full_path("clang")
            env["CXX"] = get_full_path("clang++")
            return env

    raise RuntimeError("Wrong platform or compiler ID")


def show_environment(env: dict):
    table = Table(
        title="Environment",
        title_style="bold dim",
        show_header=False,
        box=box.DOUBLE,
        style="dim",
    )
    table.add_column(justify="right", style="bold dim")
    table.add_column(justify="left", style="italic green")
    for k, v in env.items():
        table.add_row(str(k), "" if v is None else str(v))
    Console().print(table)


class Allowed:
    CXXSTDS: Final[tuple[int]] = [14, 17, 20, 23]
    CONFIGS: Final[tuple[str]] = ["Debug", "Release", "RelWithDebInfo"]
    PLATFORMS: Final[tuple[str]] = ["linux", "arm"]
    CXXIDS: Final[tuple[str]] = ["gcc", "clang"]


@dataclass(slots=True, kw_only=True)
class BuildOpTpl:
    config_cmd: str
    build_cmd: str = ""
    test_cmd: str = ""
    configs: list[str] = field(default_factory=lambda: ["N/A"])
    platforms: list[str] = field(default_factory=list)
    env: dict[str, str] = field(default_factory=list)
    cxxstds: list[int] = field(default_factory=lambda: Allowed.CXXSTDS)
    cxxids: list[str] = field(default_factory=lambda: ["gcc", "clang"])
    build_dir: str

    def __post_init__(self):
        for config in self.configs:
            if config not in Allowed.CONFIGS and config != "N/A":
                raise ValueError(f"Config must be one of {sorted(Allowed.CONFIGS)}")

        for cxxstd in self.cxxstds:
            if cxxstd not in Allowed.CXXSTDS and cxxstd != 0:
                raise ValueError(
                    f"C++ standard must be one of {sorted(Allowed.CXXSTDS)}"
                )

        for cxxid in self.cxxids:
            if cxxid not in Allowed.CXXIDS:
                raise ValueError(f"Compiler ID must be one of {sorted(Allowed.CXXIDS)}")

        for platform in self.platforms:
            if platform not in Allowed.PLATFORMS:
                raise ValueError(f"Platform must be one of {sorted(Allowed.PLATFORMS)}")

    def get_full_config_command(self, cxxstd: int):
        config_cmd_str = self.config_cmd
        if len(self.cxxstds) != 0:
            if cxxstd != 0:
                if cxxstd not in self.cxxstds and cxxstd != 0:
                    raise ValueError(
                        f"C++ standard must be one of {sorted(self.cxxstds)} (0 for N/A)"
                    )

                config_cmd_str += f" -DCMAKE_CXX_STANDARD={cxxstd}"

        return config_cmd_str

    def get_full_build_command(self, config: str):
        build_cmd_str = self.build_cmd
        if len(self.configs) != 0:
            if config != "N/A":
                if config not in self.configs:
                    raise ValueError(f"Config must be one of {sorted(self.configs)}")

                build_cmd_str += f" --config {str(config)}"

        build_cmd_str += " --verbose"

        return build_cmd_str

    def get_full_test_command(self):
        test_cmd_str = self.test_cmd
        return test_cmd_str

    def run_config_command(self, cxxid: str, cxxstd: int, platform: str):
        if cxxid not in self.cxxids:
            raise ValueError(f"Compiler ID must be one of {sorted(self.cxxids)}")

        if len(self.platforms) != 0:
            if platform not in self.platforms:
                raise ValueError(f"Platform must be one of {sorted(self.platforms)}")

        config_cmd = shlex.split(self.get_full_config_command(cxxstd))

        env = dict(os.environ)
        tmp_env = get_environment(cxxid, platform)
        if self.env is not None:
            tmp_env |= self.env

        env |= tmp_env

        subprocess.run(config_cmd, check=True, env=env)

    def run_build_command(self, cxxid: str, platform: str, config: str):
        build_cmd = shlex.split(self.get_full_build_command(config))

        env = dict(os.environ)
        tmp_env = get_environment(cxxid, platform)
        if self.env is not None:
            tmp_env |= self.env

        env |= tmp_env

        subprocess.run(build_cmd, check=True, env=env)

    def run_test_command(self, cxxid: str, platform: str):
        test_cmd = shlex.split(self.test_cmd)

        env = dict(os.environ)
        tmp_env = get_environment(cxxid, platform)
        if self.env is not None:
            tmp_env |= self.env

        env |= tmp_env

        subprocess.run(test_cmd, check=True, env=env)

    def run(self, cxxid: str, cxxstd: str, platform: str, config: str):
        build_dir = Path(self.build_dir)
        if build_dir.exists():
            shutil.rmtree(build_dir)

        cmds = []
        if self.config_cmd != "":
            cmds.append(self.get_full_config_command(cxxstd))

        if self.build_cmd != "":
            cmds.append(self.get_full_build_command(config))

        if self.test_cmd != "":
            cmds.append(self.get_full_test_command())

        commands_to_table("Commands", cmds)
        env = get_environment(cxxid, platform)
        show_environment(env)

        if self.config_cmd != "":
            self.run_config_command(cxxid, cxxstd, platform)

        if self.build_cmd != "":
            self.run_build_command(cxxid, platform, config)

        if self.test_cmd != "":
            self.run_test_command(cxxid, platform)


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

    Console().print(table)


def run_builds(args: argparse.Namespace, commands: dict[str, BuildOpTpl]):
    for i, (name, buildop) in enumerate(commands.items(), start=1):
        if args.preset != "":
            if name != args.preset:
                continue

        cxxids = buildop.cxxids
        cxxstds = [0]
        if len(buildop.cxxstds) != 0:
            cxxstds = buildop.cxxstds

        platforms = buildop.platforms

        configs = ["N/A"]
        if len(buildop.configs) != 0:
            configs = buildop.configs

        for cxxid, cxxstd, platform, config in product(
            cxxids, cxxstds, platforms, configs
        ):
            if args.cxxid != "":
                if args.cxxid != cxxid:
                    continue

            if args.cxxstd != 0:
                if args.cxxstd != cxxstd:
                    continue

            if args.platform != "":
                if args.platform != platform:
                    continue

            if args.config != "":
                if args.config != config:
                    continue

            print(f'Running build operation "{name}"')
            buildop.run(cxxid, cxxstd, platform, config)

    print("All combinations have been processed.")
