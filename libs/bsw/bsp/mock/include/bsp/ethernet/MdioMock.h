// Copyright 2024 Accenture.

/**
 * \ingroup bsp_ethernet
 */
#ifndef GUARD_824793EA_55F5_48EC_BDDB_808F89539B2B
#define GUARD_824793EA_55F5_48EC_BDDB_808F89539B2B

#include "bsp/ethernet/IMdio.h"

#include <gmock/gmock.h>

namespace bsp
{
namespace ethernet
{
/**
 * Mock for interface IMdio.
 */
class MdioMock : public ::bsp::ethernet::IMdio
{
public:
    /** \see    IMdio::miimRead() */
    MOCK_METHOD3(miimRead, bsp::BspReturnCode(uint8_t, uint8_t, uint16_t&));

    /** \see    IMdio::miimWrite() */
    MOCK_METHOD3(miimWrite, bsp::BspReturnCode(uint8_t, uint8_t, uint16_t));

    /** \see    IMdio::smiRead() */
    MOCK_METHOD2(smiRead, bsp::BspReturnCode(uint8_t, uint8_t&));

    /** \see    IMdio::smiWrite() */
    MOCK_METHOD2(smiWrite, bsp::BspReturnCode(uint8_t, uint8_t));
};

} // namespace ethernet
} // namespace bsp

#endif /* GUARD_824793EA_55F5_48EC_BDDB_808F89539B2B */
