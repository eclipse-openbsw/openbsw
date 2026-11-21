"""Defines the automotive Ethernet Switch and its components for FLYNC"""

from __future__ import annotations

from typing import (
    Annotated,
    Any,
    List,
    Literal,
    Optional,
    Self,
)

from pydantic import (
    AfterValidator,
    BeforeValidator,
    Field,
    PrivateAttr,
    StrictInt,
    field_serializer,
    field_validator,
    model_validator,
)

from flync.core.annotations import (
    External,
    Implied,
    ImpliedStrategy,
    NamingStrategy,
    OutputStrategy,
)
from flync.core.base_models.base_model import FLYNCBaseModel
from flync.core.datatypes import Bitmask
from flync.core.utils.exceptions import Category, err_minor, warn
from flync.core.validators import traffic_classes as traffic_class_validators
from flync.core.validators.address import validate_vlan_id
from flync.core.validators.connection_compatibility import validate_cbs_idleslopes_fit_portspeed
from flync.core.validators.generic import (
    none_to_empty_list,
    validate_elements_in,
    validate_list_items_unique,
    validate_or_remove,
)
from flync.model.flync_4_ecu.controller import Controller
from flync.model.flync_4_ecu.phy import (
    BASET,
    BASET1,
    BASET1S,
    MII,
    RGMII,
    RMII,
    SGMII,
    XFI,
)
from flync.model.flync_4_ecu.vlan_entry import VLANEntry
from flync.model.flync_4_metadata import EmbeddedMetadata
from flync.model.flync_4_security import MACsecConfig
from flync.model.flync_4_tsn import (
    FrameFilter,
    PTPConfig,
    Stream,
    TrafficClass,
)


class SwitchPort(FLYNCBaseModel):
    """
    Represents a Switch Port and its configuration.

    Parameters
    ----------
    name : str
        Name of the Switch Port.

    silicon_port_no : int
        Silicon hardware port number (vendor-specific).
        Must be greater or equal to 0.

    default_vlan_id : int
        VLAN ID to be added to an untagged frame ingressing on the port (>= 0 and <= 4095).
        Use ``None`` for an untagged port (no default VLAN).

    mii_config : :class:`~flync.model.flync_4_ecu.phy.MII` or :class:`~flync.model.flync_4_ecu.phy.RMII` or \
    :class:`~flync.model.flync_4_ecu.phy.SGMII` or :class:`~flync.model.flync_4_ecu.phy.RGMII`, optional
        Media-independent interface configuration (e.g., MII or RMII).

    ptp_config : :class:`~flync.model.flync_4_tsn.PTPConfig`, optional
        Precision Time Protocol configuration.

    ingress_streams : list of :class:`~flync.model.flync_4_tsn.Stream`, optional
        Stream-based IEEE 802.1Qci configuration.

    traffic_classes : list of :class:`~flync.model.flync_4_tsn.TrafficClass`, optional
        Traffic class definitions and traffic shaping configuration applied to egress port queues.

    macsec_config : :class:`~flync.model.flync_4_security.MACsecConfig`, optional
        MACsec configuration for the port.

    Private Attributes
    ------------------
    _type :
        The type of the object generated. Set to controller_interface.

    _mdi_config : :class:`~flync.model.flync_4_ecu.phy.BaseT1` or :class:`~flync.model.flync_4_ecu.phy.BaseT1S` or \
    :class:`~flync.model.flync_4_ecu.phy.BaseT`

    _connected_component:
        The switch port, controller interface or ecu port connected to the switch port.
        This attribute is managed internally and is not part of the public API.

    """

    name: str = Field()
    silicon_port_no: int = Field(ge=0)
    default_vlan_id: int = Field(..., ge=0, le=4095)
    mii_config: Optional[MII | RMII | SGMII | RGMII | XFI] = Field(default=None, discriminator="type")
    ptp_config: Annotated[
        Optional[PTPConfig],
        BeforeValidator(validate_or_remove("PTP config", PTPConfig)),
    ] = Field(default=None)
    ingress_streams: Annotated[
        Optional[List[Stream]],
        BeforeValidator(validate_or_remove("ingress streams", List[Stream])),
        BeforeValidator(none_to_empty_list),
    ] = Field(default=[])
    traffic_classes: Annotated[
        Optional[List[TrafficClass]],
        AfterValidator(traffic_class_validators.validate_traffic_classes),
        BeforeValidator(validate_or_remove("traffic classes", List[TrafficClass])),
        BeforeValidator(none_to_empty_list),
    ] = Field(default=[])
    macsec_config: Annotated[
        Optional[MACsecConfig],
        BeforeValidator(validate_or_remove("MACsec config", MACsecConfig)),
    ] = Field(default=None)
    _mdi_config: BASET1 | BASET1S | BASET | None = None
    _connected_component: Optional[Any] = PrivateAttr(default=None)
    _type: Literal["switch_port"] = PrivateAttr(default="switch_port")
    _switch: Optional["Switch"] = PrivateAttr(default=None)

    @property
    def mdi_config(self):
        return self._mdi_config

    @property
    def type(self):
        return self._type

    @property
    def connected_component(self):
        return self._connected_component

    @property
    def switch(self) -> Optional["Switch"]:
        return self._switch

    @model_validator(mode="after")
    def validate_traffic_classes(self):
        if self.mii_config and self.traffic_classes:
            validate_cbs_idleslopes_fit_portspeed(
                self.traffic_classes,
                self.mii_config.speed,
            )
        return self

    def copy_mdi_config_to_switch(self, mdi_config):
        """
        Helper function.
        Copies the MDI config from ECU port to switch port.
        """

        self._mdi_config = mdi_config

    def get_switch(self):
        """
        Helper function.
        Returns the switch that the port is a part of.
        """

        if self._switch is None:
            raise err_minor(f"The switch port {self.name} is not a part of any switch", category=Category.STRUCTURAL, error_number="087")
        return self._switch

    def get_vlan_connected_ports(self, vlan):
        """
        Helper function.
        Returns the switch ports that are part of the same VLAN as that port.
        """

        ports = []
        for vlan_entry in self.get_switch().vlans:
            if vlan_entry.id == vlan:
                ports.extend(vlan_entry.ports)
        ports_obj = [sp for sport in ports for sp in self.get_switch().ports if sp.name == sport]
        return ports_obj

    def is_part_of_vlan(self, vlan):
        for vlan_entry in self.get_switch().vlans:
            if vlan_entry.id == vlan and self.name in vlan_entry.ports:
                return True
        return False


