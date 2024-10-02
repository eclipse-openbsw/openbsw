// Copyright 2024 Accenture.

#ifndef GUARD_60B6D233_94B4_49E4_9F75_D9416948C6F6
#define GUARD_60B6D233_94B4_49E4_9F75_D9416948C6F6

#include <estd/slice.h>

namespace docan
{
/**
 * Interface for DoCan abstraction of data frame transmitter.
 * \tparam DataLinkLayer class providing data link functionality
 */
template<class DataLinkLayer>
class IDoCanDataFrameTransmitterCallback
{
public:
    using DataLinkLayerType = DataLinkLayer;
    using MessageSizeType   = typename DataLinkLayerType::MessageSizeType;
    using FrameIndexType    = typename DataLinkLayerType::FrameIndexType;
    using JobHandleType     = typename DataLinkLayerType::JobHandleType;

    /**
     * Called to indicate the result of sending one or more data frames.
     * \param jobHandle handle identifying the send job
     * \param frameCount number of frames that have been sent successfully
     * \param dataSize number of corresponding data bytes that have successfully been sent
     */
    virtual void
    dataFramesSent(JobHandleType jobHandle, FrameIndexType frameCount, MessageSizeType dataSize)
        = 0;

private:
    IDoCanDataFrameTransmitterCallback& operator=(IDoCanDataFrameTransmitterCallback const&)
        = delete;
};

} // namespace docan

#endif // DOCAN_IDOCANFRAMETRANSMITTERCALLBACK_H_
