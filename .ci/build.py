import shutil
from pathlib import Path
import subprocess
import sys
import os

def get_full_command_path(command):
    if (cmd := shutil.which(command)) is None:
        print(f"ERROR: Compiler command {command} not found!")
        sys.exit(1)
    return cmd

def get_environment_variables(platform, compiler):
    env = dict(os.environ)
    if platform == "s32k148evb":
        if compiler == "gcc":
            env["CC"] = get_full_command_path("arm-none-eabi-gcc")
            env["CXX"] = get_full_command_path("arm-none-eabi-g++")
        elif compiler == "clang":
            env["CC"] = get_full_command_path("/usr/bin/llvm-arm/bin/clang")
            env["CXX"] = get_full_command_path("/usr/bin/llvm-arm/bin/clang++")
    
    elif platform == "posix":
        if compiler == "clang":
            env["CC"] = get_full_command_path("clang")
            env["CXX"] = get_full_command_path("clang++")
        elif compiler == "gcc":
            env["CC"] = get_full_command_path("gcc")
            env["CXX"] = get_full_command_path("g++")
    return env

def configure_and_build(platform, rtos, compiler, cpp_standard):
    build_dir = Path(f"cmake-build-{platform}-{rtos}-{compiler}")
    if build_dir.exists():
        shutil.rmtree(build_dir)

    cmake_command = [
        "cmake",
        "-B", build_dir,
        "-S", "executables/referenceApp",
        f"-DCMAKE_CXX_STANDARD={cpp_standard}",
        f"-DBUILD_TARGET_PLATFORM={platform}",
        f"-DBUILD_TARGET_RTOS={rtos}",
        f"-DCMAKE_TOOLCHAIN_FILE={'../../admin/cmake/ArmNoneEabi-' + compiler + '.cmake' if platform == 's32k148evb' else ''}"
    ]
    subprocess.run(cmake_command, check=True, env=get_environment_variables(platform, compiler))
    subprocess.run(["cmake", "--build", build_dir, "--target", "app.referenceApp", "-j"], check=True, env=get_environment_variables(platform, compiler))

def main():
    if len(sys.argv) != 5:
        print("ERROR: Usage: build.py <platform> <rtos> <compiler> <cpp_standard>")
        sys.exit(1)

    platform = sys.argv[1]
    rtos = sys.argv[2]
    compiler = sys.argv[3]
    cpp_standard = sys.argv[4]

    configure_and_build(platform, rtos, compiler, cpp_standard)

if __name__ == "__main__":
    main()
