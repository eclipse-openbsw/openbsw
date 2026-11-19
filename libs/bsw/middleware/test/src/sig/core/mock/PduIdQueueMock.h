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

namespace middleware::sig::core::test
{

/**
 * Stands in for the PDU id queue. It has the nested Receiver that PduBrokerConnection uses, which
 * forwards to the mocked methods.
 */
class PduIdQueueMock
{
public:
    MOCK_METHOD(bool, isEmpty, (), (const));
    MOCK_METHOD(uint32_t, peek, (), (const));
    MOCK_METHOD(void, advance, ());

    class Receiver
    {
    public:
        explicit Receiver(PduIdQueueMock& queue) : _queue(queue) {}

        bool isEmpty() const { return _queue.isEmpty(); }

        uint32_t peek() const { return _queue.peek(); }

        void advance() { _queue.advance(); }

    private:
        PduIdQueueMock& _queue;
    };
};

} // namespace middleware::sig::core::test
