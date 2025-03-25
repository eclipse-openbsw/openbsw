// Copyright 2024 Accenture.

#pragma once

#include "docan/datalink/IDoCanDataFrameTransmitter.h"
#include "docan/datalink/IDoCanFlowControlFrameTransmitter.h"
#include "docan/datalink/IDoCanFrameReceiver.h"

namespace docan
{
/**
 * Interface for an abstract frame transceiver.
 * \tparam DataLinkLayer class providing data link functionality
 */
template<class DataLinkLayer>
class IDoCanPhysicalTransceiver
: public IDoCanDataFrameTransmitter<DataLinkLayer>
, public IDoCanFlowControlFrameTransmitter<DataLinkLayer>
{
public:
    /**
     * Initialize.
     * \param receiver reference to the frame receiver interface
     */
    virtual void init(IDoCanFrameReceiver<DataLinkLayer>& receiver) = 0;

    /**
     * Shutdown.
     */
    virtual void shutdown() = 0;

private:
    IDoCanPhysicalTransceiver& operator=(IDoCanPhysicalTransceiver const&) = delete;
};

} // namespace docan

