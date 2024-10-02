// Copyright 2024 Accenture.

#ifndef GUARD_2248D13B_185E_4DFC_86C6_C354310E3921
#define GUARD_2248D13B_185E_4DFC_86C6_C354310E3921

#include <bsp/Bsp.h>

#include <estd/singleton.h>
#include <platform/estdint.h>

#include <gmock/gmock.h>

namespace bios
{
class Output : public ::estd::singleton<Output>
{
public:
    // Api for all DynamicClients
    class IDynamicOutputClient
    {
    public:
        virtual bsp::BspReturnCode set(uint16_t chan, uint8_t vol, bool latch = true) = 0;
        virtual bsp::BspReturnCode get(uint16_t chan, bool& result)                   = 0;
    };

    using OutputId = uint8_t;

    class Mock
    {
    public:
        MOCK_METHOD2(set, ::bsp::BspReturnCode(Output::OutputId, uint8_t));
        MOCK_METHOD2(get, ::bsp::BspReturnCode(Output::OutputId, uint8_t));
    };

    Output() : ::estd::singleton<Output>(*this) {}

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

#endif // GUARD_2248D13B_185E_4DFC_86C6_C354310E3921
