// Copyright 2024 Accenture.

#ifndef GUARD_53C96C3A_3F15_4751_9E94_D893F5C2A8CA
#define GUARD_53C96C3A_3F15_4751_9E94_D893F5C2A8CA

#include <platform/estdint.h>

namespace bsp
{
namespace lin
{
class ILinSciReceiver
{
public:
    struct RawData
    {
        uint8_t pid;
        uint8_t payload[8];
        uint8_t payloadLength;
        uint8_t crc;
    };

    virtual uint8_t payloadLength(uint8_t pid) = 0;

    virtual void push(RawData& rawData) = 0;

protected:
    ILinSciReceiver& operator=(ILinSciReceiver const&) = default;
};

} // namespace lin
} // namespace bsp

#endif /* GUARD_53C96C3A_3F15_4751_9E94_D893F5C2A8CA */
