// Copyright 2024 Accenture.

/**
 * Contains IntervalFilter class.
 * \file IntervalFilter.h
 * \ingroup filter
 */
#ifndef GUARD_F66B083F_F756_481B_A1D9_A9B2B549F8EE
#define GUARD_F66B083F_F756_481B_A1D9_A9B2B549F8EE

#include "can/canframes/CANFrame.h"
#include "can/filter/IFilter.h"
#include "can/filter/IMerger.h"

#include <platform/estdint.h>

namespace can
{
/**
 * Cpp2CAN IntervalFilter
 *
 *
 * \see IFilter
 */
class IntervalFilter : public IFilter
{
public:
    /** maximum id the filter may take */
    static uint32_t const MAX_ID = CanId::Extended<CANFrame::MAX_FRAME_ID_EXTENDED>::value;

    /**
     * constructor
     * \post getLowerBound() == MAX_ID
     * \post getUpperBound() == 0x0
     *
     * Nothing wil be accepted by default
     */
    IntervalFilter();

    /**
     * constructor initializing an interval
     * \param from    first id that will be accepted
     * \param to        last id that will be accepted
     * \pre from <= MAX_ID
     * \pre to <= MAX_ID
     * \post getLowerBound() == from
     * \post getUpperBound() == to
     *
     * \note * If from or to exceed MAX_ID, they will be set to MAX_ID.
     */
    explicit IntervalFilter(uint32_t from, uint32_t to);

    /**
     * \see IFilter::add()
     * \param filterId    id to add to filter
     * \pre filterId <= MAX_ID, otherwise no effect
     * \post getLowerBound() == min(filterId, getLowerBound())
     * \post getUpperBound() == max(filterId, getUpperBound())
     *
     * \note * This call is equal to add(filterId, filterId).
     */
    void add(uint32_t filterId) override;

    /**
     * \see IFilter::add()
     * \param from    lower bound of interval to add
     * \param to        upper bound of interval to add
     * \pre from <= MAX_ID
     * \pre to <= MAX_ID
     * \post getLowerBound() == min(from, getLowerBound())
     * \post getUpperBound() == max(to, getUpperBound())
     *
     * If from or to exceed MAX_ID, they will be set to MAX_ID.
     *
     * This call will not replace the current filter configuration, but
     * adjust lower and upper bound of the filter according to the parameters.
     */
    void add(uint32_t from, uint32_t to) override;

    /**
     * checks if an id matches the filter
     * \param filterId        id to check
     * \return *             - true: filter matches filterId
     *             - false: filterId is not in filters range
     */
    bool match(uint32_t filterId) const override;

    /**
     * \see IFilter::acceptMerger()
     */
    void acceptMerger(IMerger& merger) override { merger.mergeWithInterval(*this); }

    /**
     * \see IFilter::clear();
     */
    void clear() override;

    /**
     * \see IFilter::open();
     */
    void open() override;

    /**
     * \return lower bound of filter
     */
    uint32_t getLowerBound() const { return _from; }

    /**
     * \return upper bound of filter
     */
    uint32_t getUpperBound() const { return _to; }

private:
    // fields
    uint32_t _from;
    uint32_t _to;
};

} /*namespace can*/

#endif // GUARD_F66B083F_F756_481B_A1D9_A9B2B549F8EE
