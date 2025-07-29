// Copyright 2024 Accenture.

#pragma once

#include "io/DynamicClientCfg.h"
#include "io/Io.h"
#include "platform/estdint.h"

#include <etl/uncopyable.h>

namespace bios
{
class Output
{
public:
    /**
     * Configuration of outputs
     */
    struct OutputConfig
    {
        Io::PinId ioNumber;
        Io::Level defaultValue;
        bool isInverted;
    };

#undef BSPOUTPUTCONFIG
#include "bsp/io/output/outputConfiguration.h"

    // Api for all DynamicClients
    class IDynamicOutputClient
    {
        IDynamicOutputClient& operator=(IDynamicOutputClient const&);

    public:
        virtual bsp::BspReturnCode set(uint16_t chan, uint8_t vol, bool latch) = 0;
        virtual bsp::BspReturnCode get(uint16_t chan, bool& result)            = 0;
    };

    Output() {}

    void init(uint8_t hw, bool doSetup = true);

    void shutdown();

    void cyclic(void);

    /**
     * Interface output
     * \param see outputConfiguration
     * \param vol 0 -> 0, !0 -> 1
     * \return see bsp.h
     */
    static bsp::BspReturnCode set(OutputId chan, uint8_t vol, bool latch = true);

    /**
     * Interface output
     * \param see outputConfiguration
     * \return see bsp.h
     */
    static bsp::BspReturnCode get(OutputId chan, bool& result);

    /**
     * Interface output
     * \param see outputConfiguration
     * \return see bsp.h
     */
    static bsp::BspReturnCode invert(OutputId chan);

    /**
     * Interface output
     * \param see outputConfiguration
     * \return see bsp.h
     */
    static bsp::BspReturnCode release(OutputId chan);

    /**
     * Interface output
     * \param see outputConfiguration
     * \return see bsp.h
     */
    static bsp::BspReturnCode init(OutputId chan);

    /**
     * setDynamicOutputClient
     * \param outputNumber see outputConfiguration
     * \param clientOutputNumber number inside from OutputClient
     * \param see IDynamicOutputClient
     * \return see bsp.h
     */

    static bsp::BspReturnCode setDynamicClient(
        uint16_t outputNumber, uint16_t clientOutputNumber, IDynamicOutputClient* client);

    /**
     * clrDynamicOutputClient
     * \param outputNumber see outputConfiguration
     * \param see IDynamicOutputClient
     * \return see bsp.h
     */
    static bsp::BspReturnCode clrDynamicClient(uint16_t outputNumber);

    static char const* getName(uint16_t channel);

private:
    static void cleanDynamicClients();

    OutputConfig const* getConfiguration(uint8_t hw);
    static Output::OutputConfig const sfOutputConfigurations[][NUMBER_OF_INTERNAL_OUTPUTS];
    static Output::OutputConfig const* sfpOutputConfiguration;

    using dynamicClientType = uint16_t;
    static uint8_t const NUMBER_OF_DYNAMIC_OUTPUTS
        = ((NUMBER_OF_EXTERNAL_OUTPUTS > 0) ? static_cast<uint8_t>(NUMBER_OF_EXTERNAL_OUTPUTS)
                                            : 1U);

    static dynamicClient<dynamicClientType, IDynamicOutputClient, 4, NUMBER_OF_DYNAMIC_OUTPUTS>
        dynamicOutputCfg;
};

} /* namespace bios */
