// Copyright 2024 Accenture.

#ifndef GUARD_EB766B15_490E_4361_AB2C_10672A55C5EB
#define GUARD_EB766B15_490E_4361_AB2C_10672A55C5EB

#include "docan/transmitter/IDoCanTickGenerator.h"

#include <gmock/gmock.h>

namespace docan
{
/**
 * Interface for DoCan tick generator.
 */
class DoCanTickGeneratorMock : public IDoCanTickGenerator
{
public:
    MOCK_METHOD0(tickNeeded, void());
};

} // namespace docan

#endif // GUARD_EB766B15_490E_4361_AB2C_10672A55C5EB
