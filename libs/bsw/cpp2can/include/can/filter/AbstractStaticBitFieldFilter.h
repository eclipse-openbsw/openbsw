// Copyright 2024 Accenture.

/**
 * Contains AbstractStaticBitFieldFilter class.
 * \file AbstractStaticBitFieldFilter.h
 * \ingroup filter
 */
#ifndef GUARD_E7A6E92B_357C_4D76_A06B_8631D2F5D167
#define GUARD_E7A6E92B_357C_4D76_A06B_8631D2F5D167

#include "can/canframes/CANFrame.h"
#include "can/filter/IFilter.h"
#include "can/filter/IMerger.h"

#include <estd/assert.h>
#include <platform/estdint.h>

#include <cstring>

namespace can
{
/**
 * Cpp2CAN AbstractStaticBitFieldFilter
 *
 *
 * A StaticBitFieldFilter allows to filter an id range
 * on CAN (0 - MAX_ID) and has a constant bitfield.
 */
class AbstractStaticBitFieldFilter : public IFilter
{
public:
    /** maximum id storable in this filter */
    static uint16_t const MAX_ID         = static_cast<uint16_t>(CANFrame::MAX_FRAME_ID);
    /** number of bits occupied by this filter */
    static uint16_t const NUMBER_OF_BITS = MAX_ID + 1U;
    /** size (in byte) of filters bitmask */
    static uint16_t const MASK_SIZE      = NUMBER_OF_BITS / 8U;

    /**
     * default constructor
     */
    AbstractStaticBitFieldFilter() : IFilter() {}

    /**
     * \see IFilter::add()
     */
    void add(uint32_t) override
    {
        // not applicable with a static filter
    }

    /**
     * \see IFilter::add()
     */
    void add(uint32_t, uint32_t) override
    {
        // not applicable with a static filter
    }

    /**
     * \see IFilter::match()
     * \param filterId        id to check
     * \return *             - true if internal mask matches filterId
     *             - false else
     */
    bool match(uint32_t filterId) const override;

    /**
     * \see IFilter::clear()
     */
    void clear() override
    {
        // not applicable with a static filter
    }

    /**
     * \see IFilter::open()
     */
    void open() override
    {
        // not applicable with a static filter
    }

    /**
     * \see IFilter::acceptMerger()
     */
    void acceptMerger(IMerger& merger) override { merger.mergeWithStaticBitField(*this); }

    /**
     * Get the byte at position byteIndex from the filter mask
     * \param byteIndex index of byte to retrieve
     */
    virtual uint8_t getMaskValue(uint16_t byteIndex) const = 0;

private:
    // friends
    friend bool
    operator==(AbstractStaticBitFieldFilter const& x, AbstractStaticBitFieldFilter const& y);
    // no copies
    AbstractStaticBitFieldFilter(AbstractStaticBitFieldFilter const&);
    AbstractStaticBitFieldFilter& operator=(AbstractStaticBitFieldFilter const&);
};

/**
 * compares two AbstractStaticBitFieldFilter objects
 * \param x    first operand of comparison
 * \param y    second operand of comparison
 * \return *             - true: both filters are equal
 *             - false: filters are not equal
 */
inline bool operator==(AbstractStaticBitFieldFilter const& x, AbstractStaticBitFieldFilter const& y)
{
    for (uint16_t i = 0U; i < AbstractStaticBitFieldFilter::MASK_SIZE; ++i)
    {
        if (x.getMaskValue(i) != y.getMaskValue(i))
        {
            return false;
        }
    }
    return true;
}

} // namespace can

#endif // GUARD_E7A6E92B_357C_4D76_A06B_8631D2F5D167
