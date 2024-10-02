// Copyright 2024 Accenture.

/**
 * \ingroup bsp_ethernet
 */
#ifndef GUARD_18EB24D4_4FDF_4EF4_94FA_F525BC2AF61A
#define GUARD_18EB24D4_4FDF_4EF4_94FA_F525BC2AF61A

#include "bsp/ethernet/IMdioClause22.h"

#include <gmock/gmock.h>

namespace bsp
{
namespace ethernet
{
class MdioClause22Mock : public ::bsp::ethernet::IMdioClause22
{
public:
    MOCK_CONST_METHOD3(miimRead, bsp::BspReturnCode(uint8_t, uint8_t, uint16_t&));

    MOCK_METHOD3(miimWrite, bsp::BspReturnCode(uint8_t, uint8_t, uint16_t));
};

} // namespace ethernet
} // namespace bsp

#endif // GUARD_18EB24D4_4FDF_4EF4_94FA_F525BC2AF61A
