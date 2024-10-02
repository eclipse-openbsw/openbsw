// Copyright 2024 Accenture.

/**
 * \ingroup bsp_i2c
 */
#ifndef GUARD_513A9FF8_08E6_4172_B3F4_A143A83C1B80
#define GUARD_513A9FF8_08E6_4172_B3F4_A143A83C1B80

#include "bsp/i2c/I2cCommon.h"

#include <util/types/Enum.h>

#include <estd/functional.h>
#include <estd/slice.h>
#include <estd/uncopyable.h>
#include <platform/estdint.h>

namespace bsp
{
namespace i2c
{
class II2cMaster
{
public:
    using CompletionCallback = ::estd::function<void(I2cReturnCode)>;

    /**
     * Performs a non-blocking send transaction on the I2C bus
     * \param address   The address of the I2C slave on the bus
     * \param data    A pointer to the data which shall be sent. The application has to make sure
     * that the pointer stays valid until callback is called.
     * \param callback  A callback which gets called after the operation finishes.
     * \param sendStop  Defines whether to eventually send a stop condition or not. This can be used
     * to perform repeated start (e.g. write followed by read)
     */
    virtual void write(
        AddressType address,
        ::estd::slice<uint8_t const> data,
        CompletionCallback callback,
        bool sendStop)
        = 0;

    /**
     * Performs a non-blocking read transaction on the I2C bus
     * \param address   The address of the I2C slave on the bus
     * \param data    Pointer where the received data should be written to. The application has to
     * make sure that the pointer stays valid until callback is called.
     * \param callback  A callback which gets called after the operation finishes.
     * \param sendStop  Defines whether to eventually send a stop condition or not. This can be used
     * to perform repeated start.
     */
    virtual void read(
        AddressType address,
        ::estd::slice<uint8_t> data,
        CompletionCallback callback,
        bool sendStop)
        = 0;

protected:
    II2cMaster& operator=(II2cMaster const&) = default;
};

} // namespace i2c
} // namespace bsp

#endif // GUARD_513A9FF8_08E6_4172_B3F4_A143A83C1B80
