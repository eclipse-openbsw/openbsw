"""Sort the signals and nodes of a DBC file.

Loads the input DBC with cantools and writes it back out with cantools,
which re-serialises every message so that its signals are ordered by start
bit (descending). Signals sharing the same start bit are tie-broken with a
human/natural sort on their name (``..._KEY_ID_10`` before ``..._KEY_ID_2``),
so the output is stable regardless of the input order. The nodes in the
``BU_:`` line are also sorted by name. All other content (messages, value
tables, comments) is preserved by the cantools round-trip.

Usage:
    python scripts/helpers/dbc_sorter.py <input.dbc> <output.dbc>
"""

import argparse
import re
import sys

import cantools

_DIGIT_RUN = re.compile(r"(\d+)")


def _natural_key(text: str) -> list:
    """Return a sort key that orders embedded numbers numerically.

    ``..._KEY_ID_10`` sorts before ``..._KEY_ID_2`` instead of relying on
    byte-wise string comparison. Non-digit runs are compared case-insensitively.
    """

    def convert(part: str) -> int | str:
        return int(part) if part.isdigit() else part.casefold()

    return [convert(part) for part in _DIGIT_RUN.split(text)]


def _sort_signals(signals: list) -> list:
    """Sort signals by start bit (descending), tie-breaking on natural name order."""
    return sorted(
        signals,
        key=lambda signal: (-signal.start, _natural_key(signal.name)),
    )


def main() -> int:
    parser = argparse.ArgumentParser(description="Sort the signals within each PDU and the nodes of a DBC file via a cantools round-trip.")
    parser.add_argument("input_file", help="Path to the input DBC file.")
    parser.add_argument("output_file", help="Path to write the sorted DBC file to.")
    args = parser.parse_args()

    try:
        database = cantools.database.load_file(args.input_file)
    except Exception as exc:  # noqa: BLE001 - report the parse failure and exit
        print(f"Error: failed to load '{args.input_file}': {exc}", file=sys.stderr)
        return 1

    database.nodes.sort(key=str)
    for message in database.messages:
        for signal in message.signals:
            signal.receivers.sort()

    try:
        # ``as_dbc_string`` is used instead of ``dump_file`` so that the same
        # natural sort is applied both to the signal definitions (``sort_signals``)
        # and to the metadata sections such as comments, ``BA_`` attribute values
        # and ``VAL_`` value tables (``sort_attribute_signals``). ``dump_file``
        # only forwards ``sort_signals`` and would leave the metadata in the
        # original byte-wise order.
        output = database.as_dbc_string(
            sort_signals=_sort_signals,
            sort_attribute_signals=_sort_signals,
        )
        with open(args.output_file, "w", encoding="cp1252", newline="", errors="replace") as fout:
            fout.write(output)
    except Exception as exc:  # noqa: BLE001 - report the write failure and exit
        print(f"Error: failed to write '{args.output_file}': {exc}", file=sys.stderr)
        return 1

    print(f"Wrote sorted DBC ({len(database.messages)} messages, {len(database.nodes)} nodes) to '{args.output_file}'")
    return 0


if __name__ == "__main__":
    sys.exit(main())
