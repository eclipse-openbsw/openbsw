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
            "-DCMAKE_C_COMPILER_LAUNCHER=sccache",
            "-DCMAKE_CXX_COMPILER_LAUNCHER=sccache",
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
            "lcov", "--capture",
            "--directory", f"code_coverage/{name}",
            "--no-external",
            "--base-directory", ".",
            "--output-file", unfiltered
        ], check=True)

        subprocess.run([
            "lcov", "--remove",
            unfiltered,
            *exclude_patterns,
            "--output-file", filtered
        ], check=True)

        tracefiles.append(filtered)

    merge_args = ["lcov"]
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
    result = subprocess.run(
        ["lcov", "--summary", f"{build_dir_name}/coverage.info"],
        capture_output=True,
        text=True,
        check=True,
    )

    summary = result.stdout

    line_percentage = re.search(r"lines\.*:\s+(\d+\.\d+)%", summary)
    func_percentage = re.search(r"functions\.*:\s+(\d+\.\d+)%", summary)

    if line_percentage:
        line_value = line_percentage.group(1)
        print(f"Line Coverage: {line_value}%")
        subprocess.run([
            "wget",
            f"https://img.shields.io/badge/lines-{line_value}%25-brightgreen.svg",
            "-O", f"{build_dir_name}/line_coverage_badge.svg"
        ], check=True)

    if func_percentage:
        func_value = func_percentage.group(1)
        print(f"Function Coverage: {func_value}%")
        subprocess.run([
            "wget",
            f"https://img.shields.io/badge/functions-{func_value}%25-brightgreen.svg",
            "-O", f"{build_dir_name}/function_coverage_badge.svg"
        ], check=True)

if __name__ == "__main__":
    try:
        env = dict(os.environ)
        threads = os.cpu_count() - 1 or 1
        env["CTEST_PARALLEL_LEVEL"] = str(threads)
        env["CMAKE_BUILD_PARALLEL_LEVEL"] = str(threads)

        env["CC"] = get_full_path("gcc-11")
        env["CXX"] = get_full_path("g++-11")

        build(env)
        generate_combined_coverage()
        generate_badges()
    except subprocess.CalledProcessError as e:
        print(f"Command failed with exit code {e.returncode}")
        sys.exit(e.returncode)