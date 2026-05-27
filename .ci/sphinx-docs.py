# *******************************************************************************
# Copyright (c) 2024 Accenture
#
# This program and the accompanying materials are made available under the
# terms of the Apache License Version 2.0 which is available at
# https://www.apache.org/licenses/LICENSE-2.0
#
# SPDX-License-Identifier: Apache-2.0
# *******************************************************************************

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