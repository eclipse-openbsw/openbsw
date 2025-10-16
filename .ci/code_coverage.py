from pathlib import Path
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
    env["CC"] = get_full_path("gcc-11")
    env["CXX"] = get_full_path("g++-11")

    subprocess.run(
        [
            "cmake",
            "--preset",
            "tests-posix-debug",
            "-B",
            f"{build_dir_name}",
            "-DCMAKE_C_COMPILER_LAUNCHER=sccache",
            "-DCMAKE_CXX_COMPILER_LAUNCHER=sccache",
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
            "--capture",
            "--directory",
            f"{build_dir_name}",
            "--no-external",
            "--base-directory",
            ".",
            "--output-file",
            f"{build_dir_name}/coverage_unfiltered.info",
        ],
        check=True,
    )

    # Remove unwanted paths from coverage

    subprocess.run(
        [
            "lcov",
            "--remove",
            f"{build_dir_name}/coverage_unfiltered.info",
            "*/libs/3rdparty/googletest/*",
            "*/mock/*",
            "*/gmock/*",
            "*/gtest/*",
            "*/test/*",
            "--output-file",
            f"{build_dir_name}/coverage.info",
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
        ],
        check=True,
    )


def generate_badges():
    result = subprocess.run(
        [
            "lcov",
            "--summary",
            f"{build_dir_name}/coverage.info",
        ],
        capture_output=True,
        text=True,
        check=True,
    )
    summary = result.stdout

    line_percentage = re.search(r"lines\.*:\s+(\d+\.\d+)%", summary)
    function_percentage = re.search(r"functions\.*:\s+(\d+\.\d+)%", summary)

    if line_percentage:
        line_value = line_percentage.group(1)
        print(f"Line Percentage: {line_value}%")
        subprocess.run(
            [
                "wget",
                f"https://img.shields.io/badge/coverage-{line_value}%25-brightgreen.svg",
                "-O",
                "line_coverage_badge.svg",
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
                "function_coverage_badge.svg",
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