class PortScopedAction(FLYNCBaseModel):
    """
    Base class for TCAM actions whose ``ports`` list defaults to all switch ports
    when left empty or undefined.

    The expanded port list is made available at runtime (see
    :meth:`Switch._fill_empty_tcam_port_lists`), while the field serializer ensures
    serialization reflects the user's original input: an empty list is dumped as an
    empty list, and an omitted list stays omitted (under ``exclude_unset``).

    Parameters
    ----------
    ports : list of str, optional
        Switch ports to which this action applies. Defaults to all ports if omitted.
    """

    ports: Annotated[Optional[List[str]], BeforeValidator(none_to_empty_list)] = Field(default_factory=list)
    _ports_autofilled: bool = False

    @field_serializer("ports", check_fields=False)
    def serialize_ports(self, ports: Optional[List[str]], _info):
        """Dump the user's original port list, not the runtime-expanded one."""
        return [] if self._ports_autofilled else ports


class Drop(PortScopedAction):
    """
    Action that discards traffic on the selected egress ports.

    Parameters
    ----------
    type : Literal["drop"]
        Discriminator used by Pydantic.

    ports : list of str, Optional
        Egress ports where the drop action should be applied. Defaults to all ports of the switch if kept empty or undefined.
    """

    type: Literal["drop"] = Field(default="drop")


class Mirror(PortScopedAction):
    """
    Action that mirrors incoming traffic to additional egress ports.

    Parameters
    ----------
    type : Literal["mirror"]
        Discriminator used by Pydantic.

    ports : list of str, Optional
        Egress ports that will receive the mirrored traffic. Defaults to all ports of the switch if kept empty or undefined.
    """

    type: Literal["mirror"] = Field(default="mirror")


class ForceEgress(PortScopedAction):
    """
    Action that forces a packet to leave through a given set of ports, bypassing the normal forwarding decision.

    Parameters
    ----------
    type : Literal["force_egress"]
        Discriminator used by Pydantic.

    ports : list of str, Optional
        Egress ports to which the messages are force-forwarded. Defaults to all ports of the switch if kept empty or undefined.
    """

    type: Literal["force_egress"] = Field(default="force_egress")


