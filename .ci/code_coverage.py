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


def build():
    build_dir = Path(build_dir_name)
    build_dir.parents
    if build_dir.exists():
        shutil.rmtree(build_dir)

    env = dict(os.environ)

    threads = os.cpu_count() - 1
    if threads is None:
        threads = 1

    env["CTEST_PARALLEL_LEVEL"] = str(threads)
    env["CMAKE_BUILD_PARALLEL_LEVEL"] = str(threads)
    env["CC"] = get_full_path(f"gcc-{GCC_VERSION}")
    env["CXX"] = get_full_path(f"g++-{GCC_VERSION}")

    subprocess.run(
        [
            "cmake",
            "--preset",
            "tests-posix-debug",
            "-B",
            f"{build_dir_name}",
        ],
        check=True,
        env=env,
    )

    subprocess.run(
        ["cmake", "--build", f"{build_dir_name}", "--config", "Debug", "--verbose"],
        check=True,
        env=env,
    )

    subprocess.run(
        ["ctest", "--test-dir", f"{build_dir_name}", "--output-on-failure"],
        check=True,
        env=env,
    )


def generate_coverage():
    # Capture coverage data
    subprocess.run(
        [
            "lcov",
            "--gcov-tool", f"gcov-{GCC_VERSION}",
            "--capture",
            "--directory",
            f"{build_dir_name}",
            "--no-external",
            "--base-directory",
            ".",
            "--output-file",
            f"{build_dir_name}/coverage_unfiltered.info",
            "--ignore-errors", "mismatch",
        ],
        check=True,
    )

    # Remove unwanted paths from coverage

    subprocess.run(
        [
            "lcov",
            "--gcov-tool", f"gcov-{GCC_VERSION}",
            "--remove",
            f"{build_dir_name}/coverage_unfiltered.info",
            "*/3rdparty/*",
            "*/mock/*",
            "*/test/*",
            "--output-file",
            f"{build_dir_name}/coverage.info",
            "--ignore-errors", "mismatch",
        ],
        check=True,
    )

    # Generate HTML report
    subprocess.run(
        [
            "genhtml",
            f"{build_dir_name}/coverage.info",
            "--output-directory",
            f"{build_dir_name}/coverage",
            "--prefix",
            "/home/jenkins/",
        ],
        check=True,
    )


def generate_badges():
    # FIXME: It's questionable whether we want to have a dependency to an
    # external service for generating these badges. This introduces a possible
    # cause of instabilities in case the external service becomes unavailable,
    # as already happened in the CI.

    result = subprocess.run(
        [
            "lcov",
            "--gcov-tool", f"gcov-{GCC_VERSION}",
            "--summary",
            f"{build_dir_name}/coverage.info",
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
        build()
        generate_coverage()
        generate_badges()
    except subprocess.CalledProcessError as e:
        print(f"Command failed with exit code {e.returncode}")
        sys.exit(e.returncode)
