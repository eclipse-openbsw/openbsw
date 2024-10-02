// Copyright 2024 Accenture.

#ifndef GUARD_5676EA4E_2F67_4842_A14D_2688944DE5F4
#define GUARD_5676EA4E_2F67_4842_A14D_2688944DE5F4

#include <estd/slice.h>
#include <platform/estdint.h>

namespace bsp
{
namespace flash
{
/**
 * IFlash formulates a interface to access the Flash Memory.
 */
struct IFlash
{
    /**
     * A FlashBlock is a logical abstraction of one or more physical flashblocks.
     *
     * <pre>
     * example:
     *  | <------- Flasblock ----------------> |
     *  -----------------------------------------------------------
     *  | PHY Block 8 Byte | PHY Block 16 Byte | PHY Block 8 Byte |
     *  -----------------------------------------------------------
     * </pre>
     */
    class FlashBlock
    {
    public:
        FlashBlock(uint32_t offset, uint32_t length);

        uint32_t offset() const;

        uint32_t length() const;

        bool isValid() const;

    private:
        uint32_t const _offset;
        uint32_t const _length;
    };

    /**
     * Get a memory view of the flash memory managed by the IFlash implementor.
     *
     * \return a memoryView of the the entire flash memory.
     */
    virtual ::estd::slice<uint8_t const> memory() const = 0;

    /**
     * Writes data to flash.
     *
     * \param offset into the flash memory to determine where data shall be written to.
     * \param data which shall be written.
     *
     * \return amount of bytes written.
     *
     * \attention a write call might not write the entire data immediately to the flash, data
     *            might be buffered between successive write calls.
     *            To make sure all data is written to flash call flush.
     */
    virtual uint32_t write(uint32_t offset, ::estd::slice<uint8_t const> data) = 0;

    /**
     * Makes sure all buffered data is written to the flash.
     *
     * \return <code>true</code> if all buffered data have been flushed successfully to the flash,
     *         otherwise <code>false</code>.
     */
    virtual bool flush() = 0;

    /**
     * Erase flash.
     *
     * \param flashBlock which shall be deleted.
     * \attention one logical flash block can consist of multiple physical flashblock,
     *            it is inevitable that the logical flash block complies to the underlying
     *            physical flash block boundaries. To make sure this is the case one
     *            can use the block method which returns a logical flash block which
     *            contains the provided memory area.
     *
     * \return <code>true</code> if the flash was erased sucessfully, otherwise <code>false</code>.
     */
    virtual bool erase(FlashBlock const& flashBlock) = 0;

    /**
     * Get a logical flash block based on an offset and a length.
     *
     * \param offset into the flash.
     * \pre the offset must point to the start of a flash block.
     *
     * \return  a flash block (start, size) wrapping at least the amount specified or
     *          the closest possible block. If the request exceedes the flash itself
     *          or an error occurs an invalid FlashBlock will be returned.
     *
     * examples:
     * <pre>
     *
     * Logical Flashblock fits
     * -----------------------
     *  params:
     *  offset = 8
     *  length = 16
     *
     *  Actual flash layout:
     *  -----------------------------------------------------------
     *  | PHY Block 8 Byte | PHY Block 16 Byte | PHY Block 8 Byte |
     *  -----------------------------------------------------------
     *                     | <Returned Block>  |
     *                      FlashBlock (offset=8, legnth= 16)
     *
     *
     * Logical Flashblock is approximated
     * ----------------------------------
     *  params:
     *  offset = 1
     *  length = 12
     *
     *  Actual flash layout:
     *  -----------------------------------------------------------
     *  | PHY Block 8 Byte | PHY Block 16 Byte | PHY Block 8 Byte |
     *  -----------------------------------------------------------
     *  | <-----Returned Flashblock ---------> |
     *     FlashBlock (offset=0, legnth= 24)
     *
     *
     * Logical Flashblock is invalid
     * -----------------------------
     *  params:
     *  offset = 24
     *  length = 16
     *
     *  Actual flash layout:
     *  -----------------------------------------------------------
     *  | PHY Block 8 Byte | PHY Block 16 Byte | PHY Block 8 Byte |
     *  -----------------------------------------------------------
     *                                         | <----------- Too Long -------------> |
     *                                         Invalid FlashBlock (offset=0, legnth= 0)
     * </pre>
     */
    virtual FlashBlock block(uint32_t offset, uint32_t length) const = 0;
};

inline IFlash::FlashBlock::FlashBlock(uint32_t const offset, uint32_t const length)
: _offset(offset), _length(length)
{}

inline uint32_t IFlash::FlashBlock::offset() const { return _offset; }

inline uint32_t IFlash::FlashBlock::length() const { return _length; }

inline bool IFlash::FlashBlock::isValid() const { return _length != 0U; }

} // namespace flash
} // namespace bsp

#endif /* GUARD_5676EA4E_2F67_4842_A14D_2688944DE5F4 */
