// Copyright 2025 BMW AG

#pragma once

#include <cstddef>
#include <cstdint>

namespace bsp
{

/**
 * Base class used to define the generic interface for the uart communication
 */
class UartApi
{
public:
    /**
     * sends out an array of bytes
     * @param data - pointer to the data to be send
     *        length - the number of bytes to be send
     * @return the number of bytes written
     */
    size_t write(uint8_t const* data, size_t length);

    /**
     * reads an array of bytes
     * @param data - pointer to the array where the data will be read
     *        length - the number of bytes to be read
     * @return the of bytes read from the uart interface
     */
    size_t read(uint8_t* data, size_t length);
};

} // namespace bsp
