// Copyright 2024 Accenture.

#ifndef GUARD_A559F766_2356_472C_A4A8_C87FF2C731BB
#define GUARD_A559F766_2356_472C_A4A8_C87FF2C731BB

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

#endif /* GUARD_A559F766_2356_472C_A4A8_C87FF2C731BB */
