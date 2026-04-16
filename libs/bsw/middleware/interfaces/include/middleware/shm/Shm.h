// Copyright 2025 BMW AG

#pragma once

#include <cstdint>
#include <cstdlib>

namespace middleware::shm
{
/** Return the starting addr of the reserved shm section for intercore communication */
uint8_t* getShmSectionStartAddress();
/** Return the size of the reserved shm section for intercore communication */
size_t getShmSectionSize();

} // namespace middleware::shm
