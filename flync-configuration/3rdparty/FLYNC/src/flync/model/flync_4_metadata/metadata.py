"""Defines metadata models for FLYNC"""

from typing import Dict, Literal, Optional

from packaging.version import InvalidVersion
from packaging.version import Version as Pep440Version
from pydantic import Field, field_serializer, model_validator
from semver import Version as SemVersion

from flync.core.base_models.base_model import FLYNCBaseModel
from flync.core.utils.exceptions import Category, err_major


class BaseVersion(FLYNCBaseModel):
    """
    Represents a version descriptor used within the model.

    Parameters
    ----------
    version_schema : Literal["semver", "pep440"], optional
        Versioning scheme that defines how the version string is interpreted (defaults to ``"semver"``).

    version : str
        Version value expressed according to the selected ``version_schema``. Must be provided as a raw string.

    """

    version_schema: Optional[Literal["semver", "pep440"]] = Field(default="semver")
    version: str = Field(examples=["0.0.1"])

    @field_serializer("version")
    def serialize_version(self, v: str):
        return str(v)

    @model_validator(mode="after")
    def validate_and_parse_version(self):
        raw_version = self.version

        if self.version_schema == "pep440":
            try:
                parsed = raw_version if isinstance(raw_version, Pep440Version) else Pep440Version(str(raw_version))
            except InvalidVersion as e:
                raise err_major(f"Version '{raw_version}' is not valid PEP 440", category=Category.FORMAT, error_number="092") from e

        elif self.version_schema == "semver":
            try:
                parsed = raw_version if isinstance(raw_version, SemVersion) else SemVersion.parse(str(raw_version))
            except ValueError as e:
                raise err_major(f"Version '{raw_version}' is not valid Semantic Version", category=Category.FORMAT, error_number="093") from e

        object.__setattr__(self, "version", parsed)
        return self


class SoftwareBaseMetadata(BaseVersion):
    """
    Represents software-related metadata.

    This model describes versioning information for software components.
    """


class HardwareBaseMetadata(BaseVersion):
    """
    Represents hardware-related metadata.

    This model describes supplier and versioning information for hardware components.

    Parameters
    ----------
    supplier : str, optional
        Name of the hardware supplier.

    product_id : str, optional
        Supplier-specific part identification.
    """

    supplier: Optional[str] = Field(default=None)
    product_id: Optional[str] = Field(default=None)


class BaseMetadata(FLYNCBaseModel):
    """
    Base class for model and system metadata definitions.

    This class provides common metadata attributes shared across different configuration artifacts, such as systems, ECUs, and services.
    It defines identifying and versioning information used for traceability and documentation.

    Parameters
    ----------
    type : str
        Type identifier of the metadata object.

    author : str
        Author or organization responsible for the entity definition.

    compatible_flync_version : :class:`~flync.model.flync_4_metadata.metadata.BaseVersion`
        FLYNC version with which this model is compatible.

    extensions : dict of str to str, optional
        Optional map of extension keys and values for custom or tool-specific metadata.
    """

    type: str = Field()
    author: str = Field(examples=["N/A"])
    compatible_flync_version: BaseVersion = Field()
    extensions: Optional[Dict[str, str]] = Field(default=None)


class SystemMetadata(BaseMetadata):
    """
    Represents system-level metadata.

    This metadata describes the overall system context, including OEM and platform information.

    Parameters
    ----------
    type: Literal["system"].
        Literal identifier specifying System metadata.

    oem : str, optional
        Original Equipment Manufacturer responsible for the system.

    platform : str, optional
        Target platform or system family identifier.

    variant : str, optional
        System variant of the platform.

    release: :class:`~flync.model.flync_4_metadata.metadata.BaseVersion`
        Versioning information about the system.

    """

    type: Literal["system"] = Field("system")
    oem: Optional[str] = Field(default=None)
    platform: Optional[str] = Field(default=None)
    variant: Optional[str] = Field(default=None)
    release: BaseVersion = Field()


class ECUMetadata(BaseMetadata):
    """
    Represents metadata for an Electronic Control Unit (ECU).

    This metadata combines system-level identification with optional hardware and software descriptions.

    Parameters
    -----------

    type : Literal["ecu"]
        Literal identifier specifying ECU metadata.

    hardware : \
        :class:`~flync.model.flync_4_metadata.metadata.HardwareBaseMetadata`\
        | None, optional
        Hardware metadata associated with the ECU.

    software : \
        :class:`~flync.model.flync_4_metadata.metadata.SoftwareBaseMetadata`\
        | None , optional
        Software metadata associated with the ECU.
    """

    type: Literal["ecu"] = Field("ecu")
    hardware: Optional[HardwareBaseMetadata] = Field(default=None)
    software: Optional[SoftwareBaseMetadata] = Field(default=None)


class EmbeddedMetadata(BaseMetadata):
    """
    Represents metadata for an embedded platform.

    Parameters
    ----------
    type : Literal["embedded"]
        Literal identifier specifying an embedded device.

    hardware : :class:`~flync.model.flync_4_metadata.metadata.HardwareBaseMetadata` | None , optional
        Hardware metadata associated with the embedded device.

    app : :class:`~flync.model.flync_4_metadata.metadata.SoftwareBaseMetadata` | None , optional
        Software metadata for the application.

    bootloader : :class:`~flync.model.flync_4_metadata.metadata.SoftwareBaseMetadata` | None , optional
        Software metadata for the bootloader.

    target_system : str
        Name of the Embedded target device.
    """

    type: Literal["embedded"] = Field("embedded")
    hardware: Optional[HardwareBaseMetadata] = Field(default=None)
    app: Optional[SoftwareBaseMetadata] = Field(default=None)
    bootloader: Optional[SoftwareBaseMetadata] = Field(default=None)
    target_system: str = Field(examples=["AARCH"])


class SocketsPerVLANMetadata(BaseMetadata):
    """
    Represents metadata for sockets-per-VLAN configuration.

    Parameters
    ----------
    type: Literal["sockets_per_vlan"]
        Literal identifier specifying sockets-per-VLAN metadata.
    """

    type: Literal["sockets_per_vlan"] = Field("sockets_per_vlan")


class SOMEIPServiceMetadata(BaseMetadata):
    """
    Represents metadata for a SOME/IP service interface.

    Parameters
    ----------
    type: Literal["someip_service"]
        Literal identifier specifying SOME/IP service metadata.
    """

    type: Literal["someip_service"] = Field("someip_service")
