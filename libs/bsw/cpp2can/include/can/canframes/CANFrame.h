// Copyright 2024 Accenture.

/**
 * Contains Cpp2CAN CANFrame.
 * \file CANFrame.h
 * \ingroup cpp2can
 */
#ifndef GUARD_47B39A9C_ECE1_49E4_A566_154ED1C7E163
#define GUARD_47B39A9C_ECE1_49E4_A566_154ED1C7E163

#include "can/canframes/CanId.h"

#include <estd/assert.h>
#include <platform/estdint.h>

#include <cstring>

namespace can
{
/**
 * Class representing a CANFrame.
 *
 *
 * A object of type CANFrame does not provide any payload-buffer by default.
 * It has to be provided with buffers by using a certain constructor or the
 * setPayload() method.
 *
 * \note * CANFrame may be used as base class for special frame classes providing
 * the programmer with explicit methods to access the signals that are
 * encoded in the CANFrames payload.
 */
class CANFrame
{
public:
    /** bitmask to extract sender */
    static uint8_t const SENDER_MASK       = 0xFFU;
    /** overhead of a CANFrame (in bit) */
    static uint8_t const CAN_OVERHEAD_BITS = 47U;
    /** maximum payload length of a CANFrame */
#ifdef CPP2CAN_USE_64_BYTE_FRAMES
    static uint8_t const MAX_FRAME_LENGTH = 64U;
#else
    static uint8_t const MAX_FRAME_LENGTH = 8U;
#endif
    /** maximum value of a CANFrame id */
    static uint32_t const MAX_FRAME_ID          = 0x7FFU;
    /** maximum value of a CANFrame id */
    static uint32_t const MAX_FRAME_ID_EXTENDED = 0x1FFFFFFFU;

    /**
     * \post getId() == 0x00
     * \post isExtendedId() == false
     * \post getPayload() == pointer to buffer
     * \post getPayloadLength() == 0
     * \post getTimestamp() == 0
     */
    CANFrame();

    explicit CANFrame(uint32_t id);

    /**
     * Copy constructor
     * \param frame    CANFrame to copy content from
     */
    CANFrame(CANFrame const& frame);

    /**
     * Constructor initializing id, payload and length
     *
     * \param id frame id \see ::can::CanId
     * \pre length <= MAX_FRAME_LENGTH
     * @throws    assertion
     */
    CANFrame(uint32_t id, uint8_t const payload[], uint8_t length);

    /**
     * Constructor initializing raw id with extended flag, payload and length.
     *
     * \pre rawId <= MAX_FRAME_ID_EXTENDED
     * \pre length <= MAX_FRAME_LENGTH
     * @throws    assertion
     */
    CANFrame(uint32_t rawId, uint8_t const payload[], uint8_t length, bool isExtendedId);

    /**
     * \return the identifier of this CANFrame. Use the CanId class to check for normal/extended id.
     */
    uint32_t getId() const { return _id; }

    void setId(uint32_t id);

    /**
     * \return pointer to modifiable payload of this CANFrame
     */
    uint8_t* getPayload() { return _payload; }

    /**
     * \return pointer to read only payload of this CANFrame
     */
    uint8_t const* getPayload() const { return _payload; }

    uint8_t& operator[](uint8_t pos);

    // this const uint8_t& is important because in some bits
    // of the code we take the address of this! Ideally we would
    // just return the uint8_t by value.
    uint8_t const& operator[](uint8_t pos) const;

    /**
     * Sets the CANFrames payload and length
     *
     * \note * This method just sets a pointer to the payload and does not copy it!
     *
     * \post getPayload() == payload
     * \post getPayloadLength() == length
     * \post getMaxPayloadLength() == length
     */
    void setPayload(uint8_t const payload[], uint8_t length);

    /**
     * \pre getPayload() != NULL
     * \pre length <= getMaxPayloadLength()
     * @throws    assertion
     */
    void setPayloadLength(uint8_t length);

    uint8_t getPayloadLength() const { return _payloadLength; }

    static uint8_t getMaxPayloadLength() { return CANFrame::MAX_FRAME_LENGTH; }

    /**
     * Assigns content of a CANFrame to another.
     * \param canFrame    frame to copy from
     * \return reference to frame with new content
     * \pre getMaxPayloadLength() >= canFrame.getPayloadLength()
     * @throws    assertion
     */
    CANFrame& operator=(CANFrame const& canFrame);

    /**
     * \return value of timestamp
     */
    uint32_t timestamp() const { return _timestamp; }

    /**
     * \return read only access to timestamp
     */
    void setTimestamp(uint32_t aTimestamp);

protected:
    friend bool operator==(CANFrame const& frame1, CANFrame const& frame2);

private:
    uint32_t _id;
    uint32_t _timestamp;
    uint8_t _payload[CANFrame::MAX_FRAME_LENGTH];
    uint8_t _payloadLength;
};

/**
 * Compares two CANFrames without considering the timestamp
 * \param frame1    first frame to compare
 * \param frame2    second frame to compare
 * \return *             - true if frames are equal
 *             - false if frames are not equal
 */
bool operator==(CANFrame const& frame1, CANFrame const& frame2);

/*
 * inline
 */

inline void CANFrame::setId(uint32_t const id) { _id = id; }

inline uint8_t& CANFrame::operator[](uint8_t const pos) { return _payload[pos]; }

inline uint8_t const& CANFrame::operator[](uint8_t const pos) const { return _payload[pos]; }

inline void CANFrame::setPayload(uint8_t const* const payload, uint8_t const length)
{
    estd_assert(length <= MAX_FRAME_LENGTH);

    (void)memcpy(_payload, payload, static_cast<size_t>(length));
    _payloadLength = length;
}

inline void CANFrame::setTimestamp(uint32_t const aTimestamp) { _timestamp = aTimestamp; }

} // namespace can

#endif // GUARD_47B39A9C_ECE1_49E4_A566_154ED1C7E163
