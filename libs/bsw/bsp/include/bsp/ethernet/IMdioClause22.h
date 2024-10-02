// Copyright 2024 Accenture.

/**
 * \ingroup bsp_ethernet
 */
#ifndef GUARD_9695B585_5AFB_44CC_B709_59592B13F419
#define GUARD_9695B585_5AFB_44CC_B709_59592B13F419

#include "bsp/Bsp.h"

#include <platform/estdint.h>

namespace bsp
{
namespace ethernet
{
/**
 * Interface for MDIO clause 22 driver.
 *
 * \par MDIO
 * MDIO allows to access registers in a standardized way.
 * The PHY address is limited to 5 bits, i.e. valid values
 * range from 0 - 31.
 */
class IMdioClause22
{
public:
    /**
     * Reads a 16 bit MIIM value from a PHY register identified by
     * the PHY address (also named PHYAD) and register address.
     */
    virtual bsp::BspReturnCode
    miimRead(uint8_t phyAddress, uint8_t regAddress, uint16_t& data) const
        = 0;

    /**
     * Writes a 16 bit MIIM value to a PHY register identified by
     * the PHY address (also named PHYAD) and register address.
     */
    virtual bsp::BspReturnCode miimWrite(uint8_t phyAddress, uint8_t regAddress, uint16_t data) = 0;

protected:
    IMdioClause22& operator=(IMdioClause22 const&) = default;
};

} // namespace ethernet
} // namespace bsp

#endif /* GUARD_9695B585_5AFB_44CC_B709_59592B13F419 */
