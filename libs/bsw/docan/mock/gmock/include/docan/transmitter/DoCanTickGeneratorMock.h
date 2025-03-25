// Copyright 2024 Accenture.

#pragma once

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

