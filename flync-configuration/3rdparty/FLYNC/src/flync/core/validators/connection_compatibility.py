"""
Validators for the compatibility of interface/connection settings between two
components: MII, MACsec, gPTP, HTB, CBS/ingress streams and VLAN uniqueness.
"""

from flync.core.utils.exceptions import Category, err_major, err_minor
from flync.core.validators.generic import validate_list_items_unique


def validate_ingress_streams_fields(streams, location: str):
    """
    Raise err_minor if any stream carries an ipv or ats value.

    ``location`` is a human-readable label such as ``"compute node"`` or ``"controller interface"`` used in the error message.
    """

    for ingress_stream in streams:
        if ingress_stream.ipv is not None:
            raise err_minor(
                f"Validation Error in Ingress Streams. "
                f"Removing config from the interface. "
                f"Ingress stream {ingress_stream.name} "
                f"at the {location} should not have an ipv value.",
                category=Category.CONSISTENCY,
                error_number="007",
            )
        if ingress_stream.ats is not None:
            raise err_minor(
                f"Validation Error in Ingress Streams. "
                f"Removing config from the interface. "
                f"Ingress stream {ingress_stream.name} at the "
                f"{location} should not have an ats value",
                category=Category.CONSISTENCY,
                error_number="008",
            )
    return streams


def validate_vlan_ids_unique(virtual_interfaces, name: str):
    """
    Raise err_major if any VLAN ID appears more than once.
    """

    all_vlans = [vi.vlanid for vi in virtual_interfaces]
    list_label = f"VLAN IDs of virtual Controller Interface in interface {name}"
    validate_list_items_unique(all_vlans, list_label)


def validate_cbs_idleslopes_fit_portspeed(traffic_classes: list, port_speed: int):
    """
    Custom Validator for a list of Traffic Classes to check conformity to MII/MDI speed.

    Args:
        traffic_classes (list): List of element type `TrafficClass`.

        port_speed (int): MII or MDI speed of the port.

    Raises:
        err_major: The sum of idleslopes of all shapers on one port must be equal or lower than the port speed.

    Returns:
        list: Return list of traffic classes as received.
    """

    if not traffic_classes:
        return
    if not port_speed:
        raise err_major(
            "Cannot validate Traffic Classes! No port speed defined. Make sure to configure MII or MDI.",
            category=Category.REQUIRED,
            error_number="010",
        )

    sum_idleslopes = 0

    for tr_class in traffic_classes:
        if tr_class.selection_mechanisms and tr_class.selection_mechanisms.type == "cbs":
            sum_idleslopes += tr_class.selection_mechanisms.idleslope

    if sum_idleslopes > port_speed * 1000:
        raise err_major(
            ("The sum of idleslopes of all shapers on one port" + " cannot be higher than the link speed!"),
            category=Category.CONSISTENCY,
            error_number="011",
        )
    return traffic_classes


def validate_optional_mii_config_compatibility(comp1, comp2, id):
    """
    Custom validator for optional MII configuration compatibility between two components.

    Args:
        comp1 (object): First component that may contain a ``mii_config`` attribute.

        comp2 (object): Second component that may contain a ``mii_config`` attribute.

        id (Any): Identifier of the connection (used only in error messages).

    Raises:
        err_major: One component has an MII config while the other does not.

        err_major: Both components have an MII config but the *mode* values are identical. The modes must differ.

        err_major: Both components have an MII config but the *speed* values are different.

        err_major: Both components have an MII config but the *type* values are different.
    """

    if comp1 is None or comp2 is None:
        return

    mii_comp1 = comp1.mii_config
    mii_comp2 = comp2.mii_config

    # Neither side specifies MII
    if mii_comp1 is None and mii_comp2 is None:
        return

    # Look for wrong config variants: exactly one side declares an MII config
    if mii_comp1 is None or mii_comp2 is None:
        raise err_major(
            f"Invalid MII config in connection {id}: "
            f"{comp1.name} ↔ {comp2.name} "
            f"(MII mismatch for PHY type). Both or None of "
            f"the components should have a MII config",
            category=Category.COMPATIBILITY,
            error_number="012",
        )

    # Both sides use MII, so let us check it:
    _check_mii_pair_compatibility(comp1, comp2, mii_comp1, mii_comp2, id)


