// Copyright 2024 Accenture.

#ifndef GUARD_3E449B5D_624D_4BFA_A515_D29AEB498226
#define GUARD_3E449B5D_624D_4BFA_A515_D29AEB498226

#include <bsp/Bsp.h>

#include <estd/uncopyable.h>
#include <estd/vector.h>
#include <platform/estdint.h>

namespace bsp
{
namespace memory
{
struct OverlayDescriptor
{
    uint8_t coreId;
    uintptr_t targetBaseAddress;
    uint32_t overlayBlockSize;
    uintptr_t redirectionBaseAddress;

    bool operator==(OverlayDescriptor const& descriptor) const
    {
        return (
            (this->coreId == descriptor.coreId)
            && (this->targetBaseAddress == descriptor.targetBaseAddress)
            && (this->overlayBlockSize == descriptor.overlayBlockSize)
            && (this->redirectionBaseAddress == descriptor.redirectionBaseAddress));
    }
};

/**
 * When using this class, be careful with caches.
 */
class IOverlayController
{
public:
    virtual ::bsp::BspReturnCode enableOverlayBlock(OverlayDescriptor const& descriptor) = 0;
    virtual void disableOverlayBlock(OverlayDescriptor const& descriptor)                = 0;

protected:
    IOverlayController& operator=(IOverlayController const&) = default;
};

} // namespace memory
} // namespace bsp

#endif // GUARD_3E449B5D_624D_4BFA_A515_D29AEB498226
