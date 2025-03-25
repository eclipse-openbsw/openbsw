// Copyright 2024 Accenture.

#pragma once

#include "platform/estdint.h"

/**
 * This data type provides an abstract runtime mapping between a common shared I/O and
 * higher-level instances that can use this I/O.
 *
 * Usage: create an instance of dynamic client inside the class responsible for distribution of the
 * I/O channels that have to be dynamic, and provide public methods for adding and removing the
 * dynamic client.
 *
 * @tparam T - type that contains client's index + index of a channel inside a client.
 * @tparam IDynamicClient - type of an object that should be registered as a dynamic client
 * @tparam Size - bit width for count of clients, number of clients then is 2^Size
 * @tparam dynamicChannelsCount - number of dynamic channels that can be stored in this structure
 * @tparam dynamicClientsCount - number of dynamic clients that use this proxy.
 */
template<
    typename T,
    typename IDynamicClient,
    int32_t Size,
    int32_t dynamicChannelsCount,
    int32_t dynamicClientsCount = 0>
struct dynamicClient
{
    //[Enum_start]

    enum
    {
        bitWidthDynamicClientCount         = Size,
        bitWidthDynamicClientsChannelCount = ((sizeof(T) * 8) - bitWidthDynamicClientCount),
        maxDynamicClientCount              = (0x01U << bitWidthDynamicClientCount),
        channelInvalid                     = (0x01U << bitWidthDynamicClientsChannelCount) - 1U,
        dynamicClientInvalid               = (0x01U << bitWidthDynamicClientCount) - 1U,
        dynamicChannels                    = dynamicChannelsCount
    };

    //[Enum_end]

    /**
     * If dynamicClientsCount is not explicitly specified, maximum possible dynamicClientsCount is
     * selected.
     */
    static T const dynamicClientCountCfg
        = (dynamicClientsCount == 0 ? static_cast<int32_t>(maxDynamicClientCount)
                                    : dynamicClientsCount);

    /**
     * This structure is responsible for distributing number of clients and number of
     * instances per client.
     * If T is 8 bit, then dynamicClient can devote N bits to instances' indexing and (8-N) bits to
     * indexing inside a single client.
     * Example:
     * T = uint8_t, Size = 2 --> instanceNumber is in [0..3], numberInsideClient is in [0..31]
     */
    using tDynamicClientCfg = struct
    {
        T numberInsideClient : bitWidthDynamicClientsChannelCount;
        T instanceNumber : bitWidthDynamicClientCount;
    };

    // METHOD_START
    /**
     * Clean all Clients
     */
    static void cleanDynamicClients();

    /**
     * Check Valid Channel
     * @param : number of channel
     * @ret   : true is valid ,false - not valid
     */
    static bool getClientValid(T channel);

    /**
     * setDynamicClient
     * @param : channel number ,
     * @param : numberInsideClient , channel number inside the Client
     * @param : IDynamicClient* client
     * @ret   : true , false
     */
    bool setDynamicClient(T channel, T numberInsideClient, IDynamicClient* client);

    /**
     * clearDynamicClient
     * @param : channel number ,
     * @ret   : true , false
     */
    bool clearDynamicClient(T channel);

    /**
     * getClientInstance, channel has to be first checked using `getClientValid()` method
     * @param : number of channel
     * @ret   : IDynamicClient*
     */
    static inline IDynamicClient* getClientInstance(T channel)
    {
        return (fpDynamicClient[fDynamicChannelCfg[channel].instanceNumber]);
    }

    static inline T getChannelInsideClient(T channel)
    {
        return (fDynamicChannelCfg[channel].numberInsideClient);
    }

    static tDynamicClientCfg fDynamicChannelCfg[dynamicChannels];
    static IDynamicClient* fpDynamicClient[dynamicClientCountCfg];

    // METHOD_END
};

template<
    typename T,
    typename IDynamicClient,
    int32_t Size,
    int32_t dynamicChannelsCount,
    int32_t dynamicClientCount>
typename dynamicClient<T, IDynamicClient, Size, dynamicChannelsCount, dynamicClientCount>::
    tDynamicClientCfg
        dynamicClient<T, IDynamicClient, Size, dynamicChannelsCount, dynamicClientCount>::
            fDynamicChannelCfg[dynamicChannels];

template<
    typename T,
    typename IDynamicClient,
    int32_t Size,
    int32_t dynamicChannels,
    int32_t dynamicClientCount>
IDynamicClient* dynamicClient<T, IDynamicClient, Size, dynamicChannels, dynamicClientCount>::
    fpDynamicClient[dynamicClientCountCfg];

