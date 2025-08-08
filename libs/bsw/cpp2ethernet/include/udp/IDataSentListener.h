// Copyright 2025 Accenture.

#pragma once

#include <platform/estdint.h>

namespace udp
{
class IDataSentListener
{
protected:
    IDataSentListener() = default;

public:
    IDataSentListener(IDataSentListener const&)                  = delete;
    IDataSentListener& operator=(IDataSentListener const&)       = delete;
    virtual void dataSent(uint8_t const data[], uint16_t length) = 0;
};

} // namespace udp
