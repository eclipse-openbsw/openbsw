// Copyright 2024 Accenture.

#ifndef GUARD_6D4238CB_4CF6_465A_A475_E90F10079457
#define GUARD_6D4238CB_4CF6_465A_A475_E90F10079457

#include <platform/estdint.h>

// This class is in the global scope for backwards compatibility
class CRCCalculator
{
public:
    static uint16_t getCRC16(uint8_t const* pBuffer, uint32_t bufferLength);

    static uint16_t getCRC16(uint16_t preResult, uint8_t const* pBuffer, uint32_t bufferLength);

    static uint8_t getCRC8(uint16_t applicationId, uint8_t const* pBuffer, uint16_t bufferLength);

    static uint8_t getCRC8ATM(uint8_t initValue, uint8_t const* pBuffer, uint16_t bufferLength);

private:
    static uint16_t const crc16Table[256];
    static uint8_t const crc8Table[256];
    static uint8_t const crc8TableATM[256];
};

#endif /*GUARD_6D4238CB_4CF6_465A_A475_E90F10079457*/
