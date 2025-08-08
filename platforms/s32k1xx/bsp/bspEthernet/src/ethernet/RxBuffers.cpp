// Copyright 2025 Accenture.

#include "ethernet/RxBuffers.h"

#include "ethernet/EthernetLogger.h"
#include "interrupts/SuspendResumeAllInterruptsScopedLock.h"
#include "mcu/mcu.h"

using namespace ::util::logger;

namespace ethernet
{

uint8_t freeRxDescriptorIndex(
    size_t const descriptorIndex, uint8_t const nextBusy, ::etl::span<pbuf*> const pbufAtIndex)
{
    estd_assert(pbufAtIndex.size() != 0L);

    auto const pbuf1 = reinterpret_cast<::lwiputils::RxCustomPbuf*>(pbufAtIndex[nextBusy]);
    auto const pbuf2 = reinterpret_cast<::lwiputils::RxCustomPbuf*>(pbufAtIndex[descriptorIndex]);

    ::etl::swap(pbufAtIndex[nextBusy], pbufAtIndex[descriptorIndex]);
    ::etl::swap(pbuf1->slot, pbuf2->slot);

    return (nextBusy + 1) % pbufAtIndex.size();
}

void freeCustomPbufHelper(pbuf* const p)
{
    // We can "upcast" here since the initial allocation was made as RxCustimPbuf
    // The pbuf is embedded as the first memeber so the address stays the same.
    auto const customPbuf = reinterpret_cast<::lwiputils::RxCustomPbuf*>(p);
    auto const driver     = reinterpret_cast<RxBuffers*>(customPbuf->driver);
    auto const index
        = reinterpret_cast<ENET_ERXD*>(customPbuf->slot) - driver->_descriptors.begin();

    {
        ::interrupts::SuspendResumeAllInterruptsScopedLock const scopedCriticalSection;

        auto const nextBusy = driver->_nextBusy;
        driver->_descriptors[nextBusy].status1
            = (driver->_descriptors[nextBusy].status1 & ENET_ERXD_STATUS1_WRAP(1))
              | ENET_ERXD_STATUS1_EMPTY(1);

        driver->_nextBusy = freeRxDescriptorIndex(index, nextBusy, driver->_pbufAtIndex);
    }

    IP_ENET->RDAR = IP_ENET->RDAR | ENET_RDAR_RDAR(1);
}

void RxBuffers::init()
{
    // initialize receive buffer descriptors
    estd_assert((RX_BUFFER_SIZE % 4) == 0U);
    for (size_t i = 0U; i < _descriptors.size(); i++)
    {
        _descriptors[i].status1 = ENET_ERXD_STATUS1_EMPTY(1);
        _descriptors[i].status3 = _descriptors[i].status3 | ENET_ERXD_STATUS3_INT(1);
        _descriptors[i].length  = 0U;
        estd_assert((reinterpret_cast<uint32_t>(_descriptors[i].data) % 64) == 0U);

        _descriptors[_descriptors.size() - 1].status1
            = _descriptors[_descriptors.size() - 1].status1 | ENET_ERXD_STATUS1_WRAP(1U);
    }

    for (size_t i = 0U; i < _customPbufs.size(); i++)
    {
        void* const payload = static_cast<void*>(_descriptors[i].data);
        pbuf* const p       = pbuf_alloced_custom(
            PBUF_RAW, RX_BUFFER_SIZE, PBUF_REF, &_customPbufs[i].buf, payload, RX_BUFFER_SIZE);

        estd_assert(p);
        _customPbufs[i].driver                   = this;
        _customPbufs[i].buf.custom_free_function = &freeCustomPbufHelper;
        _customPbufs[i].slot                     = &_descriptors[i];
        _pbufAtIndex[i]                          = p;
    }
}

void RxBuffers::interrupt()
{
    struct pbuf* buf;
    netif* pNetif;

    auto sender = ::lwiputils::PbufQueue::Sender(_queue);
    do
    {
        buf = readFrame(pNetif);
        if (buf != nullptr)
        {
            // frame transfer was successful
            if (sender.full())
            {
                // enqueue failed
                static bool once = false;
                if (!once)
                {
                    once = true;
                    Logger::log(ETHERNET, _WARN, "RX-Queue is full");
                }
                (void)pbuf_free(buf);
            }
            else
            {
                // allocation was successful

                sender.next() = buf;
                sender.write_next();
                // frame has been successfully enqueued
                // TODO: notify TCPIP task by setting event
            }
        }
    } while (buf != nullptr);
}

static bool isErrornousBuffer(ENET_ERXD const& descriptor)
{
    if ((descriptor.status1 & ENET_ERXD_STATUS1_TR(1)) != 0U)
    { // frame truncated, ignore other error-flags
        Logger::log(ETHERNET, _WARN, "Frame truncated");
        return true;
    }

    if ((descriptor.status1 & ENET_ERXD_STATUS1_LAST(1)) == 0U)
    { // other error flags only valid, if LAST-bit is set
        return false;
    }

    static uint16_t const ERROR_FLAGS = static_cast<uint16_t>(ENET_ERXD_STATUS1_OVER(1))
                                        | static_cast<uint16_t>(ENET_ERXD_STATUS1_NO(1))
                                        | static_cast<uint16_t>(ENET_ERXD_STATUS1_LENGTH(1));

    if ((descriptor.status1 & ERROR_FLAGS) != 0U)
    {
        Logger::log(ETHERNET, _WARN, "RX error (0x%x)", descriptor.status1);
        return true;
    }

    bool const protocolDetected = ((descriptor.status4 & ENET_ERXD_STATUS4_PROTTYPE(1)) != 0U);
    bool const checksumError    = ((descriptor.status2 & ENET_ERXD_STATUS2_ICE(1)) != 0U)
                               || ((descriptor.status2 & ENET_ERXD_STATUS2_PCR(1)) != 0U);

    if (protocolDetected && checksumError)
    {
        Logger::log(ETHERNET, _WARN, "RX IP/Proto checksum error");
        return true;
    }

    return false;
}

pbuf* RxBuffers::getCurrentBuffer()
{
    struct pbuf* const pCurrentBuffer = _pbufAtIndex[_nextFree];
    _descriptors[_nextFree].status1   = _descriptors[_nextFree].status1 | ENET_ERXD_STATUS1_RO1(1);
    pCurrentBuffer->payload           = _descriptors[_nextFree].data;
    pCurrentBuffer->next              = 0L;
    pCurrentBuffer->ref               = 0U;

    return pCurrentBuffer;
}

void handleErrornousBuffer(pbuf*& pFrame, pbuf* const pCurrentBuffer)
{
    if (pFrame != 0L)
    {
        pbuf_free(pFrame);
        pFrame = 0L;
    }
    (void)pbuf_free(pCurrentBuffer);
}

pbuf* RxBuffers::readFrame(netif*& /*pNetif*/)
{
    struct pbuf* pCompleteFrame = 0L;
    uint16_t status             = _descriptors[_nextFree].status1;

    if (0 == (status & (ENET_ERXD_STATUS1_EMPTY(1) | ENET_ERXD_STATUS1_RO1(1))))
    {
        // buffer contains some rx data
        do
        {
            static struct pbuf* pFrame        = 0L;
            struct pbuf* const pCurrentBuffer = getCurrentBuffer();

            if (isErrornousBuffer(_descriptors[_nextFree]))
            {
                handleErrornousBuffer(pFrame, pCurrentBuffer);
            }
            else
            {
                if ((status & (ENET_ERXD_STATUS1_LAST(1))) != 0)
                {
                    handleLastBuffer(pFrame, pCompleteFrame, pCurrentBuffer);
                }
                else
                {
                    handleBuffer(pFrame, pCurrentBuffer);
                }
            }

            // advance _nextFree
            _nextFree = (_nextFree + 1U) % _descriptors.size();
            status    = _descriptors[_nextFree].status1;
        } while ((!pCompleteFrame)
                 && (0U == (status & (ENET_ERXD_STATUS1_EMPTY(1) | ENET_ERXD_STATUS1_RO1(1)))));
    }

    return pCompleteFrame;
}

void RxBuffers::handleBuffer(pbuf*& pFrame, pbuf* const pCurrentBuffer) const
{
    pCurrentBuffer->len     = _descriptors[_nextFree].length;
    pCurrentBuffer->tot_len = pCurrentBuffer->len;

    if (!pFrame)
    {
        // first frame
        pFrame      = pCurrentBuffer;
        pFrame->ref = 1;
    }
    else
    {
        pbuf_chain(pFrame, pCurrentBuffer);
    }
}

void RxBuffers::handleLastBuffer(
    pbuf*& pFrame, pbuf*& pCompleteFrame, pbuf* const pCurrentBuffer) const
{
    if (!pFrame)
    {
        // this frame spans only one buffer
        pCurrentBuffer->len     = _descriptors[_nextFree].length;
        pCurrentBuffer->tot_len = pCurrentBuffer->len;
        pFrame                  = pCurrentBuffer;
        pFrame->ref             = 1;
    }
    else
    {
        pCurrentBuffer->len     = _descriptors[_nextFree].length - pFrame->tot_len;
        pCurrentBuffer->tot_len = pCurrentBuffer->len;
        pbuf_chain(pFrame, pCurrentBuffer);
    }
    pCompleteFrame = pFrame;
    pFrame         = 0L;
}

} // namespace ethernet
