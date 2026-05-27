/********************************************************************************
 * Copyright (c) 2025 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include "ip/IPAddress.h"

#include <platform/estdint.h>

namespace udp
{
class AbstractDatagramSocket;

/**
 * Interface for classes that want to receive data from an
 * AbstractDatagramSocket.
 *
 */
class IDataListener
{
protected:
    IDataListener();

public:
    IDataListener(IDataListener const&)            = delete;
    IDataListener& operator=(IDataListener const&) = delete;
    // [IDataListener]
    /**
     * callback that gets called when data has been received
     * \param  length  number of bytes received
     *
     * \note
     * It is expected that the data is read from the AbstractSocket within this
     * callback. The AbstractSocket may overwrite its internal buffers after
     * this callback is done.
     */
    virtual void dataReceived(
        AbstractDatagramSocket& socket,
        ::ip::IPAddress sourceAddress,
        uint16_t sourcePort,
        ::ip::IPAddress destinationAddress,
        uint16_t length)
        = 0;
    // [IDataListener]
};

/*
 * inline methods
 */
inline IDataListener::IDataListener() = default;

} /* namespace udp */