class VLANOverwrite(PortScopedAction):
    """
    Action that overwrites VLAN ID and/or PCP values on selected ports.

    Parameters
    ----------
    type : Literal["vlan_overwrite"]
        Discriminator used by Pydantic.

    overwrite_vlan_id : int, optional
        New VLAN identifier (0-4095).
        If ``None``, the VLAN ID is left unchanged.

    overwrite_vlan_pcp : int, optional
        New PCP value (0-7). If ``None``, the PCP value is left unchanged.

    ports : list of str, Optional
        Egress ports at which the overwriting should take place. Defaults to all ports of the switch if kept empty or undefined.
    """

    type: Literal["vlan_overwrite"] = Field(default="vlan_overwrite")
    overwrite_vlan_id: Annotated[Optional[int], AfterValidator(validate_vlan_id)] = Field(default=None)
    overwrite_vlan_pcp: Optional[int] = Field(default=None)


class RemoveVLAN(PortScopedAction):
    """
    Action that removes the VLAN tag from packets on the given ports.

    Parameters
    ----------
    type : Literal["remove_vlan"]
        Discriminator used by Pydantic.

    ports : list of str, Optional
        Egress ports where the VLAN tag will be removed. Defaults to all ports of the switch if kept empty or undefined.
    """

    type: Literal["remove_vlan"] = Field(default="remove_vlan")


