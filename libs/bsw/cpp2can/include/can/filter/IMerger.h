// Copyright 2024 Accenture.

/**
 * Contains IMerger interface.
 * \file IMerger.h
 * \ingroup filter
 */
#ifndef GUARD_058F2EBC_3D62_4D90_9E7A_4B24E3C99085
#define GUARD_058F2EBC_3D62_4D90_9E7A_4B24E3C99085

namespace can
{
class BitFieldFilter;
class AbstractStaticBitFieldFilter;
class IntervalFilter;

/**
 * interface for class that are able to merge with other filter classes
 * @class        IMerger
 *
 *
 * \see BitFieldFilter
 * \see AbstractStaticBitFieldFilter
 * \see IntervalFilter
 */
class IMerger
{
public:
    IMerger()                          = default;
    IMerger(IMerger const&)            = delete;
    IMerger& operator=(IMerger const&) = delete;

    /**
     * merges with a BitFieldFilter
     * \param filter    BitFieldFilter to merge with
     */
    virtual void mergeWithBitField(BitFieldFilter const& filter) = 0;

    /**
     * merges with a AbstractStaticBitFieldFilter
     * \param filter    AbstractStaticBitFieldFilter to merge with
     */
    virtual void mergeWithStaticBitField(AbstractStaticBitFieldFilter const& filter) = 0;

    /**
     * merges with a IntervalFilter
     * \param filter    IntervalFilter to merge with
     */
    virtual void mergeWithInterval(IntervalFilter const& filter) = 0;
};

} // namespace can

#endif // GUARD_058F2EBC_3D62_4D90_9E7A_4B24E3C99085
