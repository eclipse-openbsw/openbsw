// Copyright 2024 Accenture.

#pragma once

#include "uds/resume/IResumableResetDriverPersistence.h"

#include <gmock/gmock.h>

namespace uds
{
class ResumableResetDriverPersistenceMock : public IResumableResetDriverPersistence
{
public:
    MOCK_CONST_METHOD1(readRequest, bool(::transport::TransportMessage& message));
    MOCK_METHOD1(writeRequest, void(::transport::TransportMessage const& message));
    MOCK_METHOD0(clear, void());
};
} // namespace uds

