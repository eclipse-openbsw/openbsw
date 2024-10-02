// Copyright 2024 Accenture.

#ifndef GUARD_D87C647C_7757_4F9C_8A36_43150D09F907
#define GUARD_D87C647C_7757_4F9C_8A36_43150D09F907 1

#include "bsp/lin/linTransceiver/LinPhy.h"

#include <gmock/gmock.h>

namespace bsp
{
namespace lin
{
class LinPhyMock : public LinPhy
{
public:
    MOCK_METHOD1(init, void(uint32_t));
    MOCK_METHOD2(setMode, void(Mode, uint32_t));
    MOCK_METHOD1(getMode, Mode(uint32_t));
    MOCK_METHOD1(getPhyErrorStatus, ErrorCode(uint32_t));
};
} // namespace lin
} // namespace bsp

#endif
