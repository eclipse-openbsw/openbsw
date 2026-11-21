/********************************************************************************
 * Copyright (c) 2026 BMW AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include <gmock/gmock.h>
#include <cstdint>

namespace middleware::core::test
{

class ModeMock
{
public:
    MOCK_METHOD(bool, execute, ());
    MOCK_METHOD(bool, triggerEvent, ());
    MOCK_METHOD(bool, restartCyclicTimer, ());
    MOCK_METHOD(void, cancelPendingTransmissions, ());

    void EXPECT_EXECUTE(bool const returnValue = true)
    {
        EXPECT_CALL(*this, execute()).WillOnce(::testing::Return(returnValue));
    }

    void EXPECT_TRIGGER_EVENT(bool const returnValue = true)
    {
        EXPECT_CALL(*this, triggerEvent()).WillOnce(::testing::Return(returnValue));
    }

    void EXPECT_NO_EXECUTE() { EXPECT_CALL(*this, execute()).Times(0); }

    void EXPECT_NO_TRIGGER_EVENT() { EXPECT_CALL(*this, triggerEvent()).Times(0); }

    void EXPECT_CANCEL_PENDING_TRANSMISSIONS(uint8_t const times = 1U)
    {
        EXPECT_CALL(*this, cancelPendingTransmissions()).Times(times);
    }

    void EXPECT_RESTART_CYCLIC_TIMER(uint8_t const times = 0U, bool const returnValue = true)
    {
        EXPECT_CALL(*this, restartCyclicTimer())
            .Times(times)
            .WillRepeatedly(::testing::Return(returnValue));
    }
};

template<uint8_t ModeId>
class ModeMockAdapter
{
public:
    static void setMock(ModeMock* mock) { gInstance = mock; }

    static void unsetMock() { gInstance = nullptr; }

    static bool execute()
    {
        assert(gInstance != nullptr);
        return gInstance->execute();
    }

    static bool triggerEvent()
    {
        assert(gInstance != nullptr);
        return gInstance->triggerEvent();
    }

    static bool restartCyclicTimer()
    {
        assert(gInstance != nullptr);
        return gInstance->restartCyclicTimer();
    }

    static void cancelPendingTransmissions()
    {
        assert(gInstance != nullptr);
        gInstance->cancelPendingTransmissions();
    }

private:
    static inline ModeMock* gInstance{nullptr};
};

} // namespace middleware::core::test