def _check_mii_pair_compatibility(comp1, comp2, mii_comp1, mii_comp2, id):
    """
    Compare the MII settings of two components that both carry an MII config.

    Args:
        comp1 (object): First component (used only in error messages).

        comp2 (object): Second component (used only in error messages).

        mii_comp1 (object): ``mii_config`` of ``comp1``.

        mii_comp2 (object): ``mii_config`` of ``comp2``.

        id (Any): Identifier of the connection (used only in error messages).

    Raises:
        err_major: The *mode* values are identical. The modes must differ.

        err_major: The *speed* values are different.

        err_major: The *type* values are different.
    """

    if mii_comp1.mode == mii_comp2.mode:
        raise err_major(
            f"Incompatible MII Mode: {comp1.name} ({mii_comp1.mode}) ↔ {comp2.name}({mii_comp2.mode})",
            category=Category.COMPATIBILITY,
            error_number="013",
        )
    if mii_comp1.speed != mii_comp2.speed:
        raise err_major(
            f"Incompatible MII Speed: {comp1.name} ({mii_comp1.speed}) ↔ {comp2.name}({mii_comp2.speed})",
            category=Category.COMPATIBILITY,
            error_number="014",
        )
    if mii_comp1.type != mii_comp2.type:
        raise err_major(
            f"Incompatible MII Type: {comp1.name} ({mii_comp1.type}) ↔ {comp2.name}({mii_comp2.type})",
            category=Category.COMPATIBILITY,
            error_number="015",
        )


def validate_compulsory_mii_config_compatibility(comp1, comp2, id):
    """
    Validator that enforces a **mandatory** MII configuration on both components and then checks optional compatibility.

    Args:
        comp1 (object): First component. Must have ``mii_config``.

        comp2 (object): Second component. Must have ``mii_config``.

        id (Any): Identifier of the connection (used only in error messages).

    Raises:
        err_major: Either component is missing a required MII configuration.

        err_major: Propagated from :func:`validate_optional_mii_config_compatibility` when the optional checks fail.
    """

    if not comp1.mii_config or not comp2.mii_config:
        raise err_major(
            f"Invalid MII config in connection {id}: {comp1.name} ↔ {comp2.name} (MII configuration missing).",
            category=Category.COMPATIBILITY,
            error_number="016",
        )
    validate_optional_mii_config_compatibility(comp1, comp2, id)


def validate_htb(comp, speed):
    """
    Validator that checks an HTB (Hierarchical Token Bucket) configuration against the physical link speed.

    Args:
        comp (object): Component that owns an ``htb`` attribute with ``child_classes``.

        speed (int): Link speed of the interface (same unit as the HTB rates).

    Raises:
        err_major: The sum of the ``rate`` values of all child classes exceeds the provided ``speed``.
    """

    if not comp or not speed or not comp.htb:
        return
    sum_child_rates = sum(child.rate for child in comp.htb.child_classes)
    if sum_child_rates > speed:
        raise err_major(
            f"Incompatible HTB config for {comp.name}: sum of all child class rates {sum_child_rates} should be less than link speed {speed}",
            category=Category.CONSISTENCY,
            error_number="017",
        )


def validate_macsec(comp1, comp2, id):
    """
    Validator for MACsec configuration compatibility between two components.

    Args:
        comp1 (object): First component: May contain a ``macsec_config``.

        comp2 (object): Second component: May contain a ``macsec_config``.

        id (Any): Identifier of the connection (used only in error messages).

    Raises:
        err_major: One component has a MACsec config while the other does not.

        err_major: MKA (Key Agreement) enabled state differs between the two components.

        err_major: ``macsec_mode`` differs between the two components.
    """

    if comp1 is None or comp2 is None:
        return

    macsec1 = comp1.macsec_config
    macsec2 = comp2.macsec_config

    # Neither side uses MACsec
    if macsec1 is None and macsec2 is None:
        return

    # Look for wrong config variants: exactly one side declares a MACsec config
    if macsec1 is None or macsec2 is None:
        configured, unconfigured = (comp1, comp2) if macsec1 is not None else (comp2, comp1)
        raise err_major(
            f"Incomplete MACsec config in connection {id}: {configured.name} has a macsec config "
            f"but {unconfigured.name} does not. Both or none of the components should have one.",
            category=Category.COMPATIBILITY,
            error_number="018",
        )

    # Both sides use MACsec, so let us check it:
    _check_macsec_pair_compatibility(comp1, comp2, macsec1, macsec2, id)


