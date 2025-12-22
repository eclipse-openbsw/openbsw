import subprocess
import os
import sys

def run_cmd(cmd, cwd=None):
    print(f"\n>>> Running: {cmd}", flush=True)
    subprocess.run(cmd, shell=True, check=True, cwd=cwd)

def main():
    target = "s32k148"
    toolchain = "gcc"
    cxxstd = "14"
    matrix = [
        {"app": "freertos", "preset": "s32k148-freertos-gcc"},
        {"app": "threadx",  "preset": "s32k148-threadx-gcc"},
    ]
    for item in matrix:
        preset = item["preset"]
        run_cmd(f'python3 .ci/build.py --preset "{preset}" --platform "arm" --cxxid "{toolchain}" --cxxstd "{cxxstd}"')

    os.chdir("test/pyTest")

    for item in matrix:
        app = item["app"]
        run_cmd(f'pytest -s --target={target} --no-restart --app={app}')

if __name__ == "__main__":
    try:
        main()
    except subprocess.CalledProcessError as e:
        print(f"\nERROR: command failed with exit code {e.returncode}")
        sys.exit(e.returncode)

