// Copyright 2024 Accenture.

#pragma once

#include "uds/resume/IResumableResetDriverPersistence.h"

#include <gmock/gmock.h>

namespace uds
{
class ResumableResetDriverPersistenceMock : public IResumableResetDriverPersistence
{
public:
    MOCK_METHOD(bool, readRequest, (::transport::TransportMessage & message), (const, override));
    MOCK_METHOD(void, writeRequest, (::transport::TransportMessage const& message), (override));
    MOCK_METHOD(void, clear, (), (override));
};
} // namespace uds
