import os
import subprocess
import shutil
from pathlib import Path
import sys

def build_sphinx_docs():
    build_dir = Path("doc/dev/build")
    if build_dir.exists():
        shutil.rmtree(build_dir)
    os.chdir("./doc/dev")
    subprocess.run(["make", "html", "OFFICIAL_BUILD=1"], check=True)

if __name__ == "__main__":
    try:
       build_sphinx_docs()
    except subprocess.CalledProcessError as e:
        print(f"Command failed with exit code {e.returncode}")
        sys.exit(e.returncode)