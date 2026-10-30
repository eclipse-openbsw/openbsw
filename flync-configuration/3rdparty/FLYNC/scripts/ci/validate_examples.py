"""
Example validation gate for the FLYNC SDK.

Runs ``flync validate`` over every example under ``examples/`` and exits non-zero if any of them failed. Warnings are reported but do not
fail the run.

Paths git ignores are skipped: developers keep local work under ``examples/`` that CI never sees, and validating it would make the gate
fail on one machine and pass on another.
"""

import subprocess
import sys
from pathlib import Path

EXAMPLES_DIR = Path(__file__).resolve().parents[2] / "examples"

CLI = Path(sys.executable).parent / "flync"

MAX_DEPTH = 10


def _kind(path):
    """Return what kind of example this directory holds, or None if it holds none."""

    if (path / "system_metadata.flync.yaml").is_file():
        return "workspace"
    if (path / "ecu_metadata.flync.yaml").is_file():
        return "ecu"
    return None


def _git_ignored(paths):
    """Return the paths git ignores. An empty set if git cannot answer, which errs toward validating too much."""

    try:
        done = subprocess.run(
            ["git", "check-ignore", "--stdin"],
            input="\n".join(str(p) for p in paths),
            capture_output=True,
            text=True,
            cwd=EXAMPLES_DIR.parent,
            check=False,
        )
    except (OSError, subprocess.SubprocessError):
        return set()
    # 0 means some matched, 1 means none did; anything else (128: not a repository) is not an answer.
    return {Path(line) for line in done.stdout.splitlines()} if done.returncode in (0, 1) else set()


def _discover(root, depth=0):
    """Yield ``(path, kind)`` for every example under *root*, descending into directories that hold none."""

    if depth > MAX_DEPTH:
        print(f"  (not validated: {root.relative_to(EXAMPLES_DIR)} and below, more than {MAX_DEPTH} levels deep)")
        return

    entries = sorted(p for p in root.iterdir() if p.is_dir() and not p.is_symlink() and not p.name.startswith("."))
    ignored = _git_ignored(entries)

    for entry in entries:
        if entry in ignored:
            print(f"  (skipping {entry.relative_to(EXAMPLES_DIR)}: git ignores it, so CI never sees it)")
            continue

        kind = _kind(entry)
        if kind:
            yield entry, kind
            continue

        found = list(_discover(entry, depth + 1))
        if found:
            yield from found
        else:
            print(f"  (not validated: {entry.relative_to(EXAMPLES_DIR)} is not a FLYNC configuration)")


def _validate(path, kind, name):
    """Run ``flync validate`` on one example and return its exit code. Output goes straight to the console."""

    argv = [str(CLI), "validate", str(path), "--config", name]
    if kind == "ecu":
        argv += ["--node", "ECU"]

    return subprocess.run(argv, check=False).returncode


def main():
    # Enable line buffering: the CLI subprocesses write to this same stdout directly.
    sys.stdout.reconfigure(line_buffering=True)

    if not CLI.is_file():
        # Worth catching here: run under the wrong interpreter this would otherwise surface as a bare FileNotFoundError per example.
        print(f"No flync CLI next to the interpreter at {CLI}. Run this through the project environment: uv run python {__file__}", file=sys.stderr)
        return 1

    print(f"===== Validating FLYNC examples under {EXAMPLES_DIR} =====")

    examples = list(_discover(EXAMPLES_DIR))
    if not examples:
        print(f"No examples found under {EXAMPLES_DIR}", file=sys.stderr)
        return 1

    failed = []

    for path, kind in examples:
        name = path.relative_to(EXAMPLES_DIR).as_posix()
        code = _validate(path, kind, name)
        if code:
            print(f"  => {name} exited {code}")
            failed.append(name)

    print(f"\n===== {len(examples)} example(s): {len(failed)} failed =====")
    if failed:
        print("FAILED: " + ", ".join(failed), file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
