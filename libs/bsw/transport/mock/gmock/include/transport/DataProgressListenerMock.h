// Copyright 2024 Accenture.

/**
 * Contains
 * \file
 * \ingroup
 */

#pragma once

#include "transport/IDataProgressListener.h"

#include <gmock/gmock.h>

namespace transport
{
class DataProgressListenerMock : public IDataProgressListener
{
public:
    DataProgressListenerMock() {}

    MOCK_METHOD2(dataProgressed, void(TransportMessage&, uint32_t));
};

} // namespace transport

