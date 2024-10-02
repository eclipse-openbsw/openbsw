// Copyright 2024 Accenture.

#ifndef GUARD_54E2611F_03DE_45CB_877B_FA76D872944C
#define GUARD_54E2611F_03DE_45CB_877B_FA76D872944C

#include "docan/common/DoCanConstants.h"

#include <platform/estdint.h>

namespace docan
{
template<class DataLinkLayer>
class DoCanFrameCodec;

/**
 * Interface for emitting flow control frames.
 * \tparam DataLinkLayer class providing data link functionality
 */
template<class DataLinkLayer>
class IDoCanFlowControlFrameTransmitter
{
public:
    using DataLinkLayerType   = DataLinkLayer;
    using FrameCodecType      = DoCanFrameCodec<DataLinkLayerType>;
    using DataLinkAddressType = typename DataLinkLayerType::AddressType;

    /**
     * Called to send out a flow control frame.
     * \param codec codec used to encode the flow control
     * \param transmissionAddress address of destination for transmission
     * \param flowStatus flow status byte
     * \param blockSize block size byte (valid for flow status CTS)
     * \param encodedMinSeparationTime encoded min separation time
     * \return true if flow control has been sent successfully, false otherwise
     */
    virtual bool sendFlowControl(
        FrameCodecType const& codec,
        DataLinkAddressType transmissionAddress,
        FlowStatus flowStatus,
        uint8_t blockSize,
        uint8_t encodedMinSeparationTime)
        = 0;

private:
    IDoCanFlowControlFrameTransmitter& operator=(IDoCanFlowControlFrameTransmitter const&) = delete;
};

} // namespace docan

#endif // GUARD_54E2611F_03DE_45CB_877B_FA76D872944C
