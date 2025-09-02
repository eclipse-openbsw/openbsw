// Copyright 2025 Accenture.

#pragma once

#include "ethernet/RxBuffers.h"
#include "ethernet/TxBuffers.h"
#include "etl/array.h"

namespace ethernet
{

class EnetDriver
{
public:
    EnetDriver(EnetDriver const&) = delete;

    struct Configuration
    {
        uint32_t miiConfig;

        struct Rx
        {
            uint16_t RX_BUFFER_SIZE;
            ::etl::span<ENET_ERXD> fRxDescriptor;
            ::etl::span<::lwiputils::RxCustomPbuf> fRxPbufs;
            ::etl::span<pbuf*> sfPbufToRxDescriptorIndexMapping;
            ::etl::span<uint8_t> rxBuffers;
        } rx;

        struct Tx
        {
            ::etl::span<ENET_ETXD> fTxDescriptor;
            ::etl::span<pbuf*> referencedPbufs;
            ::etl::span<uint8_t> descriptorIndices;
            ::etl::span<DataSentTuple> txIsrListeners;
        } tx;
    };

    explicit EnetDriver(::etl::array<uint8_t, 6> macAddr, Configuration const& configuration);

    uint8_t init();

    uint8_t start();
    uint8_t stop();

    void interruptError();
    void interruptGroup0();
    void interruptGroup1();
    void interruptGroup2();
    void interruptGroup3();

    void setGroupcastAddressRecognition(::etl::array<uint8_t, 6> const mac) const;

    void enableVlanTagging();
    void disableVlanTagging();

    void interruptRx();

    bool writeFrame(netif* const /* aNetif */, pbuf const* const buf)
    {
        return _txBuffers.writeFrame(0, buf);
    }

    bool ethernetWrite(::etl::span<uint8_t const> data, DataSentCallback const onSent)
    {
        return _txBuffers.ethernetWrite(data, onSent);
    }

    ::etl::array<uint8_t, 6> _macAddr;
    RxBuffers _rxBuffers;
    TxBuffers _txBuffers;
    uint32_t const _miiConfig;
};

} /* namespace ethernet */
