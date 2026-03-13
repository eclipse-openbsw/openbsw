"""Determine clang-tidy execution scope for CI workflows.

This helper computes whether clang-tidy should run in:
- full mode: for pushes to main and merge_group events (analyze full repository scope)
- changed mode: for pull_request events (analyze changed C/C++ files only)

It writes both the filtered changed-file list and GitHub step outputs (mode, has_changes)
used by the workflow to choose between full scan, changed-scope scan, or skip path.
"""

import argparse
import json
import subprocess
import sys
from pathlib import Path


ZERO_SHA = "0000000000000000000000000000000000000000"
RELEVANT_SUFFIXES = (".c", ".cc", ".cpp", ".h", ".hh", ".hpp", ".hxx")
SUPPORTED_EVENTS = {"pull_request", "merge_group", "push"}


def parse_args(argv: list[str] | None = None) -> argparse.Namespace:
    """Parse CLI arguments for clang-tidy scope determination."""
    parser = argparse.ArgumentParser(description="Determine clang-tidy CI scope")
    parser.add_argument("--event-name", required=True, help="GitHub event name")
    parser.add_argument("--ref-name", required=True, help="GitHub ref name")
    parser.add_argument("--sha", required=True, help="GitHub commit SHA")
    parser.add_argument(
        "--event-path", type=Path, required=True, help="Path to GitHub event payload"
    )
    parser.add_argument(
        "--output-file",
        type=Path,
        required=True,
        help="File that will receive changed C/C++ source paths",
    )
    parser.add_argument(
        "--github-output",
        type=Path,
        required=True,
        help="Path to the GitHub Actions step output file",
    )
    return parser.parse_args(argv)


def load_event(event_path: Path) -> dict:
    """Load and validate the GitHub event payload JSON object."""
    try:
        with event_path.open("r", encoding="utf-8") as event_file:
            event = json.load(event_file)
    except FileNotFoundError as exc:
        raise RuntimeError(
            f"GitHub event payload file not found: {event_path}."
        ) from exc
    except json.JSONDecodeError as exc:
        raise RuntimeError(
            f"Invalid JSON in GitHub event payload {event_path}: {exc.msg} at line {exc.lineno} column {exc.colno}."
        ) from exc

    if not isinstance(event, dict):
        raise RuntimeError(f"GitHub event payload {event_path} must contain a JSON object.")

    return event


def determine_refs(event_name: str, sha: str, event: dict) -> tuple[str, str]:
    """Resolve base and head SHAs for supported GitHub events."""
    if event_name != "pull_request":
        raise RuntimeError(
            f"Unsupported GitHub event '{event_name}' for ref resolution. Supported event: pull_request."
        )

    head_sha = sha
    base_sha = event.get("pull_request", {}).get("base", {}).get("sha", "")

    if not base_sha:
        raise RuntimeError(f"Unable to determine base SHA for GitHub event '{event_name}'.")

    if not head_sha:
        raise RuntimeError("Unable to determine head SHA for clang-tidy scope calculation.")

    return base_sha, head_sha


def run_git_command(args: list[str]) -> str:
    """Run a git command and return stdout, raising RuntimeError on failure."""
    try:
        result = subprocess.run(
            ["git", *args],
            check=True,
            capture_output=True,
            text=True,
        )
    except FileNotFoundError as exc:
        raise RuntimeError("The 'git' executable is not available in the CI environment.") from exc
    except subprocess.CalledProcessError as exc:
        details = (exc.stderr or exc.stdout or "no additional output").strip()
        raise RuntimeError(
            f"Git command failed: git {' '.join(args)}: {details}"
        ) from exc

    return result.stdout


def list_changed_sources(base_sha: str, head_sha: str) -> list[str]:
    """Return sorted changed C/C++ file paths between base and head revisions."""
    if base_sha and base_sha != ZERO_SHA:
        try:
            merge_base = run_git_command(["merge-base", head_sha, base_sha]).strip()
            diff_output = run_git_command(
                ["diff", "--name-only", "--diff-filter=ACMR", f"{merge_base}...{head_sha}"]
            )
        except RuntimeError as exc:
            if "git merge-base" not in str(exc):
                raise

            # PR merge refs and merge-queue refs may not contain every original
            # commit object locally. Falling back to a direct base..head diff
            # still scopes clang-tidy to files changed by the checked-out ref.
            diff_output = run_git_command(
                ["diff", "--name-only", "--diff-filter=ACMR", f"{base_sha}..{head_sha}"]
            )
    else:
        diff_output = run_git_command(
            ["diff-tree", "--no-commit-id", "--name-only", "--diff-filter=ACMR", "-r", head_sha]
        )

    return sorted(
        {
            line.strip()
            for line in diff_output.splitlines()
            if line.strip().endswith(RELEVANT_SUFFIXES)
        }
    )


def write_changed_files(output_file: Path, changed_files: list[str]) -> None:
    """Write changed file paths to disk as a newline-separated list."""
    output_file.parent.mkdir(parents=True, exist_ok=True)
    output_file.write_text(
        "\n".join(changed_files) + ("\n" if changed_files else ""),
        encoding="utf-8",
    )


def write_github_outputs(github_output: Path, mode: str, has_changes: bool) -> None:
    """Append mode and has_changes outputs for GitHub Actions steps."""
    github_output.parent.mkdir(parents=True, exist_ok=True)
    with github_output.open("a", encoding="utf-8") as output_file:
        output_file.write(f"mode={mode}\n")
        output_file.write(f"has_changes={'true' if has_changes else 'false'}\n")


def main(argv: list[str] | None = None) -> int:
    """Execute scope selection flow and write workflow output artifacts."""
    args = parse_args(argv)
    args.output_file.parent.mkdir(parents=True, exist_ok=True)
    args.output_file.write_text("", encoding="utf-8")

    try:
        if (args.event_name == "push" and args.ref_name == "main") or args.event_name == "merge_group":
            write_github_outputs(args.github_output, "full", True)
            return 0

        if args.event_name == "push":
            # The workflow is intended to run push only on main. If triggered on
            # other branches, ignore silently instead of failing.
            write_github_outputs(args.github_output, "changed", False)
            return 0

        event = load_event(args.event_path)
        base_sha, head_sha = determine_refs(args.event_name, args.sha, event)
        changed_files = list_changed_sources(base_sha, head_sha)
    except RuntimeError as exc:
        print(exc, file=sys.stderr)
        return 1

    write_changed_files(args.output_file, changed_files)
    write_github_outputs(args.github_output, "changed", bool(changed_files))

    if changed_files:
        print("Changed files:")
        for changed_file in changed_files:
            print(changed_file)
    else:
        print("No modified source/header files found for clang-tidy.")

    return 0


if __name__ == "__main__":
    sys.exit(main())