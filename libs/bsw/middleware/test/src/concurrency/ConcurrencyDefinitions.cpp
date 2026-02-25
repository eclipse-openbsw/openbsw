#include "middleware/concurrency/LockStrategies.h"

namespace middleware
{
namespace concurrency
{

void suspendAllInterrupts() {}

ScopedCoreLock::ScopedCoreLock() {}

ScopedCoreLock::~ScopedCoreLock() {}

ScopedECULock::ScopedECULock(uint8_t volatile*) {}

ScopedECULock::~ScopedECULock() {}

} // namespace concurrency
} // namespace middleware