class FrameMask(Bitmask):
    """
    Byte-level pattern matching of an Ethernet frame, i.e. a :class:`~flync.core.datatypes.Bitmask` bound to a frame position.

    A frame matches if ``(frame[offset:offset + byte_length] & mask) == data``, e.g. a
    two-byte pattern at ``offset=12`` matches on the EtherType field.

    Parameters
    ----------

    offset : int
        Byte position in the frame where the pattern match begins.
        Must be greater or equal to 0.
    """

    offset: int = Field(ge=0)

    @property
    def byte_length(self) -> int:
        """Number of frame bytes inspected, derived from the bit width of the ``data`` literal."""
        return -(-self._width // 8)


class TCAMRule(FLYNCBaseModel):
    """
    Definition of a TCAM (ternary content-addressable memory) rule for a
    switch.

    Parameters
    ----------
    name : str
        Name for the description of the TCAM rule (minimum length 1).

    id : StrictInt
        Unique TCAM rule ID.
        Must be greater or equal to 0.

    match_filter : :class:`~flync.model.flync_4_tsn.FrameFilter`, optional
        Packet-matching filter for layer-based matching on MAC/IP/VLAN/ports.
        Mutually exclusive with ``frame_mask``.

    frame_mask : list of :class:`~FrameMask`, optional
        Packet-matching criteria for byte-level pattern matching on raw frame data.
        The masks of one rule must inspect disjoint byte ranges of the frame.

    frame_window : int, optional
        Maximum number of leading frame bytes the ``frame_mask`` entries may inspect.
        Unbounded when omitted.
        Must be greater or equal to 1.

    match_ports : list of str, Optional
        Ports to which the rule is bound. Defaults to all ports of the switch if kept empty or undefined.

    action : list of :class:`Drop` or :class:`Mirror` or :class:`ForceEgress` or :class:`VLANOverwrite` or :class:`RemoveVLAN`
        One or more actions performed when the rule matches.
        The ``type`` field of each action class acts as the discriminating key for Pydantic.

        A single port must not be targeted by more than one of *drop*, *force_egress* or
        *mirror*, nor by more than one of *remove_vlan* or *vlan_overwrite*.

    vehicle_state : :class:`~flync.core.datatypes.Bitmask`, optional
        Vehicle-state pattern the rule is gated on: it applies only when
        ``(current_state & vehicle_state.mask) == vehicle_state.data``. Both ``data`` and
        ``mask`` must fit in the 8-bit vehicle-state register. ``None`` (default) means the
        rule is not gated on vehicle state.
    """

    name: str = Field(min_length=1)
    id: StrictInt = Field(ge=0)
    match_filter: Optional[FrameFilter] = Field(default=None)
    frame_mask: Annotated[Optional[List[FrameMask]], BeforeValidator(none_to_empty_list)] = Field(default_factory=list)
    frame_window: Optional[int] = Field(default=None, ge=1)
    match_ports: Annotated[Optional[List[str]], BeforeValidator(none_to_empty_list)] = Field(default_factory=list)
    action: Annotated[Optional[List[(Drop | Mirror | VLANOverwrite | ForceEgress | RemoveVLAN)]], BeforeValidator(none_to_empty_list)] = Field(
        default_factory=list
    )
    vehicle_state: Optional[Bitmask] = Field(default=None)
    _match_ports_autofilled: bool = False

    @field_serializer("match_ports")
    def serialize_match_ports(self, match_ports: Optional[List[str]], _info):
        """Dump the user's original port list, not the runtime-expanded one."""
        return [] if self._match_ports_autofilled else match_ports

    @field_validator("vehicle_state")
    @classmethod
    def validate_vehicle_state_range(cls, value: Optional[Bitmask]) -> Optional[Bitmask]:
        """``vehicle_state`` is matched against an 8-bit register, so both ``data`` and ``mask`` must fit in a byte."""
        if value is not None and max(value.data, value.mask or 0) > 0xFF:
            raise err_minor(
                f"'vehicle_state' data and mask must each be <= 0xFF (255); got data={value.data}, mask={value.mask}",
                category=Category.VALUE_RANGE,
                error_number="231",
            )
        return value

    @model_validator(mode="after")
    def validate_match_filter_or_mask_exclusive(self) -> Self:
        """Validate that a rule matches either on layers (``match_filter``) or on raw bytes (``frame_mask``), not on both."""

        if self.match_filter is not None and self.frame_mask:
            raise err_minor("Cannot specify both match_filter and frame_mask; use only one", category=Category.STRUCTURAL, error_number="183")
        return self

    @model_validator(mode="after")
    def validate_frame_masks(self) -> Self:
        """Validate that the frame masks inspect disjoint byte ranges and stay inside ``frame_window``."""

        if self.frame_window is not None and not self.frame_mask:
            warn(
                f"TCAM rule {self.name}: 'frame_window' is defined but frame_mask is not. The frame window is not processed in this case.",
                category=Category.COMPATIBILITY,
                error_number="235",
            )

        masks = sorted(self.frame_mask or [], key=lambda frame_mask: frame_mask.offset)
        for previous, current in zip(masks, masks[1:]):
            if current.offset < previous.offset + previous.byte_length:
                raise err_minor(
                    f"TCAM rule {self.name}: frame_masks must not overlap; the mask at offset {previous.offset} covers "
                    f"{previous.byte_length} byte(s) and overlaps the mask at offset {current.offset}.",
                    category=Category.CONSISTENCY,
                    error_number="232",
                )

        for frame_mask in masks:
            if self.frame_window is not None and frame_mask.offset + frame_mask.byte_length > self.frame_window:
                raise err_minor(
                    f"TCAM rule {self.name}: the frame_mask at offset {frame_mask.offset} covers {frame_mask.byte_length} byte(s) and "
                    f"thus exceeds the frame_window of {self.frame_window} byte(s).",
                    category=Category.VALUE_RANGE,
                    error_number="233",
                )
        return self

    @model_validator(mode="after")
    def validate_exclusive_drop_force_mirror(self):
        """Validate that no port is targeted by more than one of the mutually-exclusive actions *drop*, *force_egress* or *mirror*."""

        all_ports = []
        for action in self.action:
            if action.type in ["drop", "force_egress", "mirror"]:
                all_ports += action.ports

        if len(all_ports) != len(set(all_ports)):
            raise err_minor(
                "A TCAM Rule can either drop OR force egress OR mirror on one port.",
                category=Category.CONSISTENCY,
                error_number="088",
            )
        return self

    @model_validator(mode="after")
    def validate_exclusive_vlan_action(self):
        """Validate that no port is targeted by both VLAN actions *remove_vlan* and *vlan_overwrite*."""

        all_ports = []
        for action in self.action:
            if action.type in ["remove_vlan", "vlan_overwrite"]:
                all_ports += action.ports

        if len(all_ports) != len(set(all_ports)):
            raise err_minor(
                "A TCAM Rule can either remove OR overwrite a vlan on one port.",
                category=Category.CONSISTENCY,
                error_number="089",
            )
        return self


class SwitchConfig(FLYNCBaseModel):
    """
    Core switch configuration data stored in ``switch.flync.yaml``.

    This model holds the switch metadata, ports, VLANs, and TCAM rules.
    It is loaded from the ``switch.flync.yaml`` file inside each switch folder.

    Parameters
    ----------
    meta : :class:`~flync.model.flync_4_metadata.metadata.EmbeddedMetadata`
        Metadata associated with the switch.

    ports : list of :class:`SwitchPort`
        List of switch ports.

    vlans : list of :class:`~flync.model.flync_4_ecu.vlan_entry.VLANEntry`
        List of VLAN entries configured on the switch.

    tcam_rules : list of :class:`TCAMRule`, optional
        List of TCAM rules configured on the switch.
    dynamic_address_aging_time : int, optional
        Aging time, in seconds, for dynamically learned address entries in the switch's Filtering Database (FDB).
        A learned MAC address is removed if no matching frame is seen within this interval.
        Must be a positive value.
    """

    meta: EmbeddedMetadata = Field()
    tcam_rules: Annotated[
        Optional[List[TCAMRule]],
        BeforeValidator(none_to_empty_list),
    ] = Field(default=[])
    ports: List[SwitchPort] = Field()
    vlans: List[VLANEntry] = Field()
    dynamic_address_aging_time: Optional[StrictInt] = Field(default=None, gt=0)


class Switch(FLYNCBaseModel):
    """
    Represents an automotive Ethernet network switch configuration.

    Parameters
    ----------
    name : str
        Name of the switch. Implied from the folder name.

    switch_config : :class:`SwitchConfig`
        The core switch configuration loaded from ``switch.flync.yaml``.

    host_controller : :class:`~flync.model.flync_4_ecu.controller.Controller`, optional
        The host controller managing the switch, stored in the
        ``switch_host_controller/`` sub-folder.

    """

    name: Annotated[
        str,
        Implied(strategy=ImpliedStrategy.FOLDER_NAME),
    ] = Field()
    switch_config: Annotated[
        SwitchConfig,
        External(
            output_structure=OutputStrategy.SINGLE_FILE | OutputStrategy.OMMIT_ROOT,
            naming_strategy=NamingStrategy.FIXED_PATH,
            path="switch",
        ),
    ] = Field()
    host_controller: Annotated[
        Optional["Controller"],
        External(
            output_structure=OutputStrategy.FOLDER,
            naming_strategy=NamingStrategy.FIXED_PATH,
            path="switch_host_controller",
        ),
    ] = Field(default=None)

    @property
    def meta(self) -> EmbeddedMetadata:
        """Proxy to the underlying switch config metadata."""
        return self.switch_config.meta

    @property
    def tcam_rules(self) -> Optional[List[TCAMRule]]:
        """Proxy to the underlying switch config TCAM rules."""
        return self.switch_config.tcam_rules

    @property
    def ports(self) -> List[SwitchPort]:
        """Proxy to the underlying switch config ports."""
        return self.switch_config.ports

    @property
    def vlans(self) -> List[VLANEntry]:
        """Proxy to the underlying switch config VLANs."""
        return self.switch_config.vlans

    @model_validator(mode="after")
    def validate_unique_port_number(self):
        """
        Validate if the silicon port numbers for all the different switch ports are unique

        Raises:
            Validation error if a silicon port number is repeated
        """

        silicon_port_numbers = []
        for port in self.ports:
            silicon_port_numbers.append(port.silicon_port_no)
        validate_list_items_unique(
            silicon_port_numbers,
            "Switch Ports (silicon_port_number)",
        )
        return self

    @model_validator(mode="after")
    def validate_unique_port_names(self):
        """Validate port names are unique across this switch's ports."""
        validate_list_items_unique(
            [p.name for p in self.ports],
            "Switch Ports (name)",
        )
        return self

    def _has_stream_with_ipv(self, ipv) -> bool:
        """
        Check whether any ingress stream on any port of this switch uses the given internal priority value.

        Args:
            ipv: The internal priority value to look for.

        Returns:
            bool: True if a matching ingress stream exists, False otherwise.
        """

        return any(stream.ipv == ipv for p in self.ports for stream in (p.ingress_streams or []))

    def _has_ats_stream(self) -> bool:
        """
        Check whether any ingress stream on any port of this switch defines an ATS instance.

        Returns:
            bool: True if an ingress stream with an ATS instance exists, False otherwise.
        """

        return any(stream.ats for p in self.ports for stream in (p.ingress_streams or []))

    @model_validator(mode="after")
    def validate_ipv_mapping(self) -> Self:
        """
        Check if internal priority value of traffic classes is defined in ingress streams
        """

        for port in self.ports:
            for tr in port.traffic_classes or []:
                for iv in tr.internal_priority_values or []:
                    if not self._has_stream_with_ipv(iv):
                        raise err_minor(
                            f"Not able to find any streams with internal priority values {iv}. Traffic class {tr.name}",
                            category=Category.REFERENCE,
                            error_number="090",
                        )
        return self

    @model_validator(mode="after")
    def validate_ats_instances(self) -> Self:
        """
        Check if the shaper is ATS, the instance is defined # on some port on ingress

        Raises:
            err_minor: No ATS Instance found for traffic class

        Returns:
            _type_: Self
        """

        for port in self.ports:
            for tr in port.traffic_classes or []:
                if tr.selection_mechanisms and tr.selection_mechanisms.type == "ats" and not self._has_ats_stream():
                    raise err_minor(f"No ATS Instance found for traffic class {tr.name}", category=Category.REFERENCE, error_number="091")

        return self

    @model_validator(mode="after")
    def validate_ports_in_tcam_exist(self):
        """
        Validate that every port referenced in TCAM rules exists on the switch.

        Raises:
            err_minor: If a port listed in a TCAM rule (match_ports or action.ports) is not present in the switch's port list.
        """

        if not self.tcam_rules:
            return self
        switch_port_names = [port.name for port in self.ports]
        tcam_ports = []
        for tcam_rule in self.tcam_rules:
            tcam_ports += tcam_rule.match_ports
            for action in tcam_rule.action:
                tcam_ports += action.ports

        validate_elements_in(
            tcam_ports,
            switch_port_names,
            "TCAM Ports must exist on the Switch.",
        )
        return self

    @model_validator(mode="after")
    def validate_ports_in_vlan_entries_exist(self):
        """
        Validate that every port referenced in VLAN entries exists on the switch.

        Raises:
            err_minor: If a port listed in a VLAN entry (ports or multicast.ports) is not present in the switch's port list.
        """

        if not self.vlans:
            return self
        switch_port_names = [port.name for port in self.ports]
        vlan_ports = []
        for vlan_entry in self.vlans:
            vlan_ports += vlan_entry.ports
            for multicast_group in vlan_entry.multicast or []:
                vlan_ports += multicast_group.ports

        validate_elements_in(
            vlan_ports,
            switch_port_names,
            "VLAN Ports must exist on the Switch.",
        )
        return self

    @model_validator(mode="after")
    def validate_tcam_ids_unique(self):
        """
        Validate that each TCAM rule has a unique identifier.

        Raises:
            err_minor: Duplicate ``id`` values found among the TCAM rules.
        """

        ids = [tcam.id for tcam in self.tcam_rules]
        validate_list_items_unique(ids, "tcam_rules (id)")
        return self

    @model_validator(mode="after")
    def validate_tcam_name_unique(self):
        """
        Validate that each TCAM rule has a unique name.

        Raises:
            err_minor: Duplicate ``name`` values found among the TCAM rules.
        """

        names = [tcam.name for tcam in self.tcam_rules]
        validate_list_items_unique(names, "tcam_rules (name)")

        return self

    def get_mac(self):
        """Return MAC address from the host controller's first ethernet interface."""
        macs = self.host_controller.get_all_macs()
        return macs[0] if macs else None

    def find_switch_port(self, port_name: str) -> SwitchPort:
        return next(p for p in self.ports if p.name == port_name)

    def model_post_init(self, __context):
        for port in self.ports:
            port._switch = self

        self._fill_empty_tcam_port_lists()
        return super().model_post_init(__context)

    def _fill_empty_tcam_port_lists(self):
        """
        Fill empty or omitted port lists in TCAM rules with all available switch ports.

        The expanded list is assigned directly via ``object.__setattr__`` so it is fully
        accessible at runtime without being recorded in the model's "fields set". This keeps
        serialization faithful to the user's original input: the field serializers on
        :class:`TCAMRule` and :class:`PortScopedAction` dump an explicit empty list as ``[]``,
        while an omitted list stays omitted (under ``exclude_unset``).
        """
        if not self.tcam_rules:
            return

        all_port_names = [port.name for port in self.ports]

        for rule in self.tcam_rules:
            if not rule.match_ports:
                object.__setattr__(rule, "match_ports", all_port_names.copy())
                object.__setattr__(rule, "_match_ports_autofilled", True)

            for action in rule.action:
                if not action.ports:
                    object.__setattr__(action, "ports", all_port_names.copy())
                    object.__setattr__(action, "_ports_autofilled", True)

            # The port-based exclusivity checks ran on the (still empty) user input, so re-run them on the expanded lists.
            rule.validate_exclusive_drop_force_mirror()
            rule.validate_exclusive_vlan_action()
