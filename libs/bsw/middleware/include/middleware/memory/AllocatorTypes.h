// Copyright 2025 BMW AG

#pragma once

#include <cstdint>

namespace middleware::memory::alloc
{

/**
 * CRTP base class for lightweight allocators.
 *
 * \tparam TSuper Derived allocator type (CRTP).
 */
template<typename TSuper>
class AllocatorBase
{
public:
    static TSuper& instance()
    {
        ETL_ASSERT(_instance != nullptr, ETL_ERROR_GENERIC("Allocator instance not set."));
        return *_instance;
    }

    static void* allocate(uint32_t const size) { return instance().doAllocate(size); }

    static bool deallocate(void* const p) { return instance().doDeallocate(p); }

    static void setInstance(TSuper& super) { _instance = &super; }

private:
    static TSuper* _instance;
};

} // namespace middleware::memory::alloc
