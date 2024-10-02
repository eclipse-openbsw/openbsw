// Copyright 2024 Accenture.

#ifndef GUARD_2FC4874A_9B3F_442F_91D5_3B3788F133AC
#define GUARD_2FC4874A_9B3F_442F_91D5_3B3788F133AC

#include "bsp/xcp/IOverlayController.h"

#include <gmock/gmock.h>

namespace bsp
{
namespace memory
{
struct OverlayControllerMock : IOverlayController
{
    MOCK_METHOD1(enableOverlayBlock, ::bsp::BspReturnCode(OverlayDescriptor const&));
    MOCK_METHOD1(disableOverlayBlock, void(OverlayDescriptor const&));
};

} // namespace memory
} // namespace bsp

#endif
