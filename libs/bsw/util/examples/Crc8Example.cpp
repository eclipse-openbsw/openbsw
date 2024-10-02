// Copyright 2024 Accenture.

// EXAMPLE_START crc8example
#include "util/crc/Crc8.h"

#include <estd/slice.h>

namespace
{
using namespace ::util::crc;
using namespace ::util;

uint8_t calculateCrc8H2F(::estd::slice<uint8_t const> const data)
{
    // initialize CRC
    Crc8::Crc8H2F crc8;

    crc8.init(); // only necessary if the register will be reused (constructor initializes also)
    crc8.update(data);

    // return the CRC checksum
    return crc8.digest();
}

int main()
{
    uint8_t array[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09};
    ::estd::slice<uint8_t> data(array);

    uint8_t crcResult = calculateCrc8H2F(data);
    printf("CRC-8H2F: 0x%02x\n", crcResult);
}

} // namespace
  // EXAMPLE_END crc8example
