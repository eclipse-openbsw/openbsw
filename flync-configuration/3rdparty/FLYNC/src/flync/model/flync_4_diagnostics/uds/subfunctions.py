"""
ISO 14229-1 sub-function name tables shared by the UDS service models.

Every UDS service that offers sub-functions accepts them as *either* the ISO snake_case name
*or* the raw numeric value, because the sub-function ranges are only partly standardised -
EcuReset 0x40-0x5F and InputOutputControlByIdentifier 0x04-0xFF are supplier- or
manufacturer-specific and must still load. The ``Literal`` aliases below therefore always
appear as ``List[Union[<Alias>, int]]`` on a service, never as a bare ``Literal``.

The ``*_IDS`` maps translate a name to its on-the-wire value; they are the source for
exporters and for the docs, and are deliberately kept next to the ``Literal`` they describe
so the two cannot drift.
"""

from typing import Any, Iterable, List, Literal

#: The EcuReset (0x11) ``resetType`` values standardised by ISO 14229-1. Values 0x40-0x5F are
#: system-supplier specific and are written as plain integers instead.
ResetType = Literal[
    "hard_reset",
    "key_off_on_reset",
    "soft_reset",
    "enable_rapid_power_shutdown",
    "disable_rapid_power_shutdown",
]

#: :data:`ResetType` name to ``resetType`` sub-function value.
RESET_TYPE_IDS: dict[str, int] = {
    "hard_reset": 0x01,
    "key_off_on_reset": 0x02,
    "soft_reset": 0x03,
    "enable_rapid_power_shutdown": 0x04,
    "disable_rapid_power_shutdown": 0x05,
}

#: The ReadDTCInformation (0x19) ``reportType`` values standardised by ISO 14229-1.
DTCReportType = Literal[
    "report_number_of_dtc_by_status_mask",
    "report_dtc_by_status_mask",
    "report_dtc_snapshot_identification",
    "report_dtc_snapshot_record_by_dtc_number",
    "report_dtc_stored_data_by_record_number",
    "report_dtc_ext_data_record_by_dtc_number",
    "report_number_of_dtc_by_severity_mask_record",
    "report_dtc_by_severity_mask_record",
    "report_severity_information_of_dtc",
    "report_supported_dtc",
    "report_first_test_failed_dtc",
    "report_first_confirmed_dtc",
    "report_most_recent_test_failed_dtc",
    "report_most_recent_confirmed_dtc",
    "report_mirror_memory_dtc_by_status_mask",
    "report_mirror_memory_dtc_ext_data_record_by_dtc_number",
    "report_number_of_mirror_memory_dtc_by_status_mask",
    "report_number_of_emissions_obd_dtc_by_status_mask",
    "report_emissions_obd_dtc_by_status_mask",
    "report_dtc_fault_detection_counter",
    "report_dtc_with_permanent_status",
    "report_dtc_ext_data_record_by_record_number",
    "report_user_def_memory_dtc_by_status_mask",
    "report_user_def_memory_dtc_snapshot_record_by_dtc_number",
    "report_user_def_memory_dtc_ext_data_record_by_dtc_number",
    "report_supported_dtc_ext_data_record",
    "report_wwh_obd_dtc_by_mask_record",
    "report_wwh_obd_dtc_with_permanent_status",
    "report_dtc_information_by_dtc_readiness_group_identifier",
]

