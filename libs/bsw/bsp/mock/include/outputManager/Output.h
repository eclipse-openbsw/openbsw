// Copyright 2024 Accenture.

#pragma once

#include <bsp/Bsp.h>
#include <etl/singleton_base.h>

#include <platform/estdint.h>

#include <gmock/gmock.h>

namespace bios
{
class Output : public ::etl::singleton_base<Output>
{
public:
    // Api for all DynamicClients
    class IDynamicOutputClient
    {
    public:
        virtual bsp::BspReturnCode set(uint16_t chan, uint8_t vol, bool latch) = 0;
        virtual bsp::BspReturnCode get(uint16_t chan, bool& result)            = 0;
    };

    using OutputId = uint8_t;

    class Mock
    {
    public:
        MOCK_METHOD2(set, ::bsp::BspReturnCode(Output::OutputId, uint8_t));
        MOCK_METHOD2(get, ::bsp::BspReturnCode(Output::OutputId, uint8_t));
    };

    Output() : ::etl::singleton_base<Output>(*this) {}

    static ::bsp::BspReturnCode set(OutputId a, uint8_t b)
    {
        return Output::instance()._mock.set(a, b);
    }

    static ::bsp::BspReturnCode get(OutputId a, uint8_t b)
    {
        return Output::instance()._mock.get(a, b);
    }

    Mock _mock;
};

} // namespace bios
