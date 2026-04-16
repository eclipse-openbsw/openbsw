#include "middleware/concurrency/LockStrategies.h"

namespace middleware::concurrency
{

void suspendAllInterrupts() {}

ScopedCoreLock::ScopedCoreLock() {}

ScopedCoreLock::~ScopedCoreLock() {}

ScopedECULock::ScopedECULock(uint8_t volatile*) {}

ScopedECULock::~ScopedECULock() {}

} // namespace middleware::concurrency
