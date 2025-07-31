// Copyright 2025 Accenture.

#include "ethernet/TxBuffers.h"

#include "bsp/timer/SystemTimer.h"
#include "ethernet/EthernetLogger.h"
#include "etl/array.h"
#include "interrupts/SuspendResumeAllInterruptsScopedLock.h"
#include "mcu/mcu.h"
#include "util/estd/big_endian.h"

#include <cstring>

using namespace ::util::logger;

namespace ethernet
{
uint8_t const LENGTH_VLAN_TAG   = 4U;
uint16_t const MAX_FRAME_LENGTH = 1518U + LENGTH_VLAN_TAG;

void TxBuffers::freeDescriptor(uint8_t const n)
{
    _descriptorIndex = (_descriptorIndex + _descriptors.size() - n) % _descriptors.size();
    _usedCount -= n;
}

void TxBuffers::init()
{
    // initialize transmit buffer descriptors
    for (uint8_t i = 0U; i < _descriptors.size(); i++)
    {
        _referencedPbufs[i]     = 0L;
        _descriptors[i].status1 = ENET_ETXD_STATUS1_TO2(1);
        _descriptors[i].length  = 0U;
        _descriptors[i].status3 = _descriptors[i].status3 | ENET_ETXD_STATUS3_INT(1)
                                  | ENET_ETXD_STATUS3_PINS(1) | ENET_ETXD_STATUS3_IINS(1);
        _txIsrListeners[i].data   = ::etl::span<uint8_t const>();
        _txIsrListeners[i].onSent = DataSentCallback();
        _txIsrListeners[i].next   = 0L;
    }

    // set the wrap bit on the last buffers
    _descriptors[_descriptors.size() - 1].status1
        = _descriptors[_descriptors.size() - 1].status1 | ENET_ETXD_STATUS1_W(1);
}

/**
 * Commit all the frame's buffers to be sent after preparing them with
 * writeBuffer().
 * \param n         Number of buffers that compose a frame.
 * \param indexes   Array of buffer descriptor indexes.
 * \return          true, if buffers were committed, false otherwise.
 */
bool commitFrame(
    uint8_t const n, uint8_t const* const indexes, ::etl::span<ENET_ETXD> const txDescriptor)
{
    if (n > 0U)
    {
        // set RDY bit of the first BD in frame last
        for (uint8_t i = n; i > 0U; --i)
        {
            txDescriptor[indexes[i - 1]].status4
                = txDescriptor[indexes[i - 1]].status4 & ~ENET_ETXD_STATUS4_BDU(1);
            txDescriptor[indexes[i - 1]].status1
                = txDescriptor[indexes[i - 1]].status1 | ENET_ETXD_STATUS1_R(1);
        }

        // initiate transmission
        IP_ENET->TDAR = IP_ENET->TDAR | ENET_TDAR_TDAR_MASK;
        return true;
    }
    return false;
}

bool TxBuffers::ethernetWrite(::etl::span<uint8_t const> const data, DataSentCallback const onSent)
{
    ::interrupts::SuspendResumeAllInterruptsScopedLock const lock;

    ::etl::array<uint8_t, 1> txDescriptorIndex;
    if (!getNextDescriptorIndex(1U, txDescriptorIndex))
    {
        return false;
    }

    if (writeBuffer(txDescriptorIndex[0], data, true))
    {
        _txIsrListeners[txDescriptorIndex[0]].data   = data;
        _txIsrListeners[txDescriptorIndex[0]].onSent = onSent;
        return commitFrame(1U, &txDescriptorIndex[0], _descriptors);
    }
    return false;
}

bool TxBuffers::writeBuffer(
    uint8_t const txDescriptorIndex,
    ::etl::span<uint8_t const> const data,
    bool const lastBufferInFrame)
{
    // sanity checks
    estd_assert(txDescriptorIndex < _descriptors.size());
    estd_assert(0U != (_descriptors[txDescriptorIndex].status1 & ENET_ETXD_STATUS1_TO2(1)));

    if (data.size() > MAX_FRAME_LENGTH)
    {
        Logger::log(ETHERNET, _ERROR, "Too much transmit-data: %d", data.size());
        return false;
    }

    _descriptors[txDescriptorIndex].data   = const_cast<uint8_t*>(data.data());
    _descriptors[txDescriptorIndex].length = data.size();
    _descriptors[txDescriptorIndex].status1
        = (_descriptors[txDescriptorIndex].status1 & ENET_ETXD_STATUS1_W(1))
          | ENET_ETXD_STATUS1_TC(1);

    if (lastBufferInFrame)
    {
        _descriptors[txDescriptorIndex].status1
            = _descriptors[txDescriptorIndex].status1 | ENET_ETXD_STATUS1_L(1);
    }

    return true;
}

bool TxBuffers::writeFrame(uint16_t const vlanId, const struct pbuf* const buf)
{
    struct pbuf* const p = const_cast<struct pbuf*>(buf);

    auto const txBufferDescriptorCount = pbuf_clen(p);

    if (0U == txBufferDescriptorCount)
    {
        return true;
    }

    if (txBufferDescriptorCount > _descriptors.size())
    {
        return false;
    }

    ::interrupts::SuspendResumeAllInterruptsScopedLock const lock;
    uint8_t bufferIndex = 0U;

    bool ok = getNextDescriptorIndex(txBufferDescriptorCount, _descriptorIndices);

    if (ok)
    {
        struct pbuf* nextPbuf = p;
        do
        {
            uint8_t* payload = nullptr;
            size_t length    = 0;

            if (nextPbuf != 0L)
            {
                payload  = reinterpret_cast<uint8_t*>(nextPbuf->payload);
                length   = nextPbuf->len;
                nextPbuf = nextPbuf->next;
            }

            if (!payload)
            {
                ok = false;
                break;
            }

            if (0U == bufferIndex)
            {
                // Copy the original ETYPE
                uint16_t const etherType
                    = *reinterpret_cast<::estd::be_uint16_t*>(payload + 6U * 2);
                uint8_t* header  = payload;
                uint8_t writeLen = 0U;

                if (_enableVlanTagging) // VLAN enabled
                {
                    // Space needed for the VLAN_TAG
                    // TODO: check if this is correct
                    header -= LENGTH_VLAN_TAG;
                }

                uint8_t* insert = header;
                if (payload != header)
                {
                    (void)memmove(header, payload, 6U * 2); // Copy MACS
                    insert += 6U * 2;                       // Move pointer by 12
                }

                if (vlanId != 0)
                {
                    estd_assert(false);
                }

                if (insert != header)
                {
                    *reinterpret_cast<::estd::be_uint16_t*>(insert)
                        = etherType; // Insert the original ETYPE
                }

                payload = header;
                length += writeLen;
            }

            if (ok)
            {
                if ((!writeBuffer(
                        _descriptorIndices[bufferIndex++],
                        ::etl::span<uint8_t const>(payload, length),
                        nextPbuf ? false : true)))
                {
                    ok = false;
                    freeDescriptor(txBufferDescriptorCount);
                }
            }
        } while (nextPbuf && ok);
    }
    else // getNextTxDescriptorIndex() failed
    {
        ok = false;
    }

    if (ok)
    {
        estd_assert(bufferIndex == txBufferDescriptorCount);
        estd_assert(txBufferDescriptorCount > 0U);

        pbuf_ref(p);
        _referencedPbufs[_descriptorIndices[txBufferDescriptorCount - 1U]] = p;

        if (commitFrame(txBufferDescriptorCount, &_descriptorIndices[0], _descriptors))
        {
            return true;
        }

        (void)pbuf_free(p);
    }

    static bool once = false;
    if (!once)
    {
        once = true;
        Logger::log(ETHERNET, _ERROR, "ENET tx fifo is full");
    }
    return false;
}

bool TxBuffers::getNextDescriptorIndex(uint8_t const n, ::etl::span<uint8_t> const indexes)
{
    if ((_usedCount + n) > _descriptors.size())
    {
        return false;
    }
    for (uint8_t i = 0U; i < n; ++i)
    {
        uint8_t const nextIdx = (_descriptorIndex + i) % _descriptors.size();
        if (_descriptors[nextIdx].status1 & ENET_ETXD_STATUS1_TO2(1))
        {
            indexes[i] = nextIdx;
        }
        else
        {
            return false;
        }
    }
    // advance descriptor index
    _usedCount += n;
    _descriptorIndex = (_descriptorIndex + n) % _descriptors.size();
    return true;
}

void TxBuffers::interrupt()
{
    uint64_t const timestamp = getSystemTimeNs();
    while ((IP_ENET->EIR & ENET_EIR_TXF_MASK) != 0U)
    {
        // transmit frame interrupt occurred
        // clear interrupt flag
        IP_ENET->EIR              = ENET_EIR_TXF_MASK;
        DataSentTuple* sentTuples = 0L;
        for (size_t i = _usedCount; i > 0U; --i)
        {
            auto const txDescriptorIndex
                = (_descriptorIndex + _descriptors.size() - i) % _descriptors.size();

            if (0U == (_descriptors[txDescriptorIndex].status1 & ENET_ETXD_STATUS1_R(1)))
            {
                _descriptors[txDescriptorIndex].status1
                    = _descriptors[txDescriptorIndex].status1 | ENET_ETXD_STATUS1_TO2(1U);
                DataSentTuple* const pSentTuple = &_txIsrListeners[txDescriptorIndex];
                if (pSentTuple->onSent)
                {
                    pSentTuple->next = sentTuples;
                    sentTuples       = pSentTuple;
                }
                if (_referencedPbufs[txDescriptorIndex] != 0L)
                {
                    (void)pbuf_free(_referencedPbufs[txDescriptorIndex]);
                    _referencedPbufs[txDescriptorIndex] = 0L;
                }
                --_usedCount;
            }
            else
            {
                break;
            }
        }
        while (sentTuples != 0L)
        {
            DataSentTuple* const next = sentTuples->next;
            if (sentTuples->onSent)
            {
                sentTuples->onSent(
                    sentTuples->data, ::etl::chrono::nanoseconds(static_cast<int64_t>(timestamp)));
                sentTuples->data   = ::etl::span<uint8_t const>();
                sentTuples->onSent = DataSentCallback();
                sentTuples->next   = 0L;
            }
            sentTuples = next;
        }
    }
}

} // namespace ethernet
