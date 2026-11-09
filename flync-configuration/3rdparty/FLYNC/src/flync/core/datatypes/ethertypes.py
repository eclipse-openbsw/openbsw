"""Defines the Ethertype datatype including its validation and serialization."""

from enum import Enum


class Ethertype(Enum):
    """EtherType values relevant for automotive and TSN Ethernet networks.

    Each member maps a protocol to its IEEE-registered EtherType (the 16-bit
    field in the Ethernet header that identifies the upper-layer protocol).

    **Supported EtherTypes:**

    ============== ======== ===========================================================================
    Name           Hex      Description
    ============== ======== ===========================================================================
    ARP            0x0806   Address Resolution Protocol
    ASAM_CMP       0x99FE   ASAM Capture Module Protocol
    AVTP           0x22F0   IEEE 1722 Audio Video Transport Protocol
    EAPoL          0x888E   EAP over LAN including MACsec Key Agreement (MKA)
    ETH_FLOWCTRL   0x8808   Ethernet flow control (PAUSE / IEEE 802.3x)
    HSR            0x892F   High-availability Seamless Redundancy (IEC 62439-3)
    IPv4           0x0800   Internet Protocol version 4
    IPv6           0x86DD   Internet Protocol version 6
    LLDP           0x88CC   Link Layer Discovery Protocol (IEEE 802.1AB)
    MACsec         0x88E5   MAC Security (IEEE 802.1AE)
    PRP            0x88FB   Parallel Redundancy Protocol supervision (IEC 62439-3)
    PTP            0x88F7   Precision Time Protocol / gPTP (IEEE 1588 / 802.1AS)
    QinQ           0x88A8   Outer VLAN tag (IEEE 802.1ad)
    SRP            0x22EA   Stream Reservation Protocol (IEEE 802.1Qat)
    VLAN           0x8100   Inner VLAN tag (IEEE 802.1Q)
    WAKE_ON_LAN    0x0842   Wake-on-LAN magic packet (ECU wakeup)
    ============== ======== ===========================================================================

    **Usage:**

    The Ethertype can be specified in three ways:

    - By enum member: ``Ethertype.AVTP``
    - By name string: ``"AVTP"``
    - By hex value: ``0x22F0`` or ``"0x22F0"``

    **Example:**

    .. code-block:: python

        from flync.core.datatypes import Ethertype

        # All of these are equivalent:
        et1 = Ethertype.AVTP
        et2 = Ethertype(0x22F0)
        et3 = Ethertype["AVTP"]

        # When used in a model, serialization returns hex format:
        # {"ethertype": "0x22F0"}

    **Use in Pydantic models:**

    ``eth_type: Annotated[Ethertype, PlainSerializer(serialize_ethertype), BeforeValidator(validate_ethertype_input)]``
    """

    ARP = 0x0806
    ASAM_CMP = 0x99FE
    AVTP = 0x22F0
    EAPoL = 0x888E
    ETH_FLOWCTRL = 0x8808
    HSR = 0x892F
    IPv4 = 0x0800
    IPv6 = 0x86DD
    LLDP = 0x88CC
    MACsec = 0x88E5
    PRP = 0x88FB
    PTP = 0x88F7
    QinQ = 0x88A8
    SRP = 0x22EA
    VLAN = 0x8100
    WAKE_ON_LAN = 0x0842


def validate_ethertype_input(value):
    """
    Accept an :class:`Ethertype`, a member name (``"AVTP"``) or a numeric value (``0x22F0``).
    Returns the Ethertype member. Handles both single values and lists.
    """

    if isinstance(value, list):
        return [validate_ethertype_input(v) for v in value]

    result = value

    if isinstance(value, str):
        name = value.strip()
        if name in Ethertype.__members__:
            result = Ethertype[name]
        else:
            try:
                result = Ethertype(int(name, 0))
            except ValueError as e:
                valid_names = ", ".join(Ethertype.__members__.keys())
                valid_hex = ", ".join(f"0x{et.value:04X}" for et in Ethertype)
                raise ValueError(f"Invalid ethertype value: {value!r}. Valid names: {valid_names}. Valid hex values: {valid_hex}.") from e
    elif isinstance(value, int):
        try:
            result = Ethertype(value)
        except ValueError as e:
            valid_hex = ", ".join(f"0x{et.value:04X}" for et in Ethertype)
            raise ValueError(f"Invalid ethertype value: 0x{value:04X}. Valid hex values: {valid_hex}.") from e

    return result


def serialize_ethertype(value: Ethertype | list[Ethertype]):
    """
    Serialization for members of :class:`Ethertype` or lists of :class:`Ethertype`.
    Returns the hex value of the Ethertype.
    """

    if isinstance(value, list):
        return [serialize_ethertype(v) for v in value]

    if isinstance(value, Ethertype):
        return f"0x{value.value:04X}"

    return value
