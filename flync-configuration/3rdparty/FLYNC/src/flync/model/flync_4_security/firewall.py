"""Defines firewall configuration models for FLYNC."""

from typing import Annotated, List, Literal, Optional

from pydantic import BeforeValidator, Field, field_validator, model_validator

from flync.core.base_models.base_model import FLYNCBaseModel
from flync.core.utils.exceptions import Category, err_minor
from flync.core.validators.generic import none_to_empty_list
from flync.model.flync_4_tsn.qos import FrameFilter


class FirewallRule(FLYNCBaseModel):
    """
    Defines a single firewall rule for matching and handling frames.

    Parameters
    ----------
    name : str
        A unique name identifying the rule.

    action : Literal['reject', 'accept', 'drop']
        The action to take when the pattern matches.
        Can be one of ``'reject'``, ``'accept'``, or ``'drop'``.

    pattern : :class:`~flync.model.flync_4_tsn.qos.FrameFilter`
        The filter pattern used to match frames for this rule.
    """

    name: str = Field()
    action: Literal["reject", "accept", "drop"] = Field()
    pattern: FrameFilter = Field()

    @model_validator(mode="after")
    def validate_pattern(self):
        pattern = self.pattern
        if all(field is None for field in vars(pattern).values()):
            raise err_minor(
                "At least one of the fields in pattern of firewall rule should be present", category=Category.REQUIRED, error_number="094"
            )
        if pattern.dst_ipv4 is not None and pattern.dst_ipv6 is not None:
            raise err_minor("Firewall rule cannot have both dst ipv4 and dst ipv6 set", category=Category.CONSISTENCY, error_number="095")
        if pattern.src_ipv4 is not None and pattern.src_ipv6 is not None:
            raise err_minor("Firewall rule cannot have both src ipv4 and src ipv6 set", category=Category.CONSISTENCY, error_number="096")
        if pattern.src_ipv4 is not None and pattern.dst_ipv6 is not None:
            raise err_minor("Firewall rule cannot have both src ipv4 and dst ipv6 set", category=Category.CONSISTENCY, error_number="097")
        if pattern.src_ipv6 is not None and pattern.dst_ipv4 is not None:
            raise err_minor("Firewall rule cannot have both src ipv6 and dst ipv4 set", category=Category.CONSISTENCY, error_number="098")
        return self


class Firewall(FLYNCBaseModel):
    """
    Represents a set of firewall rules with a default action.

    Parameters
    ----------
    default_action : Literal['reject', 'accept', 'drop'], optional
        The action to apply to packets that do not match any rule (defaults to ``'reject'``).
        Can be one of ``'reject'``, ``'accept'``, or ``'drop'``.

    input_rules : list of :class:`FirewallRule`, optional
        A list of ``FirewallRule`` objects that define input traffic matching conditions and actions.

    output_rules : list of :class:`FirewallRule`, optional
        A list of ``FirewallRule`` objects that define output traffic matching conditions and actions.

    forward_rules : list of :class:`FirewallRule`, optional
        A list of ``FirewallRule`` objects that define forwarded traffic matching conditions and actions.
    """

    default_action: Optional[Literal["reject", "accept", "drop"]] = Field(default="reject")
    input_rules: Annotated[
        Optional[List[FirewallRule]],
        BeforeValidator(none_to_empty_list),
    ] = Field(default=[])
    output_rules: Annotated[
        Optional[List[FirewallRule]],
        BeforeValidator(none_to_empty_list),
    ] = Field(default=[])
    forward_rules: Annotated[
        Optional[List[FirewallRule]],
        BeforeValidator(none_to_empty_list),
    ] = Field(default=[])

    @field_validator("input_rules", "output_rules", "forward_rules")
    @classmethod
    def check_duplicate_rules(cls, rules: List[FirewallRule]) -> List[FirewallRule]:
        filter_list = []
        for rule in rules:
            if rule.pattern in filter_list:
                raise err_minor("Two or more rules cannot be the same", category=Category.UNIQUENESS, error_number="099")
            filter_list.append(rule.pattern)
        return rules
