// Copyright 2024 Accenture.

#ifndef GUARD_34BF5374_F12E_4ACB_80B6_C05391C098AC
#define GUARD_34BF5374_F12E_4ACB_80B6_C05391C098AC

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

    MOCK_METHOD3_T(
        dataFramesSent,
        void(JobHandleType jobHandle, FrameIndexType frameCount, MessageSizeType dataSize));
};

} // namespace docan

#endif // GUARD_34BF5374_F12E_4ACB_80B6_C05391C098AC
