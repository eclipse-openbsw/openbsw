// Copyright 2025 BMW AG

#pragma once

#include <etl/span.h>

#include <cstddef>
#include <cstdint>

namespace bsp
{

/**
 * Base class defining the generic interface for UART communication.
 */
class IUartApi
{
public:
    /**
     * sends out a number of bytes
     * \param data - span of data to be sent
     * \return the number of bytes written
     */
    size_t write(::etl::span<uint8_t const> const data);

    /**
     * reads a number of bytes
     * \param data - the span where the data will be read
     * \return the number of bytes read from the UART interface
     */
    size_t read(::etl::span<uint8_t> data);
};

} // namespace bsp
