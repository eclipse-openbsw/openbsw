"""Defines MACsec configuration for FLYNC."""

from typing import Annotated, List, Literal, Optional

from pydantic import AfterValidator, BeforeValidator, Field, PlainSerializer, field_validator, model_validator

from flync.core.base_models.base_model import FLYNCBaseModel
from flync.core.datatypes import Ethertype, FLYNCMacAddress, serialize_ethertype, validate_ethertype_input
from flync.core.utils.exceptions import Category, err_minor, warn
from flync.core.validators.address import validate_vlan_id


class CipherSuiteBaseModel(FLYNCBaseModel):
    """
    Common configuration items for MACsec cipher suites.

    Parameters
    ----------
    cipher_suite : Literal[GCM-AES-128, GCM-AES-256, GCM-AES-XPN-128, GCM-AES-XPN-256], optional
        MACsec Cipher Suite defined in IEEE (defaults to ``"GCM-AES-XPN-256"``).
    """

    cipher_suite: Optional[Literal["GCM-AES-128", "GCM-AES-256", "GCM-AES-XPN-128", "GCM-AES-XPN-256"]] = Field(default="GCM-AES-XPN-256")

    def xpn(self) -> bool:
        """Return True if the cipher suite uses extended packet numbering (XPN)."""
        return self.cipher_suite in ("GCM-AES-XPN-128", "GCM-AES-XPN-256")


class IntegrityWithoutConfidentiality(CipherSuiteBaseModel):
    """
    Cipher configuration representing integrity protection without confidentiality.

    This configuration supports authentication and integrity checks but does not encrypt the data.

    Parameters
    ----------
    type : Literal["integrity_without_confidentiality"]
        Identifier for the cipher type. Always ``"integrity_without_confidentiality"``.

    confidentiality_offset : Literal[0], optional
        Preference for offset timing (defaults to ``0``). Always 0 for this cipher.
    """

    type: Literal["integrity_without_confidentiality"] = Field(default="integrity_without_confidentiality")
    confidentiality_offset: Optional[Literal[0]] = Field(default=0)


class IntegrityWithConfidentiality(CipherSuiteBaseModel):
    """
    Cipher configuration representing both integrity protection and confidentiality.

    This configuration includes both encryption and authentication features.

    Parameters
    ----------
    type : Literal["integrity_with_confidentiality"]
        Identifier for the cipher type.
        Always ``"integrity_with_confidentiality"``.

    confidentiality_offset : Literal[0, 30, 50], optional
        Confidentiality Offset preference for transmission in bytes (defaults to ``0``).
        Allows choosing between no offset, 30 bytes, or 50 bytes.
    """

    type: Literal["integrity_with_confidentiality"] = Field(default="integrity_with_confidentiality")
    confidentiality_offset: Optional[Literal[0, 30, 50]] = Field(default=0)

    @model_validator(mode="after")
    def validate_xpn_confidentiality_offset(self):
        """Ensure XPN ciphers do not use a non-zero confidentiality offset."""
        if self.xpn() and self.confidentiality_offset != 0:
            raise err_minor(
                "XPN Ciphers do not support Confidentiality Offset other than 0.",
                category=Category.CONSISTENCY,
                error_number="253",
            )
        return self


DiscriminatedCipher = Annotated[
    IntegrityWithoutConfidentiality | IntegrityWithConfidentiality,
    Field(discriminator="type"),
]


