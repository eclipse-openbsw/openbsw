/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "shed/ops_internal.h" // IWYU pragma: export

#include <etl/error_handler.h>

#include <algorithm>

namespace shed
{
namespace internal
{
void move_while_impl(
    internal::multi_list::idx_type* const begin,
    internal::multi_list::idx_type* end,
    multi_list* const ml,
    size_t const dst,
    ::etl::delegate<bool(size_t, size_t)> const pred,
    ::etl::delegate<move_op(size_t)> const csfr)
{
    ETL_ASSERT(begin != nullptr && end >= begin, ETL_ERROR_GENERIC("shed: invalid range"));
    std::make_heap(begin, end, pred);

    ETL_ASSERT(ml != nullptr, ETL_ERROR_GENERIC("shed: null multi_list"));

    while (begin != end)
    {
        std::pop_heap(begin, end, pred);
        --end;

        auto const r = csfr(*end);
        if ((r == move_op::MOVE) || (r == move_op::MOVE_DONE))
        {
            ml->move_idx(*end, dst);
        }
        if ((r == move_op::SKIP_DONE) || (r == move_op::MOVE_DONE))
        {
            return;
        }
    }
}

} // namespace internal
} // namespace shed