#: :data:`DTCReportType` name to ``reportType`` sub-function value.
DTC_REPORT_TYPE_IDS: dict[str, int] = {
    "report_number_of_dtc_by_status_mask": 0x01,
    "report_dtc_by_status_mask": 0x02,
    "report_dtc_snapshot_identification": 0x03,
    "report_dtc_snapshot_record_by_dtc_number": 0x04,
    "report_dtc_stored_data_by_record_number": 0x05,
    "report_dtc_ext_data_record_by_dtc_number": 0x06,
    "report_number_of_dtc_by_severity_mask_record": 0x07,
    "report_dtc_by_severity_mask_record": 0x08,
    "report_severity_information_of_dtc": 0x09,
    "report_supported_dtc": 0x0A,
    "report_first_test_failed_dtc": 0x0B,
    "report_first_confirmed_dtc": 0x0C,
    "report_most_recent_test_failed_dtc": 0x0D,
    "report_most_recent_confirmed_dtc": 0x0E,
    "report_mirror_memory_dtc_by_status_mask": 0x0F,
    "report_mirror_memory_dtc_ext_data_record_by_dtc_number": 0x10,
    "report_number_of_mirror_memory_dtc_by_status_mask": 0x11,
    "report_number_of_emissions_obd_dtc_by_status_mask": 0x12,
    "report_emissions_obd_dtc_by_status_mask": 0x13,
    "report_dtc_fault_detection_counter": 0x14,
    "report_dtc_with_permanent_status": 0x15,
    "report_dtc_ext_data_record_by_record_number": 0x16,
    "report_user_def_memory_dtc_by_status_mask": 0x17,
    "report_user_def_memory_dtc_snapshot_record_by_dtc_number": 0x18,
    "report_user_def_memory_dtc_ext_data_record_by_dtc_number": 0x19,
    "report_supported_dtc_ext_data_record": 0x1A,
    "report_wwh_obd_dtc_by_mask_record": 0x42,
    "report_wwh_obd_dtc_with_permanent_status": 0x55,
    "report_dtc_information_by_dtc_readiness_group_identifier": 0x56,
}

#: Report types ISO 14229-1:2020 deprecated in favour of the user-defined-memory reports
#: (0x17-0x19) and the WWH-OBD reports (0x42/0x55).
DEPRECATED_DTC_REPORT_TYPES: frozenset[str] = frozenset(
    {
        "report_mirror_memory_dtc_by_status_mask",
        "report_mirror_memory_dtc_ext_data_record_by_dtc_number",
        "report_number_of_mirror_memory_dtc_by_status_mask",
        "report_number_of_emissions_obd_dtc_by_status_mask",
        "report_emissions_obd_dtc_by_status_mask",
    }
)

#: Report types that address one of the user-defined memories, and therefore need the
#: offering service to declare which memories exist.
USER_DEF_MEMORY_DTC_REPORT_TYPES: frozenset[str] = frozenset(
    {
        "report_user_def_memory_dtc_by_status_mask",
        "report_user_def_memory_dtc_snapshot_record_by_dtc_number",
        "report_user_def_memory_dtc_ext_data_record_by_dtc_number",
    }
)

#: Report types that return a DTC snapshot (freeze frame) record.
SNAPSHOT_DTC_REPORT_TYPES: frozenset[str] = frozenset(
    {
        "report_dtc_snapshot_identification",
        "report_dtc_snapshot_record_by_dtc_number",
        "report_user_def_memory_dtc_snapshot_record_by_dtc_number",
    }
)

#: Report types that return a DTC extended data record.
EXT_DATA_DTC_REPORT_TYPES: frozenset[str] = frozenset(
    {
        "report_dtc_ext_data_record_by_dtc_number",
        "report_dtc_ext_data_record_by_record_number",
        "report_user_def_memory_dtc_ext_data_record_by_dtc_number",
        "report_supported_dtc_ext_data_record",
    }
)

#: Report types that select DTCs by their severity, and therefore only make sense when the
#: DTCs the server reports actually carry a severity.
SEVERITY_DTC_REPORT_TYPES: frozenset[str] = frozenset(
    {
        "report_number_of_dtc_by_severity_mask_record",
        "report_dtc_by_severity_mask_record",
        "report_severity_information_of_dtc",
    }
)

#: The bits of the ISO 14229-1 ``DTCStatusAvailabilityMask``, least significant bit first.
DTCStatusBit = Literal[
    "test_failed",
    "test_failed_this_operation_cycle",
    "pending_dtc",
    "confirmed_dtc",
    "test_not_completed_since_last_clear",
    "test_failed_since_last_clear",
    "test_not_completed_this_operation_cycle",
    "warning_indicator_requested",
]

