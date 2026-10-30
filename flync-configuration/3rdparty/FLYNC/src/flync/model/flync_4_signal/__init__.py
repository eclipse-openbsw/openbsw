"""Top-level package for flync-4-signal."""

from flync.model.flync_4_signal.forwarder import (
    CANFrameEgress,
    CANFrameForwarder,
    EthSocketEgress,
    ForwarderEgress,
    PDUForwarder,
)
from flync.model.flync_4_signal.frame import (
    CANFDFrame,
    CANFrame,
    CANFrameBase,
    Frame,
    FrameCyclicTiming,
    FrameEventTiming,
    FrameTransmissionTiming,
    LINFrame,
)
from flync.model.flync_4_signal.pdu import (
    PDU,
    ContainedPDURef,
    ContainerPDU,
    ContainerPDUHeader,
    MultiplexedPDU,
    MuxGroup,
    PDUInstance,
    StandardPDU,
)
from flync.model.flync_4_signal.pdu_deployment import (
    PDUReceiver,
    PDUSender,
)
from flync.model.flync_4_signal.signal import (
    BitfieldGroup,
    BitfieldState,
    BitfieldTextTable,
    BitmaskFlag,
    BitmaskFlags,
    InstancePlacement,
    Signal,
    SignalDataType,
    SignalGroup,
    SignalGroupInstance,
    SignalInstance,
    TextEntry,
    TextTable,
    ValueEncoding,
)

KEY = "SIG"
__all__ = [
    # signal
    "SignalDataType",
    "TextEntry",
    "TextTable",
    "BitfieldState",
    "BitfieldGroup",
    "BitfieldTextTable",
    "BitmaskFlag",
    "BitmaskFlags",
    "ValueEncoding",
    "InstancePlacement",
    "Signal",
    "SignalInstance",
    "SignalGroup",
    "SignalGroupInstance",
    # pdu
    "PDU",
    "StandardPDU",
    "MuxGroup",
    "MultiplexedPDU",
    "ContainedPDURef",
    "ContainerPDUHeader",
    "PDUInstance",
    "ContainerPDU",
    # pdu deployment
    "PDUSender",
    "PDUReceiver",
    # frame
    "FrameEventTiming",
    "FrameCyclicTiming",
    "FrameTransmissionTiming",
    "Frame",
    "CANFrameBase",
    "CANFrame",
    "CANFDFrame",
    "LINFrame",
    # forwarder
    "PDUForwarder",
    "CANFrameForwarder",
    "ForwarderEgress",
    "CANFrameEgress",
    "EthSocketEgress",
]
