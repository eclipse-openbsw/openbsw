// Copyright 2024 Accenture.

#ifndef GUARD_1689F763_6338_4AC5_ADC7_1243E903D7CF
#define GUARD_1689F763_6338_4AC5_ADC7_1243E903D7CF

#include "estd/uncopyable.h"
#include "uds/base/Subfunction.h"

namespace uds
{
/**
 * UDS subfunction StartRoutine (0x31, 0x01)
 *
 */
class StartRoutine : public Subfunction
{
    UNCOPYABLE(StartRoutine);

public:
    StartRoutine();

private:
    static uint8_t const sfImplementedRequest[2];
};

} // namespace uds

#endif // GUARD_1689F763_6338_4AC5_ADC7_1243E903D7CF
