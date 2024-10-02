// Copyright 2024 Accenture.

#ifndef GUARD_B2C93931_57F2_4152_BE44_7BAF51EC0E87
#define GUARD_B2C93931_57F2_4152_BE44_7BAF51EC0E87

#include "transport/TransportMessage.h"

#include <estd/slice.h>

#include <cstdint>
#include <vector>

namespace transport
{
namespace test
{
struct TransportMessageWithBuffer
{
    explicit TransportMessageWithBuffer(uint32_t size);
    TransportMessageWithBuffer(
        uint8_t sourceId,
        uint8_t targetId,
        ::estd::slice<uint8_t const> data,
        uint32_t maxSize = 0);

    TransportMessageWithBuffer(TransportMessageWithBuffer const&)            = delete;
    TransportMessageWithBuffer& operator=(TransportMessageWithBuffer const&) = delete;

    TransportMessage* get() { return &m; }

    TransportMessage& operator*() { return m; }

    TransportMessage* operator->() { return &m; }

private:
    std::vector<uint8_t> buffer;
    TransportMessage m;
};

} // namespace test
} // namespace transport

#endif // GUARD_B2C93931_57F2_4152_BE44_7BAF51EC0E87
