// Copyright 2025 BMW AG

#pragma once

namespace middleware::shm
{

/// Definition is generated code
extern void initializeAllocators(bool mainCore);
extern void initializeQueues(bool mainCore);

/// Initialize shared memory entities.
/// \note The main core (the one who instantiates shared memory entities) must call this function
/// with mainCore=true.
inline void initializeShm(bool const mainCore)
{
    initializeAllocators(mainCore);
    initializeQueues(mainCore);
}

} // namespace middleware::shm
