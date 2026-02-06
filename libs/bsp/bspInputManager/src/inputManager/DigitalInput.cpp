// Copyright 2024 Accenture.

#include "inputManager/DigitalInput.h"

#include "interrupts/SuspendResumeAllInterruptsLock.h"

#ifdef BSP_INPUT_PIN_CONFIGURATION
#undef BSP_INPUT_PIN_CONFIGURATION
#endif
#define BSP_INPUT_PIN_CONFIGURATION 1

namespace bios
{
#include "bsp/io/input/inputConfiguration.h"
#include "bsp/io/input/inputConfigurationStrings.h"
#if (INPUTDIGITAL_DEBOUNCE_ACTIVE == 1)
DigitalInput::DebounceConfig DigitalInput::debounced[NUMBER_OF_DIGITAL_INPUTS];
#endif

DigitalInput::InputConfiguration const* DigitalInput::sfpDigitalInputConfiguration = nullptr;
dynamicClient<
    DigitalInput::dynamicClientType,
    DigitalInput::IDynamicInputClient,
    4,
    DigitalInput::NumberOfDynamicInputs>
    DigitalInput::dynamicInputCfg;

void DigitalInput::init(uint8_t const hw, bool const doSetup)
{
    sfpDigitalInputConfiguration = getConfiguration(hw);
    if (nullptr == sfpDigitalInputConfiguration)
    {
        return;
    }
    if (doSetup)
    {
        for (uint16_t i = 0; i < NUMBER_OF_INTERNAL_DIGITAL_INPUTS; i++)
        {
            (void)Io::setDefaultConfiguration(
                static_cast<uint16_t>(sfpDigitalInputConfiguration[i].ioNumber));
        }
    }
    cleanDynamicClients();
    cyclic();
}

void DigitalInput::cyclic(void)
{
    if (nullptr == sfpDigitalInputConfiguration)
    {
        return;
    }

#if (INPUTDIGITAL_DEBOUNCE_ACTIVE == 1)
    bool vol;
    for (uint16_t i = 0; i < NUMBER_OF_INTERNAL_DIGITAL_INPUTS; i++)
    {
        vol = Io::getPin(i);

        if (vol)
        {
            if (debounced[i].counter < digitalInputs[i].debounceThreshold)
            {
                debounced[i].counter++;
            }
            else
            {
                debounced[i].vol = 1;
            }
        }
        else
        {
            if (debounced[i].counter > 0)
            {
                debounced[i].counter--;
            }
            else
            {
                debounced[i].vol = 0;
            }
        }
    }
#endif
}

void DigitalInput::shutdown(void) { cleanDynamicClients(); }

bsp::BspReturnCode DigitalInput::get(DigitalInputId const channel, bool& result)
{
    if (nullptr == sfpDigitalInputConfiguration)
    {
        return bsp::BSP_ERROR;
    }

    uint16_t const tmpChannel = static_cast<uint16_t>(channel);

    // Internal channel
    if ((NUMBER_OF_INTERNAL_DIGITAL_INPUTS > 0)
        && (channel <= DigitalInputId::LAST_INTERNAL_DIGITAL_INPUT))
    {
        if (Io::PORT_UNAVAILABLE == sfpDigitalInputConfiguration[tmpChannel].ioNumber)
        {
            return bsp::BSP_NOT_SUPPORTED;
        }
#if (INPUTDIGITAL_DEBOUNCE_ACTIVE == 1)
        result = debounced[channel].vol != 0U;
#else
        result
            = Io::getPin(static_cast<uint16_t>(sfpDigitalInputConfiguration[tmpChannel].ioNumber));
#endif

        if (sfpDigitalInputConfiguration[tmpChannel].isInverted)
        {
            result = !result;
        }
        return bsp::BSP_OK;
    }

    if ((DigitalInputId::LAST_DYNAMIC_DIGITAL_INPUT > DigitalInputId::LAST_INTERNAL_DIGITAL_INPUT)
        && (channel > DigitalInputId::LAST_INTERNAL_DIGITAL_INPUT)
        && (channel <= DigitalInputId::LAST_DYNAMIC_DIGITAL_INPUT))
    {
        // dynamic instance
        dynamicClientType const dynamicChannel
            = static_cast<dynamicClientType>(tmpChannel - NUMBER_OF_INTERNAL_DIGITAL_INPUTS);
        if ((dynamicChannel < NumberOfDynamicInputs)
            && dynamicInputCfg.getClientValid(dynamicChannel))
        {
            return (dynamicInputCfg.getClientInstance(dynamicChannel)
                        ->get(dynamicInputCfg.getChannelInsideClient(dynamicChannel), result));
        }
    }

    return bsp::BSP_ERROR;
}

bsp::BspReturnCode DigitalInput::setDynamicClient(
    uint16_t const inputNumber, uint16_t const clientInputNumber, IDynamicInputClient* const client)
{
    // range of input check
    if ((inputNumber > static_cast<uint16_t>(DigitalInputId::LAST_INTERNAL_DIGITAL_INPUT))
        && (inputNumber <= static_cast<uint16_t>(DigitalInputId::LAST_DYNAMIC_DIGITAL_INPUT)))
    {
        dynamicClientType const dynamicChannel
            = static_cast<dynamicClientType>(inputNumber - NUMBER_OF_INTERNAL_DIGITAL_INPUTS);
        interrupts::SuspendResumeAllInterruptsLock fLock;
        fLock.suspend();
        if (dynamicInputCfg.setDynamicClient(dynamicChannel, clientInputNumber, client))
        {
            fLock.resume();
            return bsp::BSP_OK;
        }

        fLock.resume();
    }

    return bsp::BSP_ERROR;
}

bsp::BspReturnCode DigitalInput::clrDynamicClient(uint16_t const inputNumber)
{
    // range of input check
    if ((inputNumber > static_cast<uint16_t>(DigitalInputId::LAST_INTERNAL_DIGITAL_INPUT))
        && (inputNumber <= static_cast<uint16_t>(DigitalInputId::LAST_DYNAMIC_DIGITAL_INPUT)))
    {
        dynamicClientType const dynamicChannel
            = static_cast<dynamicClientType>(inputNumber - NUMBER_OF_INTERNAL_DIGITAL_INPUTS);
        interrupts::SuspendResumeAllInterruptsLock fLock;
        fLock.suspend();
        if (dynamicInputCfg.clearDynamicClient(dynamicChannel))
        {
            fLock.resume();
            return bsp::BSP_OK;
        }
        fLock.resume();
    }

    return bsp::BSP_ERROR;
}

void DigitalInput::cleanDynamicClients()
{
    interrupts::SuspendResumeAllInterruptsLock fLock;
    fLock.suspend();
    dynamicInputCfg.cleanDynamicClients();
    fLock.resume();
}

char const* DigitalInput::getName(uint16_t const channel)
{
    if (channel < ((sizeof(inputConfigurationStrings)) / (sizeof(char*))))
    {
        return inputConfigurationStrings[channel];
    }

    return ("??? ->");
}

} /* namespace bios */
