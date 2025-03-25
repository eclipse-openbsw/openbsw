// Copyright 2024 Accenture.

#pragma once

#include <transport/AbstractTransportLayer.h>

#include <estd/array.h>
#include <estd/functional.h>
#include <platform/estdint.h>

namespace docan
{
/**
 * Class holding all needed parameters for a DoCan transport layer.
 */
class DoCanParameters
{
public:
    /* Constructor.
     *
     * \param waitAllocateTimeout timeout (unit: ms) for waiting for next allocation retry if
     * messages are missing
     *
     * \param waitRxTimeout timeout (unit: ms) for frame reception
     *
     * \param waitTxCallbackTimeout timeout (unit: ms) to wait for the frame transmit callback from
     * the transceiver
     *
     * \param waitFlowControlTimeout timeout (unit: ms) to wait for reception of a flow control
     * frame when sending segmented message
     *
     * \param maxAllocateRetryCount number of retries for allocating a message (for message
     * reception)
     *
     * \param maxFlowControlWaitCount maximum number of consecutive sending of a wait flow control
     * frame
     *
     * \param minSeparationTimeUs (unit: us) minimum separation time to send in flow control frames
     *
     * \param maxBlockSize maximum block size to send in flow control frames
     */
    DoCanParameters(
        ::estd::function<uint32_t(void)> systemUs,
        uint16_t waitAllocateTimeout,
        uint16_t waitRxTimeout,
        uint16_t waitTxCallbackTimeout,
        uint16_t waitFlowControlTimeout,
        uint8_t maxAllocateRetryCount,
        uint8_t maxFlowControlWaitCount,
        uint32_t minSeparationTimeUs,
        uint8_t maxBlockSize);

    /**
     * Get the current system microsecond count
     * \return current microsecond count (us)
     */
    uint32_t nowUs() const;

    /**
     * Get timeout for waiting for next allocation retry if message is missing.
     * \return timeout (ms)
     */
    uint16_t getWaitAllocateTimeout() const;

    /**
     * Get timeout for frame reception.
     * \return timeout (ms)
     */
    uint16_t getWaitRxTimeout() const;

    /**
     * Get timeout for frame transmit callback by the transceiver.
     * \return timeout (ms)
     */
    uint16_t getWaitTxCallbackTimeout() const;

    /**
     * Get timeout to wait for reception of a flow control frame when sending segmented.
     * \return timeout (ms)
     */
    uint16_t getWaitFlowControlTimeout() const;

    /**
     * Get maximum number of retries for allocating a message (for message reception).
     * \return number of retries
     */
    uint8_t getMaxAllocateRetryCount() const;

    /**
     * Get maximum number of sending wait frames when allocating a message (for message reception).
     * \return number of retries
     */
    uint8_t getMaxFlowControlWaitCount() const;

    /**
     * Get the min separation time value to place into continue flow control frames.
     * \return encoded min separation time
     */
    uint8_t getEncodedMinSeparationTime() const;

    /**
     * Set the (already encoded) min separation time value to place into continue flow control
     * frames.
     */
    void setEncodedMinSeparationTime(uint8_t encodedMinSeparationTime);

    /**
     * Get the block size value to place into continue flow control frames.
     * \return block size value
     */
    uint8_t getMaxBlockSize() const;

    /**
     * Set the maximum block size.
     * \param maxBlockSize maximum block size
     */
    void setMaxBlockSize(uint8_t maxBlockSize);

