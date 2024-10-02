// Copyright 2024 Accenture.

/**
 * \ingroup bsp_ethernet
 */
#ifndef GUARD_310512DA_D726_472F_877A_8BB644CAF3E9
#define GUARD_310512DA_D726_472F_877A_8BB644CAF3E9

#include "bsp/ethernet/IMdioClause45.h"

#include <gmock/gmock.h>

namespace bsp
{
namespace ethernet
{
class MdioClause45Mock : public ::bsp::ethernet::IMdioClause45
{
public:
    MOCK_CONST_METHOD4(miimRead, bsp::BspReturnCode(uint8_t, uint8_t, uint16_t, uint16_t&));

    MOCK_METHOD4(miimWrite, bsp::BspReturnCode(uint8_t, uint8_t, uint16_t, uint16_t));
};

} // namespace ethernet
} // namespace bsp

#endif // GUARD_310512DA_D726_472F_877A_8BB644CAF3E9