template<
    typename T,
    typename IDynamicClient,
    int32_t Size,
    int32_t dynamicChannelsCount,
    int32_t dynamicClientCount>
void dynamicClient<T, IDynamicClient, Size, dynamicChannelsCount, dynamicClientCount>::
    cleanDynamicClients()
{
    for (int32_t i = 0; i < (dynamicClientCountCfg); i++)
    {
        fpDynamicClient[i] = nullptr;
    }
    for (int32_t i = 0; i < (dynamicChannels); i++)
    {
        fDynamicChannelCfg[i].numberInsideClient = channelInvalid;
        fDynamicChannelCfg[i].instanceNumber     = dynamicClientInvalid;
    }
}

template<
    typename T,
    typename IDynamicClient,
    int32_t Size,
    int32_t dynamicChannelsCount,
    int32_t dynamicClientCount>
bool dynamicClient<T, IDynamicClient, Size, dynamicChannelsCount, dynamicClientCount>::
    getClientValid(T channel)
{
    bool ret = false;
    if (channel < dynamicChannels)
    {
        T instanceNumber = fDynamicChannelCfg[channel].instanceNumber;
        if (instanceNumber < dynamicClientCountCfg)
        {
            if ((instanceNumber < dynamicClientInvalid)
                && (fDynamicChannelCfg[channel].numberInsideClient < channelInvalid)
                && (fpDynamicClient[instanceNumber] != nullptr))
            {
                ret = true;
            }
        }
    }

    return ret;
}

template<
    typename T,
    typename IDynamicClient,
    int32_t Size,
    int32_t dynamicChannelsCount,
    int32_t dynamicClientCount>
bool dynamicClient<T, IDynamicClient, Size, dynamicChannelsCount, dynamicClientCount>::
    setDynamicClient(T channel, T numberInsideClient, IDynamicClient* client)
{
    // range of outputNumber check
    if (numberInsideClient >= channelInvalid)
    {
        return false;
    }

    if (channel < dynamicChannels) // number is out of range
    {
        uint8_t mySlot          = 0xFFU;
        uint8_t availableClient = 0xFFU;
        for (T i = 0; (i < dynamicClientCountCfg) && (availableClient == 0xFFU); i++)
        {
            if (client == fpDynamicClient[i])
            {
                availableClient = static_cast<uint8_t>(i);
            }
            if (fpDynamicClient[i] == nullptr)
            {
                if (mySlot == 0xFFU)
                {
                    mySlot = static_cast<uint8_t>(i);
                }
            }
        }
        // Occupy slot for client
        if (availableClient == 0xFFU) // not in Map yet
        {
            if (mySlot != 0xFFU) // placed
            {
                fpDynamicClient[mySlot] = client;
            }
            else // no free place
            {
                return false;
            }
        }
        else
        {
            mySlot = availableClient;
        }

        fDynamicChannelCfg[channel].numberInsideClient = numberInsideClient;
        fDynamicChannelCfg[channel].instanceNumber     = mySlot;
        return true;
    }
    else
    {
        return false;
    }
}

template<
    typename T,
    typename IDynamicClient,
    int32_t Size,
    int32_t dynamicChannelsCount,
    int32_t dynamicClientCount>
bool dynamicClient<T, IDynamicClient, Size, dynamicChannelsCount, dynamicClientCount>::
    clearDynamicClient(T channel)
{
    if (channel < dynamicChannels) // number is out of range
    {
        T instanceNumber       = fDynamicChannelCfg[channel].instanceNumber;
        IDynamicClient* client = fpDynamicClient[instanceNumber];

        fDynamicChannelCfg[channel].numberInsideClient = channelInvalid;
        fDynamicChannelCfg[channel].instanceNumber     = dynamicClientInvalid;
        // look for other outputs

        if ((client == nullptr) || (instanceNumber >= dynamicClientCountCfg))
        {
            return true;
        }

        uint8_t availableClient = 0xFFU;
        for (T i = 0; i < dynamicClientCountCfg; i++)
        {
            if (client == fpDynamicClient[i])
            {
                availableClient = i; // Found a client
                break;
            }
        }
        if (availableClient != 0xFFU)
        {
            bool found = false;
            for (T i = 0; i < dynamicChannels; i++)
            {
                if (fDynamicChannelCfg[i].instanceNumber == availableClient)
                {
                    found = true;
                    break;
                }
            }
            if (!found)
            {
                // No client was found, the link can be deleted.
                fpDynamicClient[availableClient] = nullptr;
            }
        }

        return true;
    }
    else
    {
        return false;
    }
}

