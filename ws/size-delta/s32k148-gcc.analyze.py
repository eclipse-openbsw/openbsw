# *******************************************************************************
# Copyright (c) 2026 Accenture
#
# This program and the accompanying materials are made available under the
# terms of the Apache License Version 2.0 which is available at
# https://www.apache.org/licenses/LICENSE-2.0
#
# SPDX-License-Identifier: Apache-2.0
# *******************************************************************************

import argparse
import json
import re
import subprocess
import tempfile
from dataclasses import dataclass
from pathlib import Path
from typing import Any

OBJDUMP = "/opt/arm-gnu-toolchain/bin/arm-none-eabi-objdump"
OBJCOPY = "/opt/arm-gnu-toolchain/bin/arm-none-eabi-objcopy"
SNAPSHOT_PREFIX = "SIZE_SNAPSHOT="
SECTION_HEADER = re.compile(
    r"^\s*\d+\s+(\S+)\s+([0-9a-fA-F]+)\s+([0-9a-fA-F]+)\s+"
    r"([0-9a-fA-F]+)\s+[0-9a-fA-F]+\s+2\*\*\d+(?:\s+(.*\S))?\s*$"
)


@dataclass(frozen=True)
class Section:
    name: str
    size: int
    vma: int
    lma: int
    flags: frozenset[str]

    @property
    def rom(self) -> int:
        required = {"ALLOC", "LOAD", "CONTENTS"}
        return self.size if required <= self.flags else 0

    @property
    def ram(self) -> int:
        if "ALLOC" not in self.flags:
            return 0
        is_writable = "READONLY" not in self.flags
        is_relocated = self.vma != self.lma
        return self.size if is_writable or is_relocated else 0


@dataclass(frozen=True)
class Image:
    sections: tuple[Section, ...]
    binary_size: int

    @property
    def rom(self) -> int:
        return sum(section.rom for section in self.sections)

    @property
    def ram(self) -> int:
        return sum(section.ram for section in self.sections)


def read_sections(elf: Path) -> tuple[Section, ...]:
    result = subprocess.run(
        [OBJDUMP, "--section-headers", "--wide", str(elf)],
        check=True,
        capture_output=True,
        text=True,
    )
    lines = result.stdout.splitlines()
    sections: list[Section] = []

    for index, line in enumerate(lines):
        match = SECTION_HEADER.match(line)
        if match is None:
            continue

        flags_text = match.group(5)
        if flags_text is None and index + 1 < len(lines):
            flags_text = lines[index + 1]
        flags = frozenset(flag.strip() for flag in (flags_text or "").split(","))

        sections.append(
            Section(
                name=match.group(1),
                size=int(match.group(2), 16),
                vma=int(match.group(3), 16),
                lma=int(match.group(4), 16),
                flags=flags,
            )
        )

    if not sections:
        raise RuntimeError(f"objdump returned no sections for {elf}")
    return tuple(sections)


def read_image(elf: Path) -> Image:
    with tempfile.TemporaryDirectory() as directory:
        binary = Path(directory) / "app.referenceApp.bin"
        subprocess.run([OBJCOPY, "-O", "binary", str(elf), str(binary)], check=True)
        binary_size = binary.stat().st_size
    return Image(read_sections(elf), binary_size)


def image_payload(image: Image) -> dict[str, Any]:
    return {
        "rom": image.rom,
        "ram": image.ram,
        "binary": image.binary_size,
        "sections": [
            {"name": section.name, "rom": section.rom, "ram": section.ram}
            for section in image.sections
            if section.rom or section.ram
        ],
    }


def read_snapshot(record: Path) -> dict[str, Any]:
    for line in record.read_text().splitlines():
        if line.startswith(SNAPSHOT_PREFIX):
            return json.loads(line.removeprefix(SNAPSHOT_PREFIX))
    raise RuntimeError(f"no {SNAPSHOT_PREFIX} entry in {record}")


def signed(value: int) -> str:
    return f"{value:+d}"


def print_totals(baseline: dict[str, Any], current: dict[str, Any]) -> None:
    print("Metric          Baseline      Current        Delta")
    print("--------------  ------------  ------------  ------------")
    for name, key in (("ROM", "rom"), ("RAM", "ram"), ("Binary", "binary")):
        before = baseline[key]
        after = current[key]
        print(f"{name:<14}  {before:>12d}  {after:>12d}  {signed(after - before):>12}")


def print_section_deltas(baseline: dict[str, Any], current: dict[str, Any]) -> None:
    baseline_sections = {section["name"]: section for section in baseline["sections"]}
    current_sections = {section["name"]: section for section in current["sections"]}
    ordered_names = [section["name"] for section in current["sections"]]
    ordered_names.extend(
        section["name"]
        for section in baseline["sections"]
        if section["name"] not in current_sections
    )

    changed = []
    for name in ordered_names:
        before = baseline_sections.get(name, {"rom": 0, "ram": 0})
        after = current_sections.get(name, {"rom": 0, "ram": 0})
        if before["rom"] != after["rom"] or before["ram"] != after["ram"]:
            changed.append((name, before, after))

    print("\nChanged allocated sections")
    if not changed:
        print("(none)")
        return

    print("Section                   ROM before   ROM after       ΔROM  RAM before   RAM after       ΔRAM")
    print("------------------------  ----------  ----------  ---------  ----------  ----------  ---------")
    for name, before, after in changed:
        print(
            f"{name:<24}  {before['rom']:>10d}  {after['rom']:>10d}  "
            f"{signed(after['rom'] - before['rom']):>9}  {before['ram']:>10d}  "
            f"{after['ram']:>10d}  {signed(after['ram'] - before['ram']):>9}"
        )


def snapshot(elf: Path) -> None:
    print(SNAPSHOT_PREFIX + json.dumps(image_payload(read_image(elf)), separators=(",", ":")))


def compare(baseline_record: Path, current_record: Path) -> None:
    baseline = read_snapshot(baseline_record)
    current = read_snapshot(current_record)
    print("S32K148 GCC size delta (baseline -> current)\n")
    print_totals(baseline, current)
    print_section_deltas(baseline, current)
    print(
        "\nROM = loadable allocated section contents; "
        "RAM = writable or runtime-relocated allocated sections; "
        "Binary = raw objcopy output."
    )


def main() -> None:
    parser = argparse.ArgumentParser()
    subparsers = parser.add_subparsers(dest="command", required=True)
    snapshot_parser = subparsers.add_parser("snapshot")
    snapshot_parser.add_argument("elf", type=Path)
    compare_parser = subparsers.add_parser("compare")
    compare_parser.add_argument("baseline_record", type=Path)
    compare_parser.add_argument("current_record", type=Path)
    arguments = parser.parse_args()

    if arguments.command == "snapshot":
        snapshot(arguments.elf)
    else:
        compare(arguments.baseline_record, arguments.current_record)


if __name__ == "__main__":
    main()
