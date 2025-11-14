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
    env = dict(os.environ)

    threads = os.cpu_count() - 1
    if threads is None:
        threads = 1

    env["CTEST_PARALLEL_LEVEL"] = str(threads)
    env["CMAKE_BUILD_PARALLEL_LEVEL"] = str(threads)
    env["CC"] = get_full_path("gcc-11")
    env["CXX"] = get_full_path("g++-11")

    build_dir1 = Path(build_dir_name) / "s32k1xx"
    build_dir2 = Path(build_dir_name) / "posix"

    for d in [build_dir1, build_dir2]:
        if d.exists():
            shutil.rmtree(d)

    subprocess.run([
        "cmake", "--preset", "tests-s32k1xx-debug",
        "-B", str(build_dir1),
        "-DCMAKE_C_COMPILER_LAUNCHER=sccache",
        "-DCMAKE_CXX_COMPILER_LAUNCHER=sccache"
    ], check=True, env=env)

    subprocess.run(["cmake", "--build", str(build_dir1), "--config", "Debug"], check=True, env=env)

    subprocess.run(["ctest", "--test-dir", str(build_dir1), "--output-on-failure"], check=True, env=env)

    subprocess.run([
        "cmake", "--preset", "tests-posix-debug",
        "-B", str(build_dir2),
        "-DCMAKE_C_COMPILER_LAUNCHER=sccache",
        "-DCMAKE_CXX_COMPILER_LAUNCHER=sccache"
    ], check=True, env=env)

    subprocess.run(["cmake", "--build", str(build_dir2), "--config", "Debug"], check=True, env=env)

    subprocess.run(["ctest", "--test-dir", str(build_dir2), "--output-on-failure"], check=True, env=env)

def generate_combined_coverage():
    subprocess.run([
        "lcov", "--capture", "--directory", "code_coverage/s32k1xx",
        "--no-external", "--base-directory", ".", "--output-file",
        "code_coverage/coverage_s32k1xx_unfiltered.info"
    ], check=True)

    subprocess.run([
        "lcov", "--capture", "--directory", "code_coverage/posix",
        "--no-external", "--base-directory", ".", "--output-file",
        "code_coverage/coverage_posix_unfiltered.info"
    ], check=True)

    exclude_patterns = [
        "*/mock/*",
        "*/gmock/*",
        "*/gtest/*",
        "*/test/*",
        "*/3rdparty/*"
    ]

    subprocess.run([
        "lcov", "--remove", "code_coverage/coverage_s32k1xx_unfiltered.info", *exclude_patterns,
        "--output-file", "code_coverage/coverage_s32k1xx.info"
    ], check=True)

    subprocess.run([
        "lcov", "--remove", "code_coverage/coverage_posix_unfiltered.info", *exclude_patterns,
        "--output-file", "code_coverage/coverage_posix.info"
    ], check=True)

    subprocess.run([
        "lcov", "--add-tracefile", "code_coverage/coverage_s32k1xx.info",
        "--add-tracefile", "code_coverage/coverage_posix.info",
        "--output-file", f"{build_dir_name}/coverage.info"
    ], check=True)

    repo_root = Path(__file__).resolve().parents[1]
    subprocess.run([
        "genhtml", f"{build_dir_name}/coverage.info",
        "--prefix", str(repo_root),
        "--output-directory", f"{build_dir_name}/coverage"
    ], check=True)

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
        print(f"Line Coverage: {line_value}%")
        subprocess.run(
            [
                "wget",
                f"https://img.shields.io/badge/lines-{line_value}%25-brightgreen.svg",
                "-O",
                f"{build_dir_name}/line_coverage_badge.svg",
            ],
            check=True,
        )

    if function_percentage:
        function_value = function_percentage.group(1)
        print(f"Function Coverage: {function_value}%")
        subprocess.run(
            [
                "wget",
                f"https://img.shields.io/badge/functions-{function_value}%25-brightgreen.svg",
                "-O",
                f"{build_dir_name}/function_coverage_badge.svg",
            ],
            check=True,
        )

if __name__ == "__main__":
    try:
        build()
        generate_combined_coverage()
        generate_badges()
    except subprocess.CalledProcessError as e:
        print(f"Command failed with exit code {e.returncode}")
        sys.exit(e.returncode)