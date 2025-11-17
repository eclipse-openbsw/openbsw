import subprocess
import os
import shutil
from pathlib import Path
import sys

def clean_build():
    build_dir = Path("doc/api/doxygenOut")
    if build_dir.exists():
        shutil.rmtree(build_dir)
    file_path_doc_coverage = Path("doc/api/doc-coverage.info")
    if os.path.exists(file_path_doc_coverage):
        os.remove(file_path_doc_coverage)
    file_path_warning = Path("doc/api/DoxygenWarningLog.txt")
    if os.path.exists(file_path_warning):
        os.remove(file_path_warning)

def run_doxygen():
    os.chdir("./doc/api")
    subprocess.run(["doxygen", "Doxyfile"], check=True)

def print_doxygen_warning_log():
    with open("DoxygenWarningLog.txt", "r") as file:
        print(file.read())

def run_coverxygen():
    subprocess.run(["python3", "-m", "coverxygen", "--format", "summary", "--xml-dir", "doxygenOut/xml/", "--src-dir", "../..", "--output", "doc-coverage.info"], check=True)

def print_doc_coverage():
    with open("doc-coverage.info", "r") as file:
        print(file.read())

if __name__ == "__main__":
    try:
        clean_build()
        run_doxygen()
        print_doxygen_warning_log()
        run_coverxygen()
        print_doc_coverage()
    except subprocess.CalledProcessError as e:
        print(f"Command failed with exit code {e.returncode}")
        sys.exit(e.returncode)
