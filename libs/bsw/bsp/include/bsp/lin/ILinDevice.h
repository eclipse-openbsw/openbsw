// Copyright 2024 Accenture.

#ifndef GUARD_E2A6457B_8D54_4DC9_9D4F_756ABA354A43
#define GUARD_E2A6457B_8D54_4DC9_9D4F_756ABA354A43

#include <platform/estdint.h>

namespace bios
{
class ILinDevice
{
public:
    using tRawFrame = struct RawFrame
    {
        uint8_t payload[8];
        uint8_t id;
        uint8_t payloadLength;

        enum
        {
            DIR_TX,
            DIR_RX
        } direction;

        enum
        {
            CRC_NORMAL,
            CRC_ENHANCED
        } crcType;
    };

    virtual void init(uint8_t speedMode = 0U) = 0;

    virtual void open() = 0;

    virtual void close() = 0;

    virtual void shutdown() = 0;

    virtual void sendWakeupPattern() = 0;

    virtual void sendSleepCommand() = 0;

    virtual bool wakeupPatternReceived() = 0;

    /* ********** ********** ********** ********** ********** */

    virtual bool frameComplete() = 0;

    virtual bool slaveTimeout() = 0;

    virtual bool stuckAtZero() = 0;

    virtual bool bitError() = 0;

    virtual bool framingError() = 0;

    virtual bool checksumError() = 0;

    /* ********** ********** ********** ********** ********** */

    virtual RawFrame& getRefToRawFrame() = 0;

    virtual void transceive() = 0;

    virtual void fetchRawFrame() = 0;

    /* ********** ********** ********** ********** ********** */

    virtual uint8_t readTxPortAsGpio() { return 1U; }

    virtual uint8_t readRxPortAsGpio() { return 1U; }

protected:
    ILinDevice& operator=(ILinDevice const&) = default;
};

} // namespace bios

#endif
