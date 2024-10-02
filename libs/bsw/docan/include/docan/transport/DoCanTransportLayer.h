// Copyright 2024 Accenture.

#ifndef GUARD_018D552C_ED76_416E_8939_86A6A059F680
#define GUARD_018D552C_ED76_416E_8939_86A6A059F680
#include "docan/addressing/IDoCanAddressConverter.h"
#include "docan/common/DoCanConnection.h"
#include "docan/common/DoCanConstants.h"
#include "docan/datalink/IDoCanPhysicalTransceiver.h"
#include "docan/receiver/DoCanReceiver.h"
#include "docan/transmitter/DoCanTransmitter.h"
#include "docan/transport/DoCanTransportLayerConfig.h"

#include <async/Async.h>
#include <async/Types.h>
#include <async/util/MemberCall.h>

#include <estd/functional.h>

namespace docan
{
/**
 * DoCAN CAN transport layer.
 * \tparam DataLinkLayer class providing data link functionality
 */
template<class DataLinkLayer>
class DoCanTransportLayer final
: public ::transport::AbstractTransportLayer
, public IDoCanFrameReceiver<DataLinkLayer>
{
public:
    using DataLinkLayerType       = DataLinkLayer;
    using DataLinkAddressType     = typename DataLinkLayerType::AddressType;
    using DataLinkAddressPairType = typename DataLinkLayerType::AddressPairType;
    using MessageSizeType         = typename DataLinkLayerType::MessageSizeType;
    using FrameIndexType          = typename DataLinkLayerType::FrameIndexType;
    using FrameSizeType           = typename DataLinkLayerType::FrameSizeType;
    using ConnectionType          = DoCanConnection<DataLinkLayerType>;

    /** Constructor.
     *
     * \param busId bus identifier of transport layer
     *
     * \param filter reference to the filter. The filter is used as a reception filter for the CAN
     * transceiver and maps physical addresses (depending on addressing format) to transport
     * addresses and back
     *
     * \param transceiver reference to the CAN transceiver for sending and receiving CAN frames
     *
     * \param tickGenerator reference to the tick generator that provides a callback to request
     * ticks
     *
     * \param config reference to configuration that holds parameters and pools
     *
     * \param loggerComponent reference to logger component to use
     */
    DoCanTransportLayer(
        uint8_t busId,
        ::async::ContextType context,
        IDoCanAddressConverter<DataLinkLayerType>& addressConverter,
        IDoCanPhysicalTransceiver<DataLinkLayerType>& transceiver,
        IDoCanTickGenerator& tickGenerator,
        DoCanTransportLayerConfig<DataLinkLayerType>& config,
        uint8_t const loggerComponent);

    ErrorCode init() override;
    bool shutdown(ShutdownDelegate delegate) override;
    ErrorCode send(
        ::transport::TransportMessage& transportMessage,
        ::transport::ITransportMessageProcessedListener* pNotificationListener) override;

    void cyclicTask(uint32_t nowUs);
    bool tick(uint32_t nowUs);

private:
    void processShutdown();

    void firstDataFrameReceived(
        ConnectionType const& connection,
        MessageSizeType messageSize,
        FrameIndexType frameCount,
        FrameSizeType consecutiveFrameDataSize,
        ::estd::slice<uint8_t const> const& data) override;
    void consecutiveDataFrameReceived(
        DataLinkAddressType receptionAddress,
        uint8_t sequenceNumber,
        ::estd::slice<uint8_t const> const& data) override;
    void flowControlFrameReceived(
        DataLinkAddressType receptionAddress,
        FlowStatus flowStatus,
        uint8_t blockSize,
        uint8_t encodedMinSeparationTime) override;

    void shutdownDone();

    IDoCanPhysicalTransceiver<DataLinkLayerType>& _transceiver;
    DoCanReceiver<DataLinkLayerType> _receiver;
    DoCanTransmitter<DataLinkLayerType> _transmitter;
    ::async::MemberCall<DoCanTransportLayer, &DoCanTransportLayer::processShutdown>
        _processShutdown;
    ShutdownDelegate _shutdownDelegate;
    ::async::ContextType const _context;
};

/**
 * Inline implementation.
 */
template<class DataLinkLayer>
DoCanTransportLayer<DataLinkLayer>::DoCanTransportLayer(
    uint8_t const busId,
    ::async::ContextType const context,
    IDoCanAddressConverter<DataLinkLayerType>& addressConverter,
    IDoCanPhysicalTransceiver<DataLinkLayerType>& transceiver,
    IDoCanTickGenerator& tickGenerator,
    DoCanTransportLayerConfig<DataLinkLayerType>& config,
    uint8_t const loggerComponent)
: AbstractTransportLayer(busId)
, _transceiver(transceiver)
, _receiver(
      busId,
      context,
      getProvidingListenerHelper(),
      _transceiver,
      config.getMessageReceiverPool(),
      addressConverter,
      config.getParameters(),
      loggerComponent)
, _transmitter(
      busId,
      context,
      _transceiver,
      tickGenerator,
      config.getMessageTransmitterPool(),
      addressConverter,
      config.getParameters(),
      loggerComponent)
, _processShutdown(*this)
, _shutdownDelegate()
, _context(context)
{}

template<class DataLinkLayer>
::transport::AbstractTransportLayer::ErrorCode DoCanTransportLayer<DataLinkLayer>::init()
{
    _transceiver.init(*this);
    _transmitter.init();
    _receiver.init();
    return ErrorCode::TP_OK;
}

template<class DataLinkLayer>
bool DoCanTransportLayer<DataLinkLayer>::shutdown(ShutdownDelegate const delegate)
{
    _shutdownDelegate = delegate;
    ::async::execute(_context, _processShutdown);
    return false;
}

template<class DataLinkLayer>
::transport::AbstractTransportLayer::ErrorCode DoCanTransportLayer<DataLinkLayer>::send(
    ::transport::TransportMessage& transportMessage,
    ::transport::ITransportMessageProcessedListener* const pNotificationListener)
{
    return _transmitter.send(transportMessage, pNotificationListener);
}

template<class DataLinkLayer>
void DoCanTransportLayer<DataLinkLayer>::cyclicTask(uint32_t const nowUs)
{
    (void)_transmitter.cyclicTask(nowUs);
    _receiver.cyclicTask(nowUs);
}

template<class DataLinkLayer>
bool DoCanTransportLayer<DataLinkLayer>::tick(uint32_t const nowUs)
{
    _transmitter.cyclicTask(nowUs);
    return _transmitter.isSendingConsecutiveFrames();
}

template<class DataLinkLayer>
void DoCanTransportLayer<DataLinkLayer>::firstDataFrameReceived(
    ConnectionType const& connection,
    MessageSizeType messageSize,
    FrameIndexType frameCount,
    FrameSizeType consecutiveFrameDataSize,
    ::estd::slice<uint8_t const> const& data)
{
    _receiver.firstDataFrameReceived(
        connection, messageSize, frameCount, consecutiveFrameDataSize, data);
}

template<class DataLinkLayer>
void DoCanTransportLayer<DataLinkLayer>::consecutiveDataFrameReceived(
    DataLinkAddressType receptionAddress,
    uint8_t sequenceNumber,
    ::estd::slice<uint8_t const> const& data)
{
    _receiver.consecutiveDataFrameReceived(receptionAddress, sequenceNumber, data);
}

template<class DataLinkLayer>
void DoCanTransportLayer<DataLinkLayer>::flowControlFrameReceived(
    DataLinkAddressType receptionAddress,
    FlowStatus flowStatus,
    uint8_t blockSize,
    uint8_t encodedMinSeparationTime)
{
    _transmitter.flowControlFrameReceived(
        receptionAddress, flowStatus, blockSize, encodedMinSeparationTime);
}

template<class DataLinkLayer>
void DoCanTransportLayer<DataLinkLayer>::processShutdown()
{
    _transmitter.shutdown();
    _receiver.shutdown();
    _transceiver.shutdown();
    _shutdownDelegate(*this);
}
} // namespace docan

#endif // GUARD_018D552C_ED76_416E_8939_86A6A059F680
