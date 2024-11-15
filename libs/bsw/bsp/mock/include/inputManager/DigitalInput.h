// Copyright 2024 Accenture.

#pragma once

#include <bsp/Bsp.h>
#include <etl/singleton_base.h>

#include <platform/estdint.h>

#include <gmock/gmock.h>

namespace bios
{
class DigitalInput : public ::etl::singleton_base<DigitalInput>
{
public:
    // Api for all DynamicClients
    class IDynamicInputClient
    {
    public:
        virtual bsp::BspReturnCode get(uint16_t chan, bool& result) = 0;
    };

    using DigitalInputId = uint8_t;

    class Mock
    {
    public:
        MOCK_METHOD(::bsp::BspReturnCode, get, (DigitalInputId, bool&), ());
    };

    DigitalInput() : ::etl::singleton_base<DigitalInput>(*this) {}

    static ::bsp::BspReturnCode get(DigitalInputId a, bool& b)
    {
        return DigitalInput::instance()._mock.get(a, b);
    }

    Mock _mock;
};

} // namespace bios
