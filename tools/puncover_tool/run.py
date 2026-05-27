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
import os
import sys

here = os.path.dirname(os.path.abspath(__file__))
os.chdir(here)

# Installs dependencies
subprocess.run(["poetry", "install"], check=True)

# Adds puncover
subprocess.run(["poetry", "add", "puncover"], check=True)

# Run the script
subprocess.run(["poetry", "run", "puncover_tool"], check=True)

# Output can be viewed from output/index.html
