// Copyright 2025 BMW AG

#pragma once

#include "bsp/UartImpl.h"

#include <cstddef>
#include <cstdint>

namespace bsp
{

/**
 * Base class used to define the generic interface for the uart communication
 */
class Uart : public UartImpl
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

    /**
     * factory method which instantiates and configures an UART object.
     * If the object exists it will returns only a reference to it.
     * @param id: TERMINAL, ...
     */
    static Uart& getInstance(Id id);

private:
    Uart(Id id) : UartImpl(id) {}
};

} // namespace bsp
