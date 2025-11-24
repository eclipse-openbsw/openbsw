// Copyright 2024 Accenture.

#include "eeprom/EepromDriver.h"

#include "bsp/Bsp.h"

#include <sys/stat.h>

#include <cstring>
#include <fcntl.h>
#include <unistd.h>

namespace eeprom
{

EepromDriver::EepromDriver() : eepromFd(-1)
{
    bool fileExisted = false;

    // Try opening existing file first
    eepromFd = open(eepromFilePath.c_str(), O_RDWR);

    if (eepromFd != -1)
    {
        fileExisted = true;
    }
    else
    {
        // If opening fails, try creating it
        eepromFd = open(eepromFilePath.c_str(), O_RDWR | O_CREAT, 0666);

        if (eepromFd != -1)
        {
            chmod(eepromFilePath.c_str(), 0666);
        }
    }

    // Initialize only for newly created files
    if (eepromFd != -1 && !fileExisted)
    {
        // Resize EEPROM file
        if (ftruncate(eepromFd, EEPROM_SIZE) == 0)
        {
            // Fill with 0xFF
            uint8_t buffer[EEPROM_SIZE];
            memset(buffer, 0xFF, EEPROM_SIZE);

            // After truncate, file offset is guaranteed to be 0
            ssize_t const written = ::write(eepromFd, buffer, EEPROM_SIZE);

            if (written == static_cast<ssize_t>(EEPROM_SIZE))
            {
                fsync(eepromFd);
            }
        }
    }
}

bsp::BspReturnCode EepromDriver::init()
{
    if (eepromFd == -1)
    {
        return ::bsp::BSP_ERROR;
    }

    // Verify file size is correct
    struct stat fileStat;
    if (fstat(eepromFd, &fileStat) != 0)
    {
        return ::bsp::BSP_ERROR;
    }

    // If file size doesn't match expected EEPROM size, resize it
    if (fileStat.st_size != EEPROM_SIZE)
    {
        if (ftruncate(eepromFd, EEPROM_SIZE) != 0)
        {
            return ::bsp::BSP_ERROR;
        }

        if (fsync(eepromFd) != 0)
        {
            return ::bsp::BSP_ERROR;
        }
    }

    return ::bsp::BSP_OK;
}

bsp::BspReturnCode
EepromDriver::write(uint32_t const address, uint8_t const* const buffer, uint32_t const length)
{
    bool success
        = ((-1 != eepromFd) && (address < EEPROM_SIZE) && (length <= (EEPROM_SIZE - address)));

    success = (success && (lseek(eepromFd, address, SEEK_SET) != -1));
    success = (success && (::write(eepromFd, buffer, length) == static_cast<ssize_t>(length)));
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
    success = (success && (::read(eepromFd, buffer, length) == static_cast<ssize_t>(length)));

    if (!success)
    {
        printf("Failed to read from EEPROM file\r\n");
        return ::bsp::BSP_ERROR;
    }
    return ::bsp::BSP_OK;
}

EepromDriver::~EepromDriver()
{
    if (-1 != eepromFd)
    {
        fsync(eepromFd);
        close(eepromFd);
        eepromFd = -1;
    }
}

} // namespace eeprom
