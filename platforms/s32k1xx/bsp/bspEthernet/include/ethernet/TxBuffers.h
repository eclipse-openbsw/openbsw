// Copyright 2025 Accenture.

#pragma once

#include "ethernet/EnetDevice.h"
#include "ethernet/RawEthernet.h"
#include "etl/span.h"

extern "C"
{
#include "lwip/pbuf.h"
}

namespace ethernet
{

struct DataSentTuple
{
    DataSentCallback onSent;
    ::etl::span<uint8_t const> data;
    DataSentTuple* next;
};

class TxBuffers
{
public:
    TxBuffers(
        ::etl::span<pbuf*> const referencedPbufs,
        ::etl::span<ENET_ETXD> const descriptors,
        ::etl::span<DataSentTuple> const txIsrListeners,
        ::etl::span<uint8_t> const txDescriptorIndexes)
    : _referencedPbufs(referencedPbufs)
    , _descriptors(descriptors)
    , _txIsrListeners(txIsrListeners)
    , _descriptorIndices(txDescriptorIndexes)
    , _usedCount(0U)
    , _descriptorIndex(0U)
    {}

    void init();

    /**
     * Write buffer (descriptor), neither copy data nor send.
     * To send the frame, call commitFrame() after one or more calls to
     * writeBuffer().
     * \section Synchronization
     * The call(s) to writeBuffer() and commitFrame() must not be
     * interrupted; so protect them with e.g. ScopedCriticalSection.
     *
     * \param txDescriptorIndex Index of the buffer descriptor to write to.
     * \param length            Transmit data size.
     * \param pData             Pointer to transmit data.
     * \param lastBufferInFrame Set to true on the last buffer in the frame.
     * \return false in case of error, true on success.
     */
    bool
    writeBuffer(uint8_t txDescriptorIndex, ::etl::span<uint8_t const> data, bool lastBufferInFrame);

    bool writeFrame(uint16_t vlanId, const struct pbuf* buf);

    bool ethernetWrite(::etl::span<uint8_t const> data, DataSentCallback onSent);

    void interrupt();

    uint32_t descriptorAddress() const { return reinterpret_cast<uint32_t>(_descriptors.data()); }

    bool _enableVlanTagging;

private:
    bool getNextDescriptorIndex(uint8_t n, ::etl::span<uint8_t> indexes);
    void freeDescriptor(uint8_t n);

    ::etl::span<pbuf*> const _referencedPbufs;
    ::etl::span<ENET_ETXD> const _descriptors;
    ::etl::span<DataSentTuple> const _txIsrListeners;
    ::etl::span<uint8_t> const _descriptorIndices;

    uint8_t _usedCount;       // number of used Tx buffer descriptors
    uint8_t _descriptorIndex; // index of empty Tx buffer descriptor
};

} // namespace ethernet
