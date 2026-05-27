# *******************************************************************************
# Copyright (c) 2024 Accenture
#
# This program and the accompanying materials are made available under the
# terms of the Apache License Version 2.0 which is available at
# https://www.apache.org/licenses/LICENSE-2.0
#
# SPDX-License-Identifier: Apache-2.0
# *******************************************************************************

import subprocess
import sys

def check_code_format():
    subprocess.run(["treefmt", "--no-cache"], check=True)
    subprocess.run(["git", "diff", "--exit-code"], check=True)

if __name__ == "__main__":
    try:
       check_code_format()
    except subprocess.CalledProcessError as e:
        print(f"Command failed with exit code {e.returncode}")
        sys.exit(e.returncode)
