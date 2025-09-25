import subprocess
import os
import sys


def build_reference_app():
    threads = os.cpu_count() - 1
    if threads is None:
        threads = 1

    env = dict(os.environ)
    env["CMAKE_BUILD_PARALLEL_LEVEL"] = str(threads)

    subprocess.run(["cmake", "--preset", "posix"], check=True)
    subprocess.run(["cmake", "--build", "--preset", "posix"], check=True, env=env)


def run_pytest():
    os.chdir("./test/pyTest")
    subprocess.run(["pytest", "--target=posix"], check=True)


if __name__ == "__main__":
    try:
        build_reference_app()
        run_pytest()
    except subprocess.CalledProcessError as e:
        print(f"Command failed with exit code {e.returncode}")
        sys.exit(e.returncode)
