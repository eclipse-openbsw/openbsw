// Copyright 2024 Accenture.

/**
 * \ingroup bsp_ethernet
 */
#ifndef GUARD_6FB24926_53D6_4444_B6A3_D4D1FC164F05
#define GUARD_6FB24926_53D6_4444_B6A3_D4D1FC164F05

#include "bsp/Bsp.h"

#include <platform/estdint.h>

namespace bsp
{
namespace ethernet
{
/**
 * Interface for MDIO clause 45 driver.
 *
 * \par MDIO
 * MDIO allows to access registers in a standardized way.
 * The PHY and device address are limited to 5 bits, i.e. valid
 * values range from 0 - 31.
 */
class IMdioClause45
{
public:
    /**
     * Reads a 16 bit MIIM value from a PHY register identified by
     * the PHY address (also named PHYAD), device address (also
     * named DEVAD) and register address.
     */
    virtual bsp::BspReturnCode
    miimRead(uint8_t phyAddress, uint8_t devAddress, uint16_t regAddress, uint16_t& data) const
        = 0;

    /**
     * Writes a 16 bit MIIM value to a PHY register identified by
     * the PHY address (also named PHYAD), device address (also
     * named DEVAD) and register address.
     */
    virtual bsp::BspReturnCode
    miimWrite(uint8_t phyAddress, uint8_t devAddress, uint16_t regAddress, uint16_t data)
        = 0;

protected:
    IMdioClause45& operator=(IMdioClause45 const&) = default;
};

} // namespace ethernet
} // namespace bsp

#endif /* GUARD_6FB24926_53D6_4444_B6A3_D4D1FC164F05 */
