// Copyright 2024 Accenture.

#ifndef GUARD_5AB91BE2_331A_42D6_981C_83EFDF7742FF
#define GUARD_5AB91BE2_331A_42D6_981C_83EFDF7742FF

#include <platform/estdint.h>

namespace flash
{
/**
 * @class       IFlashDriver
 *
 * IFlashDriver formulates a generic interface to access the Flash Memory
 */
class IFlashDriver
{
public:
    enum FlashOperationStatus
    {
        FLASH_OP_SUCCESSFUL,
        FLASH_OP_FAILED
    };

    virtual FlashOperationStatus write(uint32_t destination, uint8_t const* source, uint32_t size)
        = 0;

    virtual FlashOperationStatus erase(uint32_t address, uint32_t size) = 0;

    virtual FlashOperationStatus flush() = 0;

    /**
     * Calculates flash block size for given block start address.
     * \param blockStartAddress address of a flash block
     * \param blockSize size of a flash block
     * \return FLASH_OP_SUCCESSFUL if blockStartAddress is a start address
     *          of some flash block,
     *          FLASH_OP_FAILED is returned otherwise and blockSize is set to 0
     */
    virtual FlashOperationStatus getBlockSize(uint32_t blockStartAddress, uint32_t& blockSize) = 0;

protected:
    IFlashDriver& operator=(IFlashDriver const&) = default;
};

} /* namespace flash */

#endif /* GUARD_5AB91BE2_331A_42D6_981C_83EFDF7742FF */
