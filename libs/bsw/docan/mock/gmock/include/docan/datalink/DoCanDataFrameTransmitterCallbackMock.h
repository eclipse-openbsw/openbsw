// Copyright 2024 Accenture.

#pragma once

#include "docan/datalink/IDoCanDataFrameTransmitterCallback.h"

#include <gmock/gmock.h>

namespace docan
{
/**
 * Interface for DoCan abstraction of data frame transmitter.
 * \tparam DataLinkLayer class providing data link functionality
 */
template<class DataLinkLayer>
class DoCanDataFrameTransmitterCallbackMock
: public IDoCanDataFrameTransmitterCallback<DataLinkLayer>
{
public:
    using DataLinkLayerType = DataLinkLayer;
    using MessageSizeType   = typename DataLinkLayerType::MessageSizeType;
    using FrameIndexType    = typename DataLinkLayerType::FrameIndexType;
    using JobHandleType     = typename DataLinkLayerType::JobHandleType;

    MOCK_METHOD(
        void,
        dataFramesSent,
        (JobHandleType jobHandle, FrameIndexType frameCount, MessageSizeType dataSize),
        (override));
};

} // namespace docan