    /**
     * Decode the min separation time. The returned decoded separation time is in microseconds.
     * \param encodedMinSeparationTime encoded min separation time as specified by section 9.6.5.4
     * in ISO 15765-2:2016
     * \return decoded min separation time in microseconds (us).
     */
    static uint32_t decodeMinSeparationTime(uint8_t encodedMinSeparationTime);
    /**
     * Encode a min separation time by applying the encoding scheme as defined in ISO.
     * \param minSeparationTimeUs min separation time in microseconds
     * \return encoded min separation time as specified by section 9.6.5.4 in ISO 15765-2:2016
     */
    static uint8_t encodeMinSeparationTime(uint32_t minSeparationTimeUs);

private:
    ::estd::function<uint32_t(void)> const _systemUs;
    uint16_t _waitAllocateTimeout;
    uint16_t _waitRxTimeout;
    uint16_t _waitTxCallbackTimeout;
    uint16_t _waitFlowControlTimeout;
    uint8_t _encodedMinSeparationTime;
    uint8_t _maxBlockSize;
    uint8_t _maxAllocateRetryCount;
    uint8_t _maxFlowControlWaitCount;
};

/**
 * Inline implementation.
 */

inline DoCanParameters::DoCanParameters(
    ::estd::function<uint32_t(void)> const systemUs,
    uint16_t const waitAllocateTimeout,
    uint16_t const waitRxTimeout,
    uint16_t const waitTxCallbackTimeout,
    uint16_t const waitFlowControlTimeout,
    uint8_t const maxAllocateRetryCount,
    uint8_t const maxFlowControlWaitCount,
    uint32_t const minSeparationTimeUs,
    uint8_t const maxBlockSize)
: _systemUs(systemUs)
, _waitAllocateTimeout(waitAllocateTimeout)
, _waitRxTimeout(waitRxTimeout)
, _waitTxCallbackTimeout(waitTxCallbackTimeout)
, _waitFlowControlTimeout(waitFlowControlTimeout)
, _encodedMinSeparationTime(encodeMinSeparationTime(minSeparationTimeUs))
, _maxBlockSize(maxBlockSize)
, _maxAllocateRetryCount(maxAllocateRetryCount)
, _maxFlowControlWaitCount(maxFlowControlWaitCount)
{
    estd_assert(minSeparationTimeUs < uint32_t(waitAllocateTimeout) * 1000U);
    estd_assert(minSeparationTimeUs < uint32_t(waitRxTimeout) * 1000U);
    estd_assert(minSeparationTimeUs < uint32_t(waitTxCallbackTimeout) * 1000U);
    estd_assert(minSeparationTimeUs < uint32_t(waitFlowControlTimeout) * 1000U);
}

inline uint32_t DoCanParameters::nowUs() const { return _systemUs(); }

inline uint16_t DoCanParameters::getWaitAllocateTimeout() const { return _waitAllocateTimeout; }

inline uint16_t DoCanParameters::getWaitRxTimeout() const { return _waitRxTimeout; }

inline uint16_t DoCanParameters::getWaitTxCallbackTimeout() const { return _waitTxCallbackTimeout; }

inline uint16_t DoCanParameters::getWaitFlowControlTimeout() const
{
    return _waitFlowControlTimeout;
}

inline uint8_t DoCanParameters::getMaxAllocateRetryCount() const { return _maxAllocateRetryCount; }

inline uint8_t DoCanParameters::getMaxFlowControlWaitCount() const
{
    return _maxFlowControlWaitCount;
}

inline uint8_t DoCanParameters::getEncodedMinSeparationTime() const
{
    return _encodedMinSeparationTime;
}

inline void DoCanParameters::setEncodedMinSeparationTime(uint8_t const encodedMinSeparationTime)
{
    _encodedMinSeparationTime = encodedMinSeparationTime;
}

inline uint8_t DoCanParameters::getMaxBlockSize() const { return _maxBlockSize; }

inline void DoCanParameters::setMaxBlockSize(uint8_t const maxBlockSize)
{
    _maxBlockSize = maxBlockSize;
}

inline uint32_t DoCanParameters::decodeMinSeparationTime(uint8_t const encodedMinSeparationTime)
{
    if (encodedMinSeparationTime <= 0x7FU)
    {
        return static_cast<uint32_t>(encodedMinSeparationTime) * 1000U;
    }

    if ((encodedMinSeparationTime >= 0xF1U) && (encodedMinSeparationTime <= 0xF9U))
    {
        return (static_cast<uint32_t>(encodedMinSeparationTime) - 0xF0U) * 100U;
    }

    return 0x7fU * 1000U;
}

inline uint8_t DoCanParameters::encodeMinSeparationTime(uint32_t const minSeparationTimeUs)
{
    // Encode 100-900us STmin values
    if ((minSeparationTimeUs > 0) && (minSeparationTimeUs < 1000U))
    {
        return static_cast<uint8_t>(minSeparationTimeUs / 100U) + 0xF0U;
    }

    // Encode millisecond STmin values
    if (minSeparationTimeUs < (0x7FU * 1000U))
    {
        return static_cast<uint8_t>(minSeparationTimeUs / 1000U);
    }

    // Default to max valid STmin for all other cases
    return 0x7FU;
}

} // namespace docan
