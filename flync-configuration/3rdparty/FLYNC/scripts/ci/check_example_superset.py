"""
Example superset gate for the FLYNC SDK.

``flync_example_experimental`` is intended to be a sandbox copy of the canonical
``flync_example``: it must contain everything ``flync_example`` has, plus its own
experimental additions. This script enforces that invariant by walking every real
file under ``flync_example`` and asserting the same relative path exists in
``flync_example_experimental`` with identical content. It exits non-zero on any
drift so CI can flag a standard-example change that was not mirrored.

Paths git ignores are skipped (developers keep local work under ``examples/`` that
CI never sees) -- this keeps macOS ``.DS_Store`` cruft from tripping the gate.

An allowlist names files that are *intentionally* different in the experimental
copy, where the experimental restructure is the point (e.g. the ``eth_ecu``
interface was reorganised onto hypervisor compute nodes). Keep this list as short
as possible and comment each entry.
"""

import subprocess
import sys
from pathlib import Path

EXAMPLES_DIR = Path(__file__).resolve().parents[2] / "examples"

STANDARD = EXAMPLES_DIR / "flync_example"
EXPERIMENTAL = EXAMPLES_DIR / "flync_example_experimental"

# Files that are allowed to differ between the standard and experimental examples.
# Each entry is a relative path under examples/. The experimental copy is the
# deliberately-modified one; keep the list minimal and justify every entry here.
ALLOWED_DIFFERING = {
    # eth_ecu_c1_iface1 was restructured in the experimental example: the
    # interface-level virtual_interfaces were moved onto hypervisor compute_nodes.
    # This should go away as soon as compute node is integrated
    "ecus/eth_ecu/controllers/eth_ecu_controller1/ethernet_interfaces/"
    "eth_ecu_c1_iface1/interface_config.flync.yaml",
}


def _git_ignored(paths):
    """Return the paths git ignores. An empty set if git cannot answer, which errs toward checking too much."""

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
    return {Path(line) for line in done.stdout.splitlines()} if done.returncode in (0, 1) else set()


def _walk_files(root):
    """Yield every regular file under *root* whose path git does not ignore, as a Path."""

    all_files = [p for p in root.rglob("*") if p.is_file()]
    ignored = _git_ignored(all_files)
    for path in all_files:
        if path not in ignored:
            yield path


def main():
    if not STANDARD.is_dir() or not EXPERIMENTAL.is_dir():
        print(f"Expected both example workspaces under {EXAMPLES_DIR}: {STANDARD.name} and {EXPERIMENTAL.name}", file=sys.stderr)
        return 1

    print(f"===== Checking {EXPERIMENTAL.name} is a superset of {STANDARD.name} =====")

    missing = []
    changed = []

    for path in sorted(_walk_files(STANDARD)):
        rel = path.relative_to(STANDARD)
        target = EXPERIMENTAL / rel
        if not target.is_file():
            missing.append(rel)
            continue
        if path.read_bytes() != target.read_bytes():
            changed.append(rel)

    # Of the content differences, only those not on the allowlist count as drift.
    reported = [p for p in changed if p.as_posix() not in ALLOWED_DIFFERING]

    for rel in missing:
        print(f"  MISSING   {rel}")
    for rel in reported:
        print(f"  DIFFERS   {rel}")

    allowed_reported = len(changed) - len(reported)
    summary = f"{len(missing)} missing, {len(reported)} differing"
    if allowed_reported:
        summary += f" ({allowed_reported} allowed on allowlist)"
    print(f"\n===== {summary} =====")

    if missing or reported:
        print(
            "FAILED: flync_example_experimental must contain every file of flync_example. "
            "Mirror the new/changed standard-example files into the experimental copy "
            "(or, only if the difference is a deliberate experimental restructure, add it to ALLOWED_DIFFERING).",
            file=sys.stderr,
        )
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
