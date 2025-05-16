import subprocess
import os
import sys

def build_reference_app():
    subprocess.run(["cmake", "-B", "cmake-build-posix", "-S", "executables/referenceApp"], check=True)
    subprocess.run(["cmake", "--build", "cmake-build-posix", "--target", "app.referenceApp", "-j"], check=True)

def run_pytest():
    os.chdir("./test/pyTest")
    subprocess.run(['pytest', '--target=posix'], check=True)

if __name__ == "__main__":
    try:
        build_reference_app()
        run_pytest()
    except subprocess.CalledProcessError as e:
        print(f"Command failed with exit code {e.returncode}")
        sys.exit(e.returncode)
