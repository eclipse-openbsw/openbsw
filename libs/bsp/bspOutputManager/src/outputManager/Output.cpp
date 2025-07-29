// Copyright 2024 Accenture.

#include "outputManager/Output.h"

#include "interrupts/SuspendResumeAllInterruptsLock.h"
#include "io/Io.h"

namespace bios
{
#undef BSPOUTPUTCONFIG
#define BSPOUTPUTCONFIG 1
#include "bsp/io/output/outputConfiguration.h"
#include "bsp/io/output/outputConfigurationStrings.h"

Output::OutputConfig const* Output::sfpOutputConfiguration = nullptr;
dynamicClient<
    Output::dynamicClientType,
    Output::IDynamicOutputClient,
    4,
    Output::NUMBER_OF_DYNAMIC_OUTPUTS>
    Output::dynamicOutputCfg;

void Output::init(uint8_t const hw, bool const doSetup)
{
    sfpOutputConfiguration = getConfiguration(hw);
    if (nullptr == sfpOutputConfiguration)
    {
        return;
    }
    if (true == doSetup)
    {
        for (uint8_t i = 0U; i < static_cast<uint8_t>(NUMBER_OF_INTERNAL_OUTPUTS); ++i)
        {
            (void)Io::setPin(
                static_cast<uint16_t>(sfpOutputConfiguration[i].ioNumber),
                sfpOutputConfiguration[i].defaultValue == Io::HIGH);
            (void)Io::setDefaultConfiguration(
                static_cast<uint16_t>(sfpOutputConfiguration[i].ioNumber));
        }
    }
    cleanDynamicClients();
}

void Output::shutdown() { cleanDynamicClients(); }

bsp::BspReturnCode Output::invert(OutputId const chan)
{
    if (nullptr == sfpOutputConfiguration)
    {
        return bsp::BSP_ERROR;
    }

    if (chan < NUMBER_OF_INTERNAL_OUTPUTS)
    {
        bool t;
        if (Io::PORT_UNAVAILABLE == sfpOutputConfiguration[chan].ioNumber)
        {
            return bsp::BSP_NOT_SUPPORTED;
        }
        t = Io::getPin(static_cast<uint16_t>(sfpOutputConfiguration[chan].ioNumber));
        if (t)
        {
            (void)Io::setPin(static_cast<uint16_t>(sfpOutputConfiguration[chan].ioNumber), 0);
        }
        else
        {
            (void)Io::setPin(static_cast<uint16_t>(sfpOutputConfiguration[chan].ioNumber), 1);
        }
        return bsp::BSP_OK;
    }
    else if (chan == NUMBER_OF_INTERNAL_OUTPUTS)
    {
        return bsp::BSP_NOT_SUPPORTED;
    }
    else if (chan < TOTAL_NUMBER_OF_OUTPUTS)
    {
        // dynamic instance
        dynamicClientType const channel = chan - NUMBER_OF_INTERNAL_OUTPUTS - 1;
        if (channel < NUMBER_OF_DYNAMIC_OUTPUTS)
        {
            if (dynamicOutputCfg.getClientValid(channel) == true)
            {
                bool t;
                bsp::BspReturnCode const ret = dynamicOutputCfg.getClientInstance(channel)->get(
                    dynamicOutputCfg.getChannelInsideClient(channel), t);
                if (ret == bsp::BSP_OK)
                {
                    if (t)
                    {
                        return (dynamicOutputCfg.getClientInstance(channel)->set(
                            dynamicOutputCfg.getChannelInsideClient(channel), 0U, true));
                    }
                    else
                    {
                        return (dynamicOutputCfg.getClientInstance(channel)->set(
                            dynamicOutputCfg.getChannelInsideClient(channel), 1U, true));
                    }
                }
                else
                {
                    return bsp::BSP_ERROR;
                }
            }
            else
            {
                return bsp::BSP_ERROR;
            }
        }
        else
        {
            return bsp::BSP_ERROR;
        }
    }
    else
    {
        return bsp::BSP_ERROR;
    }
}

bsp::BspReturnCode Output::init(OutputId const chan)
{
    if (nullptr == sfpOutputConfiguration)
    {
        return bsp::BSP_ERROR;
    }
    if (chan < NUMBER_OF_INTERNAL_OUTPUTS)
    {
        if (Io::PORT_UNAVAILABLE == sfpOutputConfiguration[chan].ioNumber)
        {
            return bsp::BSP_NOT_SUPPORTED;
        }
        (void)Io::setDefaultConfiguration(
            static_cast<uint16_t>(sfpOutputConfiguration[chan].ioNumber));
        return bsp::BSP_OK;
    }
    return bsp::BSP_ERROR;
}

bsp::BspReturnCode Output::release(OutputId const chan)
{
    if (nullptr == sfpOutputConfiguration)
    {
        return bsp::BSP_ERROR;
    }

    if (chan < NUMBER_OF_INTERNAL_OUTPUTS)
    {
        if (Io::PORT_UNAVAILABLE == sfpOutputConfiguration[chan].ioNumber)
        {
            return bsp::BSP_NOT_SUPPORTED;
        }
        (void)Io::resetConfig(static_cast<uint16_t>(sfpOutputConfiguration[chan].ioNumber));
        return bsp::BSP_OK;
    }
    return bsp::BSP_ERROR;
}

bsp::BspReturnCode Output::get(OutputId const chan, bool& result)
{
    if (nullptr == sfpOutputConfiguration)
    {
        return bsp::BSP_ERROR;
    }

    if (chan < NUMBER_OF_INTERNAL_OUTPUTS)
    {
        bool volRow = false;
        if (Io::PORT_UNAVAILABLE == sfpOutputConfiguration[chan].ioNumber)
        {
            return bsp::BSP_NOT_SUPPORTED;
        }
        volRow = Io::getPin(static_cast<uint16_t>(sfpOutputConfiguration[chan].ioNumber));
        if (sfpOutputConfiguration[chan].isInverted)
        {
            volRow = !volRow;
        }
        result = volRow;
        return bsp::BSP_OK;
    }
    else if (chan == NUMBER_OF_INTERNAL_OUTPUTS)
    {
        return bsp::BSP_ERROR;
    }
    else if (chan < TOTAL_NUMBER_OF_OUTPUTS)
    {
        // dynamic instance
        dynamicClientType const channel = chan - NUMBER_OF_INTERNAL_OUTPUTS - 1;
        if (channel < NUMBER_OF_DYNAMIC_OUTPUTS)
        {
            if (dynamicOutputCfg.getClientValid(channel) == true)
            {
                return (dynamicOutputCfg.getClientInstance(channel)->get(
                    dynamicOutputCfg.getChannelInsideClient(channel), result));
            }
            else
            {
                return bsp::BSP_ERROR;
            }
        }
        else
        {
            return bsp::BSP_ERROR;
        }
    }
    else
    {
        return bsp::BSP_ERROR;
    }
}

bsp::BspReturnCode Output::set(OutputId const chan, uint8_t const vol, bool const latch)
{
    if (nullptr == sfpOutputConfiguration)
    {
        return bsp::BSP_ERROR;
    }

    if (chan < NUMBER_OF_INTERNAL_OUTPUTS)
    {
        if (Io::PORT_UNAVAILABLE == sfpOutputConfiguration[chan].ioNumber)
        {
            return bsp::BSP_NOT_SUPPORTED;
        }
        bool volRow = !(vol == 0U);
        if (sfpOutputConfiguration[chan].isInverted)
        {
            volRow = vol == 0U;
        }
        (void)Io::setPin(static_cast<uint16_t>(sfpOutputConfiguration[chan].ioNumber), volRow);
        return bsp::BSP_OK;
    }
    else if (chan == NUMBER_OF_INTERNAL_OUTPUTS)
    {
        return bsp::BSP_NOT_SUPPORTED;
    }
    else if (chan < TOTAL_NUMBER_OF_OUTPUTS)
    {
        // dynamic instance
        dynamicClientType const channel = chan - NUMBER_OF_INTERNAL_OUTPUTS - 1;
        if (channel < NUMBER_OF_DYNAMIC_OUTPUTS)
        {
            if (dynamicOutputCfg.getClientValid(channel) == true)
            {
                return (dynamicOutputCfg.getClientInstance(channel)->set(
                    dynamicOutputCfg.getChannelInsideClient(channel), vol, latch));
            }
            else
            {
                return bsp::BSP_ERROR;
            }
        }
        else
        {
            return bsp::BSP_ERROR;
        }
    }
    else
    {
        return bsp::BSP_ERROR;
    }
}

bsp::BspReturnCode Output::setDynamicClient(
    uint16_t const outputNumber,
    uint16_t const clientOutputNumber,
    IDynamicOutputClient* const client)
{
    // range of outputNumber check
    if ((outputNumber > static_cast<uint16_t>(NUMBER_OF_INTERNAL_OUTPUTS))
        && (outputNumber < static_cast<uint16_t>(TOTAL_NUMBER_OF_OUTPUTS)))
    {
        dynamicClientType const dynamicChannel = outputNumber - NUMBER_OF_INTERNAL_OUTPUTS - 1;
        interrupts::SuspendResumeAllInterruptsLock fLock;
        fLock.suspend();
        if (dynamicOutputCfg.setDynamicClient(dynamicChannel, clientOutputNumber, client) == true)
        {
            fLock.resume();
            return bsp::BSP_OK;
        }
        else
        {
            fLock.resume();
            return bsp::BSP_ERROR;
        }
    }
    else
    {
        return bsp::BSP_ERROR;
    }
}

bsp::BspReturnCode Output::clrDynamicClient(uint16_t const outputNumber)
{
    if ((outputNumber > static_cast<uint16_t>(NUMBER_OF_INTERNAL_OUTPUTS))
        && (outputNumber < static_cast<uint16_t>(TOTAL_NUMBER_OF_OUTPUTS)))
    {
        dynamicClientType const dynamicChannel = outputNumber - NUMBER_OF_INTERNAL_OUTPUTS - 1;
        interrupts::SuspendResumeAllInterruptsLock fLock;
        fLock.suspend();
        if (dynamicOutputCfg.clearDynamicClient(dynamicChannel) == true)
        {
            fLock.resume();
            return bsp::BSP_OK;
        }
        else
        {
            fLock.resume();
            return bsp::BSP_ERROR;
        }
    }
    else
    {
        return bsp::BSP_ERROR;
    }
}

void Output::cleanDynamicClients()
{
    interrupts::SuspendResumeAllInterruptsLock fLock;
    fLock.suspend();
    dynamicOutputCfg.cleanDynamicClients();
    fLock.resume();
}

char const* Output::getName(uint16_t const channel)
{
    if (channel < ((sizeof(outputConfigurationStrings)) / (sizeof(char*))))
    {
        return outputConfigurationStrings[channel];
    }
    else
    {
        return ("??? ->");
    }
}

} /* namespace bios */
