// Copyright 2025 Accenture.

#pragma once

#include "bsp/Bsp.h"
#include "bsp/phy/phyConfiguration.h"
#include "platform/estdint.h"

namespace enetphy
{

class MdioTja1101
{
public:
    explicit MdioTja1101(PhyConfig const& config) : _config(config){};

    MdioTja1101(MdioTja1101 const&)            = delete;
    MdioTja1101& operator=(MdioTja1101 const&) = delete;

    /**
     * MIIM Read & Write
     */
    ::bsp::BspReturnCode miimRead(uint8_t phyAddress, uint8_t regAddress, uint16_t& data);
    ::bsp::BspReturnCode miimWrite(uint8_t phyAddr, uint8_t regAddr, uint16_t data);

private:
    uint16_t getDataPin() const;
    ::bsp::BspReturnCode transfer(uint32_t* frame, bool read) const;

    PhyConfig const& _config;
};

} // namespace enetphy
