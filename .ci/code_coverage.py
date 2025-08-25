import subprocess
from pathlib import Path
import shutil
import sys
import re

def configure_cmake(build_type):

    unit_tests_build_dir = Path("cmake-build-unit-tests")
    if unit_tests_build_dir.exists():
      shutil.rmtree(unit_tests_build_dir)

    subprocess.run([
        "cmake", "-B", "cmake-build-unit-tests", "-S", "executables/unitTest",
        "-DBUILD_UNIT_TESTS=ON",
        f"-DCMAKE_BUILD_TYPE={build_type}",
        "-DCMAKE_C_COMPILER_LAUNCHER=sccache",
        "-DCMAKE_CXX_COMPILER_LAUNCHER=sccache"
    ], check=True)

def build_with_cmake():
    subprocess.run(["cmake", "--build", "cmake-build-unit-tests", "-j4"], check=True)

def run_tests():
    subprocess.run(["ctest", "--test-dir", "cmake-build-unit-tests", "-j4", "--output-on-failure"], check=True)

def generate_coverage():
    # Capture coverage data
    subprocess.run([
        "lcov", "--no-external", "--capture", "--directory", ".",
        "--output-file", "cmake-build-unit-tests/coverage_unfiltered.info"
    ], check=True)

    # Remove unwanted paths from coverage
    subprocess.run([
        "lcov", "--remove", "cmake-build-unit-tests/coverage_unfiltered.info",
        "*libs/3rdparty/googletest/*", "*/mock/*", "*/gmock/*", "*/gtest/*", "*/test/*",
        "--output-file", "cmake-build-unit-tests/coverage.info"
    ], check=True)

    # Generate HTML report
    subprocess.run([
        "genhtml", "cmake-build-unit-tests/coverage.info",
        "--output-directory", "cmake-build-unit-tests/coverage"
    ], check=True)

def generate_badges():
    result = subprocess.run(
        ["lcov", "--summary", "cmake-build-unit-tests/coverage.info"],
        capture_output=True, text=True, check=True
    )
    summary = result.stdout

    line_percentage = re.search(r"lines\.*:\s+(\d+\.\d+)%", summary)
    function_percentage = re.search(r"functions\.*:\s+(\d+\.\d+)%", summary)

    if line_percentage:
        line_value = line_percentage.group(1)
        print(f"Line Percentage: {line_value}%")
        subprocess.run([
            "wget", f"https://img.shields.io/badge/coverage-{line_value}%25-brightgreen.svg",
            "-O", "line_coverage_badge.svg"
        ], check=True)

    if function_percentage:
        function_value = function_percentage.group(1)
        print(f"Function Percentage: {function_value}%")
        subprocess.run([
            "wget", f"https://img.shields.io/badge/coverage-{function_value}%25-brightgreen.svg",
            "-O", "function_coverage_badge.svg"
        ], check=True)

if __name__ == "__main__":
    try:
        configure_cmake("Debug")
        build_with_cmake()
        run_tests()
        generate_coverage()
        generate_badges()
    except subprocess.CalledProcessError as e:
        print(f"Command failed with exit code {e.returncode}")
        sys.exit(e.returncode)