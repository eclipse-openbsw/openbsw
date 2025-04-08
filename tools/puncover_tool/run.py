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
