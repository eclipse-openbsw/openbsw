/********************************************************************************
 * Copyright (c) 2026 Roland Reichwein
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include <bsp/uart/UartConcept.h>

namespace bsp
{
/**
 * This class implements the UART communication for the RP2040 platform.
 * It follows the method signatures defined in the UartConcept.h file.
 */
class Uart
{
public:
    /*
     * Enum for identifying different UART instances (e.g., TERMINAL).
     */
    enum class Id : size_t;

    /**
     * Sends out a number of bytes over the UART interface.
     * The method will block until the data is sent.
     * \param data - span of data to be sent
     * \return the number of bytes written to the uart interface
     */
    size_t write(::etl::span<uint8_t const> const data);

    /**
     * Reads the bytes that are currently available from the UART interface.
     * The method does not block waiting for data to arrive.
     * \param data - the span where the data will be read
     * \return the number of bytes read from the uart interface
     */
    size_t read(::etl::span<uint8_t> data);

    /**
     * Configures and starts the UART communication.
     * This method must be called before using the read/write methods.
     */
    void init();

    /**
     * Returns if this Uart instance is initialized or not.
     */
    bool isInitialized() const;

    /**
     * Waits until the UART is ready to transmit data.
     * \return true if the Uart is ready for transmission, false otherwise
     */
    bool waitForTxReady();

    /**
     * Returns the singleton instance of the Uart object.
     * \param id: TERMINAL, ...
     */
    static Uart& getInstance(Id id);

    explicit Uart(Id id);

private:
    Id _id;
    bool _initialized = false;
};

BSP_UART_CONCEPT_CHECKER(Uart)

} // namespace bsp

#include "bsp/uart/UartConfig.h"
