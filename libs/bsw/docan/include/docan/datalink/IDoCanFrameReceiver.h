// Copyright 2024 Accenture.

#ifndef GUARD_6B3BE423_91E5_4515_952D_EB4D4BE4F1E0
#define GUARD_6B3BE423_91E5_4515_952D_EB4D4BE4F1E0

#include "docan/common/DoCanConstants.h"

#include <estd/slice.h>

namespace docan
{
template<class DataLinkLayer>
class DoCanConnection;

/**
 * Callback interface for DoCan abstraction of frame receiver.
 * \tparam DataLinkLayer class providing data link functionality
 */
template<class DataLinkLayer>
class IDoCanFrameReceiver
{
public:
    using DataLinkLayerType       = DataLinkLayer;
    using ConnectionType          = DoCanConnection<DataLinkLayerType>;
    using DataLinkAddressType     = typename DataLinkLayerType::AddressType;
    using DataLinkAddressPairType = typename DataLinkLayerType::AddressPairType;
    using MessageSizeType         = typename DataLinkLayerType::MessageSizeType;
    using FrameIndexType          = typename DataLinkLayerType::FrameIndexType;
    using FrameSizeType           = typename DataLinkLayerType::FrameSizeType;

    /**
     * called on reception of a first data frame (either single or first segmented).
     * \param connection incoming connection parameters
     * \param messageSize expected message size
     * \param frameCount number of total frames to receive (1 => single frame)
     * \param consecutiveFrameDataSize data size of a regular consecutive frame
     * \param data reference to the data of the frame
     */
    virtual void firstDataFrameReceived(
        ConnectionType const& connection,
        MessageSizeType messageSize,
        FrameIndexType frameCount,
        FrameSizeType consecutiveFrameDataSize,
        ::estd::slice<uint8_t const> const& data)
        = 0;

    /**
     * called on reception of a consecutive data frame.
     * \param receptionAddress reception address of the frame
     * \param sequenceNumber sequence number as received from transport layer
     * \param frame reference to frame containing the data
     */
    virtual void consecutiveDataFrameReceived(
        DataLinkAddressType receptionAddress,
        uint8_t sequenceNumber,
        ::estd::slice<uint8_t const> const& data)
        = 0;

    /**
     * called on reception of a flow control frame.
     * \param receptionAddress reception address of the frame
     * \param flowStatus flow status as defined
     * \param blockSize block size
     * \param encodedMinSeparationTime encoded separation time
     */
    virtual void flowControlFrameReceived(
        DataLinkAddressType receptionAddress,
        FlowStatus flowStatus,
        uint8_t blockSize,
        uint8_t encodedMinSeparationTime)
        = 0;

private:
    IDoCanFrameReceiver& operator=(IDoCanFrameReceiver const&) = delete;
};

} // namespace docan

#endif // GUARD_6B3BE423_91E5_4515_952D_EB4D4BE4F1E0
