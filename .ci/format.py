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
