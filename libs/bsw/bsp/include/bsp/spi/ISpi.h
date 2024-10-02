// Copyright 2024 Accenture.

#ifndef GUARD_E3E96D5F_8DD8_4C72_82A0_B8E548DD0569
#define GUARD_E3E96D5F_8DD8_4C72_82A0_B8E548DD0569

#include <bsp/Bsp.h>

#include <platform/estdint.h>

namespace bios
{

class ISpi
{
public:
    virtual bsp::BspReturnCode transfer(
        uint8_t chan,
        uint8_t* pWdata,
        uint16_t wsize,
        uint8_t* pRdata,
        uint16_t rsize,
        uint8_t* pRdataSynchron = nullptr)
        = 0;
};

} // namespace bios

#endif // GUARD_E3E96D5F_8DD8_4C72_82A0_B8E548DD0569
