// Copyright 2024 Accenture.

#include "outputPwm/OutputPwm.h"

#include <interrupts/SuspendResumeAllInterruptsLock.h>
#include <io/Io.h>

namespace bios
{

#undef BSPOUTPUTPWMCONFIG
#define BSPOUTPUTPWMCONFIG 1
#include <bsp/io/outputPwm/outputPwmConfiguration.h>
#include <bsp/io/outputPwm/outputPwmConfigurationStrings.h>
#undef BSPOUTPUTPWMCONFIG

dynamicClient<
    OutputPwm::dynamicClientType,
    OutputPwm::IDynamicPwmClient,
    4,
    OutputPwm::outputNumberDynamic>
    OutputPwm::dynamicPwmOutputCfg;

void OutputPwm::init(uint8_t /* hwVariant */) { cleanDynamicClients(); }

void OutputPwm::shutdown() { cleanDynamicClients(); }

bsp::BspReturnCode OutputPwm::setDuty(uint16_t chan, uint16_t duty, bool immediateUpdate)
{
    if (chan < outputPwmStaticNumber)
    {
        // dummy channel
        return bsp::BSP_OK;
    }

    if (chan < outputPwmNumberAll)
    {
        dynamicClientType channel = chan - outputPwmStaticNumber; // dynamic instance
        if (channel < outputNumberDynamic)
        {
            if (dynamicPwmOutputCfg.getClientValid(channel))
            {
                return (dynamicPwmOutputCfg.getClientInstance(channel)->setDuty(
                    dynamicPwmOutputCfg.getChannelInsideClient(channel), duty, immediateUpdate));
            }
        }
    }

    return bsp::BSP_ERROR;
}

bsp::BspReturnCode OutputPwm::setPeriod(uint16_t chan, uint16_t period)
{
    if (chan < outputPwmStaticNumber)
    {
        // dummy channel
        return bsp::BSP_OK;
    }

    if (chan < outputPwmNumberAll)
    {
        dynamicClientType channel = chan - outputPwmStaticNumber; // dynamic instance
        if ((channel < outputNumberDynamic) && dynamicPwmOutputCfg.getClientValid(channel))
        {
            return (dynamicPwmOutputCfg.getClientInstance(channel)->setPeriod(
                dynamicPwmOutputCfg.getChannelInsideClient(channel), period));
        }
    }

    return bsp::BSP_ERROR;
}

bsp::BspReturnCode OutputPwm::setDynamicClient(
    uint16_t outputNumber, uint16_t clientOutputNumber, IDynamicPwmClient* client)
{
    // range of outputNumber check
    if ((outputNumber >= _pwmDynamicStart) && (outputNumber < outputPwmNumberAll))
    {
        dynamicClientType dynamicChannel = outputNumber - _pwmDynamicStart;
        interrupts::SuspendResumeAllInterruptsLock fLock;
        fLock.suspend();
        if (dynamicPwmOutputCfg.setDynamicClient(dynamicChannel, clientOutputNumber, client))
        {
            fLock.resume();
            return bsp::BSP_OK;
        }
        fLock.resume();
    }

    return bsp::BSP_ERROR;
}

bsp::BspReturnCode OutputPwm::clrDynamicClient(uint16_t outputNumber)
{
    // range of outputNumber check
    if ((outputNumber >= _pwmDynamicStart) && (outputNumber < outputPwmNumberAll))
    {
        dynamicClientType dynamicChannel = outputNumber - _pwmDynamicStart;
        interrupts::SuspendResumeAllInterruptsLock fLock;
        fLock.suspend();
        if (dynamicPwmOutputCfg.clearDynamicClient(dynamicChannel))
        {
            fLock.resume();
            return bsp::BSP_OK;
        }
        fLock.resume();
    }

    return bsp::BSP_ERROR;
}

void OutputPwm::cleanDynamicClients()
{
    interrupts::SuspendResumeAllInterruptsLock fLock;
    fLock.suspend();
    dynamicPwmOutputCfg.cleanDynamicClients();
    fLock.resume();
}

char const* OutputPwm::getName(uint16_t channel)
{
    if (channel < sizeof(outputPwmConfigurationStrings) / (sizeof(char*)))
    {
        return outputPwmConfigurationStrings[channel];
    }
    return ("??? ->");
}

} // namespace bios
