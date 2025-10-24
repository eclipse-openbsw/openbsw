import shutil
import subprocess
import sys
import json
import re
from pathlib import Path


def clean_compile_commands(build_dir: Path, tmp_dir: Path):
    compile_commands = build_dir / "compile_commands.json"

    with open(compile_commands, "r") as f:
        data = json.load(f)
        cleaned_data = []
        FLAGS_TO_REMOVE = [r"-Wno-error=maybe-uninitialized", r"-Wno-error=stringop-overflow"]
        removal_pattern = re.compile(f'(?:{"|".join(map(re.escape, FLAGS_TO_REMOVE))})\\s*')

        for entry in data:
            command = entry["command"]
            # Use sub() to replace the flag (and any trailing whitespace) with an empty string
            cleaned_command = removal_pattern.sub("", command)
            entry["command"] = cleaned_command
            cleaned_data.append(entry)

        output_file = tmp_dir / "compile_commands.json"
        with open(output_file, "w") as cf:
            json.dump(cleaned_data, cf, indent=2)


def check_clang_tidy(build_dir: str):
    git_diff = subprocess.run(
        ["git", "diff", "-U0", "upstream/main..HEAD"],
        capture_output=True,
        text=True,
        check=True,
    )
    subprocess.run(
        ["clang-tidy-diff", "-p1", "-path", build_dir],
        input=git_diff.stdout,
        text=True,
        check=True,
        stdout=sys.stdout,
        stderr=sys.stderr,
    )


if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Error: Missing build path argument.", file=sys.stderr)
        print("Usage: python3 ci_clang_tidy.py <build_directory>", file=sys.stderr)
        sys.exit(1)
    try:
        BUILD_DIR = Path(sys.argv[1])
        TMP_BUILD_DIR = BUILD_DIR / "tmp"
        TMP_BUILD_DIR.mkdir(exist_ok=True)
        clean_compile_commands(BUILD_DIR, TMP_BUILD_DIR)
        check_clang_tidy(TMP_BUILD_DIR.as_posix())
        shutil.rmtree(TMP_BUILD_DIR)
    except subprocess.CalledProcessError as e:
        print(f"Command failed with exit code {e.returncode}")
        sys.exit(e.returncode)
