// Copyright 2024 Accenture.

#pragma once

#include "docan/datalink/IDoCanDataFrameTransmitterCallback.h"

#include <estd/slice.h>

namespace docan
{
template<class DataLinkLayer>
class DoCanFrameCodec;

/**
 * Enumeration for result of sending.
 */
enum class SendResult : uint8_t
{ /// The send job has been queued, more send jobs are available
    QUEUED,
    /// The send job has been queued, no more send job is available
    QUEUED_FULL,
    /// The send job couldn't be queued, so it should be queued outside and sending retried
    FULL,
    /// The sent data cannot is invalid
    INVALID,
    /// Sending has failed due to another reason that makes it reasonable to not retry
    FAILED
};

/**
 * Interface for DoCan abstraction of frame transmitter.
 * \tparam DataLinkLayer class providing data link functionality
 */
template<class DataLinkLayer>
class IDoCanDataFrameTransmitter
{
public:
    using DataLinkLayerType   = DataLinkLayer;
    using FrameCodecType      = DoCanFrameCodec<DataLinkLayerType>;
    using DataLinkAddressType = typename DataLinkLayerType::AddressType;
    using MessageSizeType     = typename DataLinkLayerType::MessageSizeType;
    using FrameIndexType      = typename DataLinkLayerType::FrameIndexType;
    using FrameSizeType       = typename DataLinkLayerType::FrameSizeType;
    using JobHandleType       = typename DataLinkLayerType::JobHandleType;

    /**
     * Called to place a send job of one or more data frames.
     * \param codec codec used to encode data frames
     * \param callback reference to callback for which dataFramesSent will be called
     * \param jobHandle handle identifying this send job for the sender
     * \param transmissionAddress address of destination for transmission
     * \param firstFrameIndex zero-based index of first frame to send
     * \param lastFrameIndex zero-based index of frame to stop sending at
     * \param consecutiveFrameDataSize maximum data size of consecutive frames
     * \param data reference to data that is to be sent
     * \return result indicating success of sending
     */
    virtual SendResult startSendDataFrames(
        FrameCodecType const& codec,
        IDoCanDataFrameTransmitterCallback<DataLinkLayer>& callback,
        JobHandleType jobHandle,
        DataLinkAddressType transmissionAddress,
        FrameIndexType firstFrameIndex,
        FrameIndexType lastFrameIndex,
        FrameSizeType consecutiveFrameDataSize,
        ::estd::slice<uint8_t const> const& data)
        = 0;

    /**
     * Called to cancel a send job.
     * \param callback reference to callback that started the job
     * \param jobHandle handle identifying the send job to cancel
     * \param transmissionAddress address of destination for transmission, identifying the job to
     * cancel.
     */
    virtual void cancelSendDataFrames(
        IDoCanDataFrameTransmitterCallback<DataLinkLayer>& callback, JobHandleType jobHandle)
        = 0;

private:
    IDoCanDataFrameTransmitter& operator=(IDoCanDataFrameTransmitter const&) = delete;
};

} // namespace docan