class MACsecConfig(FLYNCBaseModel):
    """
    Configuration for MACsec (Media Access Control Security).

    Includes global MKA (MACsec Key Agreement) settings and per-port security configuration.

    Parameters
    ----------
    vlan_bypass : list of int
        VLANs which shall not be protected with MACsec.

    ethertype_bypass : list of :class:`~flync.core.datatypes.ethertypes.Ethertype`, optional
        Ethertypes which shall not be protected with MACsec (defaults to ``[]``).

    src_mac_address_bypass : list of :class:`~flync.core.datatypes.macaddress.FLYNCMacAddress`, optional
        Source MAC addresses which shall not be protected with MACsec (defaults to ``[]``).

    dest_mac_address_bypass : list of :class:`~flync.core.datatypes.macaddress.FLYNCMacAddress`, optional
        Destination MAC addresses which shall not be protected with MACsec (defaults to ``[]``).

    ckn : str
        Connectivity Association Key Name (CKN) used to identify the CAK. 1-32 octets
        (characters in the range 0x00-0xFF).

    mka_enabled : bool
        Whether MACsec Key Agreement (MKA) is enabled. Default is True.

    hello_time : int
        MKPDU period when a connection is established, applicable when delay_protect is disabled (milliseconds).

    bounded_hello_time : int
        Hello time applicable with delay_protect enabled (milliseconds).

    life_time : int
        Life time for a peer to transmit MKPDU's in order to consider it alive (milliseconds).

    sak_retire_time : int
        During a key rotation, time to retire the previous SAK key (milliseconds).

    hello_time_rampup : list of int, optional
        Periods between initial MKA messages after linkup in milliseconds (defaults to ``[]``).

    sak_rekey_time : int, optional
        Minimum interval in seconds before rekeying the SAK (defaults to ``3``).

    macsec_mode : Literal["disabled", "integrity", \
    "integrity_confidentiality"]
        MACsec operation mode. Options include disabled, integrity-only, and full encryption.

    kay_on : bool
        Whether to activate the KaY (Key Agreement Entity) module.
        When disabled, MACsec is not negotiated.

    key_role : Literal["key_server_always", "key_server_never"]
        Role of the device in key negotiation.

    delay_protect : bool
        When enabled, performs frequent updates of the packet number on the receiving side to prevent attackers from delaying MACsec frames.

    participant_activation : Literal["disabled", "onoperup", "always"]
        Strategy for participant activation.

    sci_included : bool, optional
        Whether to include the Secure Channel Identifier (SCI) in MACsec frames (defaults to ``False``).

    replay_protection_window : int, optional
        Size of the replay protection window (defaults to ``0``). Any value other than ``0``
        emits a warning.

    cipher_preference : list of :class:`DiscriminatedCipher`
        List of preferred ciphers to negotiate, ordered by priority.
        Defaults to using integrity-only without confidentiality.
    """

    vlan_bypass: List[Annotated[int, Field(ge=1), AfterValidator(validate_vlan_id)]] = Field()
    ethertype_bypass: List[Annotated[Ethertype, PlainSerializer(serialize_ethertype), BeforeValidator(validate_ethertype_input)]] = Field([])
    src_mac_address_bypass: List[FLYNCMacAddress] = Field([])
    dest_mac_address_bypass: List[FLYNCMacAddress] = Field([])
    ckn: str = Field(min_length=1, max_length=32)
    mka_enabled: Optional[bool] = Field(default=True)
    hello_time: int = Field()
    bounded_hello_time: int = Field()
    life_time: int = Field()
    sak_retire_time: int = Field()
    hello_time_rampup: List[int] = Field([])
    sak_rekey_time: Optional[int] = Field(default=3, ge=0)
    macsec_mode: Literal["disabled", "integrity", "integrity_confidentiality"] = Field()
    kay_on: bool = Field()
    key_role: Literal["key_server_always", "key_server_never"] = Field()
    delay_protect: bool = Field()
    participant_activation: Literal["disabled", "onoperup", "always"] = Field()
    sci_included: Optional[bool] = Field(default=False)
    replay_protection_window: int = Field(default=0, ge=0)
    cipher_preference: List[DiscriminatedCipher] = Field(default_factory=lambda: MACsecConfig.default_entries_list())

    @field_validator("ckn")
    @classmethod
    def validate_ckn_octets(cls, value: str) -> str:
        """Validate that the CKN only contains octets (characters in range 0x00-0xFF)."""
        if any(ord(char) > 0xFF for char in value):
            raise err_minor(
                "ckn must only contain octets (characters in range 0x00-0xFF), got {ckn}",
                category=Category.FORMAT,
                error_number="252",
                ckn=value,
            )
        return value

    def ckn_to_byte_array(self) -> bytearray:
        """Return the CKN as a byte array, one byte per octet."""
        return bytearray(ord(char) for char in self.ckn)

    @model_validator(mode="after")
    def warn_replay_protection_window_non_zero(self):
        """Warn if the replay protection window is set to a non-zero value."""
        if self.replay_protection_window != 0:
            warn(
                "replay_protection_window values not 0 are considered unsecure, got {replay_protection_window}",
                category=Category.VALUE_RANGE,
                error_number="251",
                replay_protection_window=self.replay_protection_window,
            )
        return self

    @model_validator(mode="after")
    def validate_mka_macsecmode_disabled(self):
        """Warn if MACsec is enabled while MKA is disabled."""
        if not self.mka_enabled and self.macsec_mode != "disabled":
            warn(
                "If MACsec is enabled, you should also enable MKA.",
                category=Category.CONSISTENCY,
                error_number="100",
            )
        return self

    @model_validator(mode="after")
    def validate_life_time_greater_than_hello_time(self):
        """Ensure life time is greater than hello time."""
        if self.life_time < self.hello_time:
            raise err_minor("Life time should be greater than hello time.", category=Category.CONSISTENCY, error_number="101")
        return self

    @staticmethod
    def default_entries_list() -> list[IntegrityWithoutConfidentiality | IntegrityWithConfidentiality]:
        """Return the default cipher preference list (integrity without confidentiality)."""
        entries: list[IntegrityWithoutConfidentiality | IntegrityWithConfidentiality] = [IntegrityWithoutConfidentiality()]
        return entries
