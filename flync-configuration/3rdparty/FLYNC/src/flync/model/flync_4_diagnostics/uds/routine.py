"""Defines UDS Routines, controlled via RoutineControl (0x31)."""

from typing import Annotated, List, Literal, Optional

from pydantic import Field, model_validator

from flync.core.annotations import Implied, ImpliedStrategy
from flync.core.base_models import FLYNCBaseModel
from flync.core.utils.exceptions import Category, err_major

from .datatypes import DiagDataRecord

#: The RoutineControl (0x31) sub-functions a routine may implement.
RoutineSubFunction = Literal["start", "stop", "request_results"]


def _default_sub_functions() -> List[RoutineSubFunction]:
    """Return the default set of supported sub-functions: startRoutine only."""

    return ["start"]


class Routine(FLYNCBaseModel):
    """
    A UDS Routine, invoked via RoutineControl (0x31).

    Stored one per file under ``communication/diagnostics/uds/routines/``; the file name
    provides :attr:`name`.

    Parameters
    ----------
    name : str
        Name of the routine, implied from the file name on disk.

    rid : int
        The 16-bit routine identifier, e.g. ``0x0203``.

    supported_sub_functions : list of Literal["start", "stop", "request_results"], optional
        Which RoutineControl sub-functions the routine implements. Defaults to ``["start"]``.

    start_request : :class:`~flync.model.flync_4_diagnostics.uds.datatypes.DiagDataRecord`, optional
        Request data layout for RoutineControl startRoutine. Required when ``"start"`` is supported.

    start_response : :class:`~flync.model.flync_4_diagnostics.uds.datatypes.DiagDataRecord`, optional
        Positive response data layout for startRoutine.

    stop_request : :class:`~flync.model.flync_4_diagnostics.uds.datatypes.DiagDataRecord`, optional
        Request data layout for stopRoutine. Required when ``"stop"`` is supported.

    stop_response : :class:`~flync.model.flync_4_diagnostics.uds.datatypes.DiagDataRecord`, optional
        Positive response data layout for stopRoutine.

    request_results_response : :class:`~flync.model.flync_4_diagnostics.uds.datatypes.DiagDataRecord`, optional
        Positive response data layout for requestRoutineResults. Required when
        ``"request_results"`` is supported.

    access_profile : str, optional
        Name of the access profile (declared by the UDS server offering this routine)
        required to invoke it. ``None`` means that server's default access profile applies.

    description : str, optional
        Human-readable description of the routine.
    """

    name: Annotated[str, Implied(strategy=ImpliedStrategy.FILE_NAME)] = Field()
    rid: Annotated[int, Field(ge=0x0000, le=0xFFFF)] = Field()
    supported_sub_functions: List[RoutineSubFunction] = Field(default_factory=_default_sub_functions)
    start_request: Optional[DiagDataRecord] = Field(default=None)
    start_response: Optional[DiagDataRecord] = Field(default=None)
    stop_request: Optional[DiagDataRecord] = Field(default=None)
    stop_response: Optional[DiagDataRecord] = Field(default=None)
    request_results_response: Optional[DiagDataRecord] = Field(default=None)
    access_profile: Optional[str] = Field(default=None)
    description: Optional[str] = Field(default=None)

    @model_validator(mode="after")
    def validate_data_present_for_supported_sub_functions(self) -> "Routine":
        """
        Raise when a supported sub-function is missing its request data layout.
        """

        if "start" in self.supported_sub_functions and self.start_request is None:
            raise err_major(
                "Routine '{routine_name}' supports 'start' but has no start_request",
                category=Category.REQUIRED,
                error_number="263",
                routine_name=self.name,
            )
        if "stop" in self.supported_sub_functions and self.stop_request is None:
            raise err_major(
                "Routine '{routine_name}' supports 'stop' but has no stop_request",
                category=Category.REQUIRED,
                error_number="264",
                routine_name=self.name,
            )
        if "request_results" in self.supported_sub_functions and self.request_results_response is None:
            raise err_major(
                "Routine '{routine_name}' supports 'request_results' but has no request_results_response",
                category=Category.REQUIRED,
                error_number="265",
                routine_name=self.name,
            )
        return self
