// Copyright 2024 Accenture.

#include "eeprom/EepromDriver.h"

#include "bsp/Bsp.h"

#include <sys/stat.h>

#include <cstring>
#include <fcntl.h>
#include <unistd.h>

namespace eeprom
{

bsp::BspReturnCode EepromDriver::init()
{
    bool success = true;
    eepromFd     = open(eepromFilePath.c_str(), O_RDWR | O_CREAT | O_EXCL, 0666);
    if (eepromFd != -1)
    {
        success = (chmod(eepromFilePath.c_str(), 0666) != -1)
                  && (ftruncate(eepromFd, EEPROM_SIZE) != -1);

        if (success)
        {
            // Initialize file with 0xFF
            uint8_t buffer[EEPROM_SIZE];
            memset(buffer, 0xFF, EEPROM_SIZE);
            ssize_t const written = ::write(eepromFd, buffer, EEPROM_SIZE);
            success               = ((EEPROM_SIZE == written) && (fsync(eepromFd) == 0));
        }

        if (!success)
        {
            close(eepromFd);
            eepromFd = -1;
        }
    }
    else if (EEXIST == errno)
    {
        // File already exists: open it without O_EXCL
        eepromFd = open(eepromFilePath.c_str(), O_RDWR);
        success  = (-1 != eepromFd);
    }
    else
    {
        success = false;
    }

    if (!success)
    {
        return ::bsp::BSP_ERROR;
    }
    return ::bsp::BSP_OK;
}

bsp::BspReturnCode
EepromDriver::write(uint32_t const address, uint8_t const* const buffer, uint32_t const length)
{
    bool success
        = ((-1 != eepromFd) && (address < EEPROM_SIZE) && (length <= (EEPROM_SIZE - address)));

    success = (success && (lseek(eepromFd, address, SEEK_SET) != -1));
    success = (success && (::write(eepromFd, buffer, length) == length));
    success = (success && (fsync(eepromFd) == 0));

    if (!success)
    {
        printf("Failed to write to EEPROM file\r\n");
        return ::bsp::BSP_ERROR;
    }
    return ::bsp::BSP_OK;
}

bsp::BspReturnCode
EepromDriver::read(uint32_t const address, uint8_t* const buffer, uint32_t const length)
{
    bool success
        = ((-1 != eepromFd) && (address < EEPROM_SIZE) && (length <= (EEPROM_SIZE - address)));

    success = (success && (lseek(eepromFd, address, SEEK_SET) != -1));
    success = (success && (::read(eepromFd, buffer, length) == length));

    if (!success)
    {
        printf("Failed to read from EEPROM file\r\n");
        return ::bsp::BSP_ERROR;
    }
    return ::bsp::BSP_OK;
}

} // namespace eeprom
