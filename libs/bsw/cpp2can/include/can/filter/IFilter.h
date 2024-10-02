// Copyright 2024 Accenture.

/**
 * Contains IFilter interface.
 * \file IFilter.h
 * \ingroup filter
 */
#ifndef GUARD_3E8D8547_086B_4CA9_AE22_2C7F6B4E7088
#define GUARD_3E8D8547_086B_4CA9_AE22_2C7F6B4E7088

#include <platform/estdint.h>

namespace can
{
class IMerger;

/**
 * common interface for filter classes
 *
 */
class IFilter
{
public:
    IFilter()                          = default;
    IFilter(IFilter const&)            = delete;
    IFilter& operator=(IFilter const&) = delete;

    /**
     * adds a single id to the filter
     * \param filterId    id to add
     * \post filter.match(filterId)
     */
    virtual void add(uint32_t filterId) = 0;

    /**
     * adds a range of ids to the filter
     * \param from    begin of range
     * \param to        end of range
     * \post filter.match(from...to);
     */
    virtual void add(uint32_t from, uint32_t to) = 0;

    /**
     * checks if a given id matches the filter
     * \return *             - true: filterId matches filter
     *             - false: filterId does not match filter
     */
    virtual bool match(uint32_t filterId) const = 0;

    /**
     * clears the filter so that nothing matches
     */
    virtual void clear() = 0;

    /**
     * opens the filters full range
     */
    virtual void open() = 0;

    /**
     * merges filter with a given merger
     * \param merger    IMerger to merge filter to
     *
     * This is part of the visitor pattern that is used to
     * merge different kinds of filters.
     */
    virtual void acceptMerger(IMerger& merger) = 0;
};

} // namespace can

#endif /*GUARD_3E8D8547_086B_4CA9_AE22_2C7F6B4E7088*/
