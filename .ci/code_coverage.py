from pathlib import Path
import os
import re
import shutil
import subprocess
import sys

# FIXME: Hard-coding the compiler version in this python script feels wrong. It
# should be changed in the future to provide the compiler from the outside.
GCC_VERSION = 11

def get_full_path(command):
    if (cmd := shutil.which(command)) is None:
        print(f"ERROR: Compiler command {command} not found!")
        sys.exit(1)
    return cmd

build_dir_name = "code_coverage"

targets = [
    ("s32k1xx", "tests-s32k1xx-debug"),
    ("posix", "tests-posix-debug"),
]

def build(env):
    for name, preset in targets:
        build_dir = Path(build_dir_name) / name

        if build_dir.exists():
            shutil.rmtree(build_dir)

        subprocess.run([
            "cmake", "--preset", preset,
            "-B", str(build_dir),
        ], check=True, env=env)

        subprocess.run([
            "cmake", "--build", str(build_dir), "--config", "Debug"
        ], check=True, env=env)

        subprocess.run([
            "ctest", "--test-dir", str(build_dir), "--output-on-failure"
        ], check=True, env=env)

def generate_combined_coverage():
    tracefiles = []

    exclude_patterns = [
        "*/mock/*",
        "*/gmock/*",
        "*/gtest/*",
        "*/test/*",
        "*/3rdparty/*",
    ]

    for name, _ in targets:
        unfiltered = f"code_coverage/coverage_{name}_unfiltered.info"
        filtered = f"code_coverage/coverage_{name}.info"

        subprocess.run([
            "lcov",
            "--gcov-tool", f"gcov-{GCC_VERSION}",
            "--capture",
            "--directory", f"code_coverage/{name}",
            "--no-external",
            "--base-directory", ".",
            "--output-file", unfiltered,
            "--ignore-errors", "mismatch",
            "--rc", "geninfo_unexecuted_blocks=1",
        ], check=True)

        subprocess.run([
            "lcov", "--remove",
            unfiltered,
            *exclude_patterns,
            "--output-file", filtered,
            "--ignore-errors", "mismatch",
        ], check=True)

        tracefiles.append(filtered)

    merge_args = ["lcov", "--ignore-errors", "mismatch"]
    for tf in tracefiles:
        merge_args += ["--add-tracefile", tf]
    merge_args += ["--output-file", f"{build_dir_name}/coverage.info"]

    subprocess.run(merge_args, check=True)

    repo_root = Path(__file__).resolve().parents[1]
    subprocess.run([
        "genhtml", f"{build_dir_name}/coverage.info",
        "--prefix", str(repo_root),
        "--output-directory", f"{build_dir_name}/coverage"
    ], check=True)

def generate_badges():
    # FIXME: It's questionable whether we want to have a dependency to an
    # external service for generating these badges. This introduces a possible
    # cause of instabilities in case the external service becomes unavailable,
    # as already happened in the CI.

    result = subprocess.run(
        [
            "lcov",
            "--gcov-tool", f"gcov-{GCC_VERSION}",
            "--summary", f"{build_dir_name}/coverage.info",
            "--ignore-errors", "mismatch",
        ],
        capture_output=True,
        text=True,
        check=True,
    )

    summary = result.stdout

    line_percentage = re.search(r"lines\.*:\s+(\d+\.\d+)%", summary)
    function_percentage = re.search(r"functions\.*:\s+(\d+\.\d+)%", summary)

    coverage_badge_path = Path(build_dir_name).joinpath("coverage_badges")
    coverage_badge_path.mkdir(parents=True, exist_ok=True)

    if line_percentage:
            line_value = line_percentage.group(1)
            print(f"Line Percentage: {line_value}%")
            subprocess.run(
                [
                    "wget",
                    f"https://img.shields.io/badge/coverage-{line_value}%25-brightgreen.svg",
                    "-O",
                    coverage_badge_path.joinpath("line_coverage_badge.svg").as_posix(),
                ],
                check=True,
            )

    if function_percentage:
            function_value = function_percentage.group(1)
            print(f"Function Percentage: {function_value}%")
            subprocess.run(
                [
                    "wget",
                    f"https://img.shields.io/badge/coverage-{function_value}%25-brightgreen.svg",
                    "-O",
                    coverage_badge_path.joinpath("function_coverage_badge.svg").as_posix(),
                ],
                check=True,
            )

if __name__ == "__main__":
    try:
        env = dict(os.environ)
        threads = os.cpu_count() - 1 or 1
        env["CTEST_PARALLEL_LEVEL"] = str(threads)
        env["CMAKE_BUILD_PARALLEL_LEVEL"] = str(threads)

        env["CC"] = get_full_path(f"gcc-{GCC_VERSION}")
        env["CXX"] = get_full_path(f"g++-{GCC_VERSION}")

        build(env)
        generate_combined_coverage()
        generate_badges()
    except subprocess.CalledProcessError as e:
        print(f"Command failed with exit code {e.returncode}")
        sys.exit(e.returncode)