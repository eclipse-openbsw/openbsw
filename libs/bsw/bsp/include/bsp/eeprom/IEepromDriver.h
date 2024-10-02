// Copyright 2024 Accenture.

#ifndef GUARD_9D2855EF_EBE0_42B5_B22C_5B850FF79081
#define GUARD_9D2855EF_EBE0_42B5_B22C_5B850FF79081

#include "bsp/Bsp.h"

#include <platform/estdint.h>

namespace eeprom
{
/**
 * IEepromDriver formulates the generic interface that describes a synchronous
 * access for a hardware-specific EEPROM.
 */
class IEepromDriver
{
public:
    struct EepromConfiguration
    {
        uint32_t fTotalSize;
        uint32_t fProtectedAreaSize;
        uint32_t fPageSize;
    };

    /**
     * Initializes the EEPROM.
     *
     * \return BSP_OK      EEPROM is initialized correctly
     *         BSP_ERROR   Error when initializing the EEPROM, EEPROM cannot
     *                      be used!
     */
    virtual bsp::BspReturnCode init() = 0;

    /**
     * Writes data to the EEPROM.
     * \param address     address in EEPROM where to write the data to
     * \param buffer      source from where data is read
     * \param length      number of bytes to be written
     * \return BSP_OK      indicates successful write
     *         BSP_ERROR   write error occurred, data in EEPROM may be corrupt
     */
    virtual bsp::BspReturnCode write(uint32_t address, uint8_t const* buffer, uint32_t length) = 0;

    /**
     * Reads data from the EEPROM.
     * \param address     where to read from in EEPROM
     * \param buffer      the data read is copied into this buffer
     * \param length      how many bytes are copied from EEPROM
     * \return BSP_OK      indicates successful read
     *         BSP_ERROR   read error occurred
     */
    virtual bsp::BspReturnCode read(uint32_t address, uint8_t* buffer, uint32_t length) = 0;

protected:
    IEepromDriver& operator=(IEepromDriver const&) = default;
};

} /* namespace eeprom */

#endif /* GUARD_9D2855EF_EBE0_42B5_B22C_5B850FF79081 */
