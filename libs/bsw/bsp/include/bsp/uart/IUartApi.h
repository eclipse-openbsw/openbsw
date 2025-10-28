// Copyright 2025 BMW AG

#pragma once

#include <etl/span.h>

#include <cstddef>
#include <cstdint>

namespace bsp
{

/**
 * Base class used to define the generic interface for the uart communication
 */
class IUartApi
{
public:
    /**
     * sends out an array of bytes
     * @param data - pointer to the data to be send
     *        length - the number of bytes to be send
     * @return the number of bytes written
     */
    size_t write(::etl::span<uint8_t const> const data);

    /**
     * reads an array of bytes
     * @param data - pointer to the array where the data will be read
     *        length - the number of bytes to be read
     * @return the of bytes read from the uart interface
     */
    size_t read(::etl::span<uint8_t> data);
};

} // namespace bsp
