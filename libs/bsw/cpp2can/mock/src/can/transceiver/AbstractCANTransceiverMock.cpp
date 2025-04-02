// Copyright 2024 Accenture.

#include "can/transceiver/AbstractCANTransceiverMock.h"

#include "can/CanLogger.h"
using namespace ::testing;

namespace can
{
using namespace ::util::logger;

AbstractCANTransceiverMock::AbstractCANTransceiverMock(uint8_t busId)
: AbstractCANTransceiver(busId), _txTimestamp(0)
{
    ON_CALL(*this, init())
        .WillByDefault(Invoke(this, &AbstractCANTransceiverMock::initImplementation));
    ON_CALL(*this, open())
        .WillByDefault(Invoke(this, &AbstractCANTransceiverMock::openImplementation));
    ON_CALL(*this, write(_))
        .WillByDefault(Invoke(this, &AbstractCANTransceiverMock::writeImplementation));
    ON_CALL(*this, write(_, _))
        .WillByDefault(Invoke(this, &AbstractCANTransceiverMock::writeImplementation2));
}

void AbstractCANTransceiverMock::inject(CANFrame const& frame)
{
    Logger::debug(
        CAN,
        "AbstractCANTransceiverMock::inject(0x%x (%d): <0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, "
        "0x%x>)",
        frame.getId(),
        frame.getPayloadLength(),
        frame.getPayload()[0],
        frame.getPayload()[1],
        frame.getPayload()[2],
        frame.getPayload()[3],
        frame.getPayload()[4],
        frame.getPayload()[5],
        frame.getPayload()[6],
        frame.getPayload()[7]);
    AbstractCANTransceiver::notifyListeners(frame);
}

void AbstractCANTransceiverMock::setTransceiverState2(
    can::ICANTransceiverStateListener::CANTransceiverState /* state */)
{
    notifyStateListenerWithPhyError();
}

ICanTransceiver::State AbstractCANTransceiverMock::getStateImplementation() const
{
    return ICanTransceiver::State::CLOSED;
}

ICanTransceiver::ErrorCode AbstractCANTransceiverMock::initImplementation()
{
    return ICanTransceiver::ErrorCode::CAN_ERR_OK;
}

ICanTransceiver::ErrorCode AbstractCANTransceiverMock::openImplementation()
{
    if (false == isInState(State::OPEN))
    {
        setState(State::OPEN);
        notifyStateListenerWithState(ICANTransceiverStateListener::CANTransceiverState::ACTIVE);
    }
    return ICanTransceiver::ErrorCode::CAN_ERR_OK;
}

ICanTransceiver::ErrorCode AbstractCANTransceiverMock::writeImplementation(CANFrame const& frame)
{
    Logger::debug(
        CAN,
        "AbstractCANTransceiverMock::write(0x%x (%d): <0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, "
        "0x%x>)",
        frame.getId(),
        frame.getPayloadLength(),
        frame.getPayload()[0],
        frame.getPayload()[1],
        frame.getPayload()[2],
        frame.getPayload()[3],
        frame.getPayload()[4],
        frame.getPayload()[5],
        frame.getPayload()[6],
        frame.getPayload()[7]);
    return ICanTransceiver::ErrorCode::CAN_ERR_OK;
}

ICanTransceiver::ErrorCode AbstractCANTransceiverMock::writeImplementation2(
    CANFrame const& frame, ICANFrameSentListener& listener)
{
    Logger::debug(
        CAN,
        "AbstractCANTransceiverMock::write(0x%x (%d): <0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, "
        "0x%x>, 0x%p)",
        frame.getId(),
        frame.getPayloadLength(),
        frame.getPayload()[0],
        frame.getPayload()[1],
        frame.getPayload()[2],
        frame.getPayload()[3],
        frame.getPayload()[4],
        frame.getPayload()[5],
        frame.getPayload()[6],
        frame.getPayload()[7],
        &listener);
    BufferedCANFrame txFrame(frame);
    txFrame.setTimestamp(_txTimestamp);
    listener.canFrameSent(txFrame);
    return ICanTransceiver::ErrorCode::CAN_ERR_OK;
}

void AbstractCANTransceiverMock::setTxTimestampForNextWrite(uint32_t timestamp)
{
    _txTimestamp = timestamp;
}

} // namespace can
