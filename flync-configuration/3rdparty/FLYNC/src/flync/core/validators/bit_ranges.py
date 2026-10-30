"""
Validators for bit-range placement inside PDUs and CAN/LIN frames, plus the
value/from_value/to_value input-format check.
"""

from typing import Any, Callable, Iterable, List, Optional, Tuple

from flync.core.utils.exceptions import (
    Category,
    err_major,
    err_minor,
)

# ---------------------------------------------------------------------------
# Bit-range placement validators
# ---------------------------------------------------------------------------
#
# These helpers operate on a list of bit ranges expressed as
# ``(item_name, start_bit, end_bit_exclusive)`` tuples and are used to
# validate placements of :class:`SignalInstance` objects inside a PDU and of
# :class:`PDUInstance` objects inside a CAN/LIN frame (when the referenced
# PDU's length is known to the caller).

BitRange = Tuple[str, int, int]


def collect_bit_ranges(items: Iterable[Any], get_range: Callable[[Any], Optional[BitRange]]) -> List[BitRange]:
    """
    Build a list of ``(name, start_bit, end_bit_exclusive)`` ranges from ``items``.

    ``get_range(item)`` is called for every entry and should return either a
    ``(name, start_bit, end_bit_exclusive)`` tuple or ``None`` when the item
    is unplaced and should be skipped (e.g. a :class:`SignalInstance` with no
    ``bit_position``).
    """

    ranges: List[BitRange] = []
    for item in items:
        r = get_range(item)
        if r is not None:
            ranges.append(r)
    return ranges


def check_bit_ranges_within(context: str, ranges: Iterable[BitRange], max_bits: int) -> None:
    """
    Raise :func:`err_minor` when any range extends past ``max_bits``.

    ``context`` is a human-readable label of the container (PDU or frame
    name) used in the error message.
    """

    for item_name, start, end in ranges:
        if end > max_bits:
            raise err_minor(
                "{context}: '{item}' bit range [{start}, {end}) overflows length of {bits} bits",
                context=context,
                item=item_name,
                start=start,
                end=end,
                bits=max_bits,
                category=Category.VALUE_RANGE,
                error_number="029",
            )


def check_bit_ranges_no_overlap(context: str, ranges: List[BitRange]) -> None:
    """
    Raise :func:`err_minor` when any two ranges in ``ranges`` intersect.

    Ranges are half-open ``[start, end)``; two ranges overlap when
    ``start_a < end_b and start_b < end_a``.  ``context`` is included in the
    error message to identify the enclosing PDU or frame.
    """

    for i, (name_a, start_a, end_a) in enumerate(ranges):
        for j in range(i + 1, len(ranges)):
            name_b, start_b, end_b = ranges[j]
            if start_a < end_b and start_b < end_a:
                raise err_minor(
                    "{context}: '{a}' [{sa}, {ea}) and '{b}' [{sb}, {eb}) overlap",
                    context=context,
                    a=name_a,
                    sa=start_a,
                    ea=end_a,
                    b=name_b,
                    sb=start_b,
                    eb=end_b,
                    category=Category.CONSISTENCY,
                    error_number="030",
                )


def validate_value_input_format(data: dict) -> dict:
    """Validating combinations of 'value', 'from_value' and 'to_value'."""
    if not isinstance(data, dict):
        return data

    has_value = "value" in data
    has_from_value = "from_value" in data
    has_to_value = "to_value" in data

    if not has_value and not has_to_value and not has_from_value:
        raise err_major(
            "Field required: Either the field 'value' or the pair of 'from_value' and 'to_value' has to be defined.",
            category=Category.REQUIRED,
            error_number="031",
        )

    if has_value and has_to_value:
        raise err_major(
            "Invalid Combination: cannot use both 'value' and 'to_value' — either use 'value' for a single value, "
            "or 'from_value' and 'to_value' in a pair.",
            category=Category.CONSISTENCY,
            error_number="032",
        )

    if has_value and has_from_value:
        raise err_major(
            "Invalid Combination: cannot use both 'value' and 'from_value' — either use 'value' for a single value, "
            "or 'from_value' and 'to_value' in a pair.",
            category=Category.CONSISTENCY,
            error_number="033",
        )

    if (has_from_value and not has_to_value) or (has_to_value and not has_from_value):
        raise err_major(
            "Invalid Combination: 'from_value' and 'to_value' must be paired — either use 'value' for a single value, "
            "or 'from_value' and 'to_value' in a pair.",
            category=Category.CONSISTENCY,
            error_number="034",
        )

    return data