#: :data:`DTCStatusBit` name to its bit position in the ``DTCStatusAvailabilityMask``.
DTC_STATUS_BITS: dict[str, int] = {
    "test_failed": 0,
    "test_failed_this_operation_cycle": 1,
    "pending_dtc": 2,
    "confirmed_dtc": 3,
    "test_not_completed_since_last_clear": 4,
    "test_failed_since_last_clear": 5,
    "test_not_completed_this_operation_cycle": 6,
    "warning_indicator_requested": 7,
}

#: The InputOutputControlByIdentifier (0x2F) ``inputOutputControlParameter`` values
#: standardised by ISO 14229-1. Values 0x04-0xFF are vehicle-manufacturer specific.
IOControlParameter = Literal[
    "return_control_to_ecu",
    "reset_to_default",
    "freeze_current_state",
    "short_term_adjustment",
]

#: :data:`IOControlParameter` name to ``inputOutputControlParameter`` value.
IO_CONTROL_PARAMETER_IDS: dict[str, int] = {
    "return_control_to_ecu": 0x00,
    "reset_to_default": 0x01,
    "freeze_current_state": 0x02,
    "short_term_adjustment": 0x03,
}

#: The single TesterPresent (0x3E) sub-function. Documented for exporters; TesterPresent
#: carries no configuration, so no service model declares it as a field.
TesterPresentSubfunction = Literal["zero_sub_function"]

#: The ControlDTCSetting (0x85) sub-functions. Both are mandatory whenever the service is
#: offered, so - like TesterPresent - no service model declares them as a field.
DTCSettingMode = Literal["on", "off"]

#: :data:`DTCSettingMode` name to ``DTCSettingType`` sub-function value.
DTC_SETTING_MODE_IDS: dict[str, int] = {"on": 0x01, "off": 0x02}

#: The functional group DTC masks ISO 14229-1 reserves for ClearDiagnosticInformation (0x14).
#: Any other 3-byte mask is a legitimate vehicle-manufacturer specific group.
ISO_DTC_GROUPS: dict[int, str] = {
    0xFFFF33: "emissions_system_group",
    0xFFFFD0: "safety_system_group",
    0xFFFFFE: "vobd_system",
    0xFFFFFF: "all_groups",
}

#: The "all groups" mask every ClearDiagnosticInformation service is expected to accept.
ALL_DTC_GROUPS: int = 0xFFFFFF


def coerce_int(value: Any) -> Any:
    """
    Normalise an integer written as a string (e.g. ``"0x19"``, ``"25"``) to an ``int``.

    Anything that is not a parsable string is returned unchanged, so the field's own
    validation reports it. Sub-function *names* are strings too and are deliberately left
    alone - they are parsed by the ``Literal`` half of the field's union.
    """

    if isinstance(value, str):
        try:
            return int(value, 0)
        except ValueError:
            return value
    return value


def status_mask_from_bits(bits: Iterable[str]) -> int:
    """
    Return the ``DTCStatusAvailabilityMask`` byte with the named :data:`DTCStatusBit` bits set.

    The mask is configured as the byte itself rather than as a list of names, so that the YAML
    shape matches the model field. OEM specifications name the bits, so this helper (and its
    inverse :func:`status_mask_bits`) is how callers move between the two.

    Raises
    ------
    KeyError
        If a name is not a :data:`DTCStatusBit`.
    """

    mask = 0
    for bit in bits:
        mask |= 1 << DTC_STATUS_BITS[bit]
    return mask


def status_mask_bits(mask: int) -> List[str]:
    """
    Return the :data:`DTCStatusBit` names set in ``mask``, least significant bit first.

    The inverse of :func:`status_mask_from_bits`.
    """

    return [name for name, bit in DTC_STATUS_BITS.items() if mask & (1 << bit)]


def resolve_subfunction_ids(values: Iterable[str | int], ids: dict[str, int]) -> List[int]:
    """
    Return the on-the-wire sub-function values of ``values``.

    Names are translated through ``ids``; integers - the escape hatch for supplier- and
    manufacturer-specific sub-functions - are passed through unchanged.
    """

    return [ids[value] if isinstance(value, str) else value for value in values]
