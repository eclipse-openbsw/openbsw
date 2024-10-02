// Copyright 2024 Accenture.

/**
 * \ingroup bsp_ethernet
 */
#ifndef GUARD_49810DF0_6E88_461D_BA8C_AEBDD095D87B
#define GUARD_49810DF0_6E88_461D_BA8C_AEBDD095D87B

#include "bsp/Bsp.h"

#include <platform/estdint.h>

namespace bsp
{
namespace ethernet
{
/**
 * Interface for MDIO driver.
 *
 * \par MDIO
 * MDIO allows to access registers in a standardized way.
 * The phy address is limited to 5 bits, i.e. valid values
 * range from 0 - 31.
 * \deprecated Newer implementations should use IMdioClause22 or IMdioClause45 instead
 */
class IMdio
{
public:
    /**
     * Reads a 16 bit MIIM value from a phy register identified by
     * phyAddress and regAddress.
     */
    virtual bsp::BspReturnCode miimRead(uint8_t phyAddress, uint8_t regAddress, uint16_t& data) = 0;

    /**
     * Writes a 16 bit MIIM value to a phy register identified by
     * phyAddress and regAddress.
     */
    virtual bsp::BspReturnCode miimWrite(uint8_t phyAddress, uint8_t regAddress, uint16_t data) = 0;

    /**
     * Reads a 8 bit SMI value from a switch register
     * identified by a given regAddress.
     */
    virtual bsp::BspReturnCode smiRead(uint8_t regAddress, uint8_t& data) = 0;

    /**
     * Writes a 8 bit SMI value to a switch register
     * identified by a given regAddress.
     */
    virtual bsp::BspReturnCode smiWrite(uint8_t regAddress, uint8_t data) = 0;

protected:
    IMdio& operator=(IMdio const&) = default;
};

} // namespace ethernet
} // namespace bsp

#endif /* GUARD_49810DF0_6E88_461D_BA8C_AEBDD095D87B */