def _check_macsec_pair_compatibility(comp1, comp2, macsec1, macsec2, id):
    """
    Compare the MACsec settings of two components that both carry a MACsec config.

    Args:
        comp1 (object): First component (used only in error messages).

        comp2 (object): Second component (used only in error messages).

        macsec1 (object): ``macsec_config`` of ``comp1``.

        macsec2 (object): ``macsec_config`` of ``comp2``.

        id (Any): Identifier of the connection (used only in error messages).

    Raises:
        err_major: MKA (Key Agreement) enabled state differs between the two components.

        err_major: ``macsec_mode`` differs between the two components.
    """

    if (not macsec1.mka_enabled and macsec2.mka_enabled) or (macsec1.mka_enabled and not macsec2.mka_enabled):
        raise err_major(
            f"MACsec should be enabled in both - {comp1.name} and {comp2.name} in connection {id} ",
            category=Category.COMPATIBILITY,
            error_number="019",
        )

    if macsec1.macsec_mode != macsec2.macsec_mode:
        raise err_major(
            f"Both {comp1.name} and {comp2.name} should have the same macsec_mode. in connection {id} ",
            category=Category.COMPATIBILITY,
            error_number="020",
        )


def validate_gptp(comp1, comp2, id):
    """
    Validator that checks gPTP (generic Precision Time Protocol) configuration compatibility between two components.

    Args:
        comp1 (object): First component. May contain a ``ptp_config``.

        comp2 (object): Second component. May contain a ``ptp_config``.

        id (Any): Identifier of the connection (used only in error messages).

    Raises:
        err_major: PTP configuration present on one side only.

        err_major: Mismatch of the ``cmlds_linkport_enabled`` flag between the two components.

        err_major: Propagated from :func:`validate_gptp_domains` when domain level checks fail.
    """

    if comp1 is None or comp2 is None:
        return

    ptp1 = comp1.ptp_config
    ptp2 = comp2.ptp_config

    if bool(ptp1) != bool(ptp2):
        raise err_major(
            f"Incompatible PTP config. PTP config not present in either {comp1.name} or  {comp2.name} in connection {id} ",
            category=Category.COMPATIBILITY,
            error_number="021",
        )

    # Neither side uses PTP, so there is nothing to compare
    if not ptp1:
        return

    validate_gptp_domains(comp1, comp2, ptp1, ptp2, id)
    validate_gptp_domains(comp2, comp1, ptp2, ptp1, id)

    if ptp1.cmlds_linkport_enabled != ptp2.cmlds_linkport_enabled:
        raise err_major(
            f"CMLDS mismatch: {comp1.name} has "
            f"cmlds_linkport_enabled="
            f"{ptp1.cmlds_linkport_enabled}, but "
            f"{comp2.name} has "
            f"{ptp2.cmlds_linkport_enabled}",
            category=Category.COMPATIBILITY,
            error_number="022",
        )


def validate_gptp_domains(comp1, comp2, ptp1, ptp2, id):
    """
    Helper that validates matching PTP domains and sync-config types between two components.

    Args:
        comp1 (object): First component (source of ``ptp1``).

        comp2 (object): Second component (source of ``ptp2``).

        ptp1 (object): ``ptp_config`` of ``comp1``.

        ptp2 (object): ``ptp_config`` of ``comp2``.

        id (Any): Identifier of the connection (used only in error messages).

    Raises:
        err_major: A domain present in ``ptp1`` is missing in ``ptp2``.

        err_major: The ``sync_config.type`` of a matching domain is identical on both sides (they must differ for a valid configuration).
    """

    if not comp1 or not comp2 or not ptp1 or not ptp2:
        return

    for ptp_port_iface in ptp1.ptp_ports:
        domain = ptp_port_iface.domain_id
        ptp_port_iface2 = next(
            (p for p in ptp2.ptp_ports if p.domain_id == domain),
            None,
        )
        if ptp_port_iface2 is None:
            raise err_major(
                f"Incompatible PTP Config: Domain {domain} not present in {comp2.name} in connection {id}",
                category=Category.COMPATIBILITY,
                error_number="023",
            )
        if ptp_port_iface.sync_config and ptp_port_iface2.sync_config and ptp_port_iface.sync_config.type == ptp_port_iface2.sync_config.type:
            raise err_major(
                f"Incompatible PTP Config: Domain ID {domain} in {comp1.name} and {comp2.name} in connection {id}",
                category=Category.COMPATIBILITY,
                error_number="024",
            )
