/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

/**
 * Contains MaskFilter class.
 * \file MaskFilter.h
 * \ingroup filter
 */
#pragma once

#include "can/filter/IFilter.h"
#include "can/filter/IMerger.h"

#include <platform/estdint.h>

namespace can
{
/**
 * Cpp2CAN MaskFilter
 *
 * A filter that matches ids by a bit mask/pattern pair: an id matches if
 * (id & mask) == pattern, i.e. it allows to fix an arbitrary subset of the 32 id bits to a
 * given value while leaving the remaining bits unconstrained (\"don't care\").
 *
 * This is useful for addressing schemes that encode information directly into fixed bit
 * positions of the CAN identifier (as opposed to a contiguous range of ids), e.g. ISO 15765-2
 * normal fixed addressing, where the upper bits of the (29 bit) identifier are fixed while the
 * target/source address bits are not.
 *
 * \see IFilter
 */
class MaskFilter : public IFilter
{
public:
    /**
     * constructor
     * \post nothing matches
     */
    MaskFilter();

    /**
     * constructor initializing the filter with a range of ids
     * \param from first id that will be accepted
     * \param to last id that will be accepted
     * \post filter.match(from...to)
     *
     * \note the resulting mask/pattern may accept more ids than just the given range, since only
     * whole, mask-aligned blocks of ids can be represented (comparable to CIDR block
     * summarization of an address range). This is intended, coarse over-matching is acceptable
     * for a filter that is used as a pre-filter only.
     */
    MaskFilter(uint32_t from, uint32_t to);

    /**
     * \see IFilter::add()
     * \post match(filterId)
     */
    void add(uint32_t filterId) override;

    /**
     * \see IFilter::add()
     * \post match(from...to)
     *
     * \note see note on MaskFilter(uint32_t, uint32_t)
     */
    void add(uint32_t from, uint32_t to) override;

    /**
     * \see IFilter::match()
     */
    bool match(uint32_t filterId) const override;

    /**
     * \see IFilter::clear()
     */
    void clear() override;

    /**
     * \see IFilter::open()
     */
    void open() override;

    /**
     * \see IFilter::acceptMerger()
     */
    void acceptMerger(IMerger& merger) override { merger.mergeWithMask(*this); }

    /**
     * \return the bit mask of this filter (bits set to 1 are fixed to the corresponding bit of
     * getPattern(), bits set to 0 are unconstrained).
     */
    uint32_t getMask() const { return _mask; }

    /**
     * \return the bit pattern of this filter (only bits that are set in getMask() are relevant).
     */
    uint32_t getPattern() const { return _pattern; }

    MaskFilter(MaskFilter const&)            = delete;
    MaskFilter& operator=(MaskFilter const&) = delete;

private:
    void widen(uint32_t mask, uint32_t pattern);

    uint32_t _mask;
    uint32_t _pattern;
    bool _empty;
};

} // namespace can
