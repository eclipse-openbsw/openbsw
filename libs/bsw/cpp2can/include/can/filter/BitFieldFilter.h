// Copyright 2024 Accenture.

/**
 * Contains BitFieldFilter class.
 * \file        BitFieldFilter.h
 * \ingroup        filter
 */
#pragma once

#include "can/canframes/CANFrame.h"
#include "can/filter/AbstractStaticBitFieldFilter.h"
#include "can/filter/IFilter.h"
#include "can/filter/IMerger.h"
#include "can/filter/IntervalFilter.h"

#include <platform/estdint.h>

#include <cstring>

namespace can
{
/**
 * Cpp2CAN BitFieldFilter
 *
 *
 * A BitFieldFilter for 11-bit CAN ids allows to filter an id range
 * on CAN (0 - MAX_ID).
 *
 * \see    IFilter
 * \see    IMerger
 */
class BitFieldFilter
: public IFilter
, public IMerger
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
    BitFieldFilter();

    /**
     * \see        IFilter::add()
     * \pre        filterId <= MAX_ID
     * \post    match(filterId)
     * \throws    assertion
     */
    void add(uint32_t filterId) override;

    /**
     * \see        IFilter::add()
     * \pre        from <= MAX_ID
     * \pre        to <= MAX_ID
     * \post    match(from...to)
     * \throws    assertion
     *
     * \note
     * Range includes both from and to.
     */
    void add(uint32_t from, uint32_t to) override;

    /**
     * \see        IFilter::match()
     * \param    filterId        id to check
     * \return
     *             - true if internal mask matches filterId
     *             - false else
     */
    bool match(uint32_t filterId) const override;

    /**
     * \see        IFilter::clear()
     */
    void clear() override { (void)memset(_mask, 0x0, static_cast<size_t>(MASK_SIZE)); }

    /**
     * \see        IFilter::open()
     */
    void open() override { (void)memset(_mask, 0xFF, static_cast<size_t>(MASK_SIZE)); }

    /**
     * \see        IFilter::acceptMerger()
     */
    void acceptMerger(IMerger& merger) override { merger.mergeWithBitField(*this); }

    /**
     * merges with a BitFieldFilter
     * \param    filter    BitFieldFilter to merge with
     */
    void mergeWithBitField(BitFieldFilter const& filter) override
    {
        for (uint16_t i = 0U; i < MASK_SIZE; ++i)
        {
            _mask[i] |= filter._mask[i];
        }
    }

    /**
     * merges with a AbstractStaticBitFieldFilter
     * \param    filter    AbstractStaticBitFieldFilter to merge with
     */
    void mergeWithStaticBitField(AbstractStaticBitFieldFilter const& filter) override
    {
        for (uint16_t i = 0U; i < MASK_SIZE; ++i)
        {
            _mask[i] |= filter.getMaskValue(i);
        }
    }

    /**
     * merges with an IntervalFilter
     * \param    filter    IntervalFilter to merge with
     */
    void mergeWithInterval(IntervalFilter const& filter) override
    {
        if (filter.getLowerBound() <= filter.getUpperBound())
        {
            uint32_t const toId
                = (filter.getUpperBound() <= MAX_ID) ? filter.getUpperBound() : MAX_ID;
            add(filter.getLowerBound(), toId);
        }
    }

    uint8_t const* getRawBitField() const { return &_mask[0]; }

private:
    friend bool operator==(BitFieldFilter const& x, BitFieldFilter const& y);
    BitFieldFilter(BitFieldFilter const&);
    BitFieldFilter& operator=(BitFieldFilter const&);
    uint8_t _mask[MASK_SIZE]{};
};

/**
 * compares two BitFieldFilter objects
 * \param    x    first operand of comparison
 * \param    y    second operand of comparison
 * \return
 *             - true: both filters are equal
 *             - false: filters are not equal
 */
inline bool operator==(BitFieldFilter const& x, BitFieldFilter const& y)
{
    for (uint16_t i = 0U; i < BitFieldFilter::MASK_SIZE; ++i)
    {
        if (x._mask[i] != y._mask[i])
        {
            return false;
        }
    }
    return true;
}

} // namespace can

