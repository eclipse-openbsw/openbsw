// Copyright 2024 Accenture.

/**
 * \ingroup     transport
 */
#pragma once

#include <etl/span.h>

#include <cstdint>

namespace transport
{
class IDataProgressListener;

/**
 * A TransportMessage is a bus-independent message that can take a certain
 * amount of data.
 *
 * \note
 * A TransportMessage does not provide any data by itself. It must be
 * initialized by calling the method init and providing it with a pointer to a
 * buffer and the length of the buffer.
 *
 * \attention
 * The TransportMessage stores some additional information like length, source
 * and target within the provided buffer. Thus the maximum payload length the
 * TransportMessage can take is smaller than the provided buffersize!
 *
 * \par Memory Layout
 * This is the memory layout of a TransportMessage buffer: *
 * <table border=1 bordercolor="black">
 * <tr>
 *   <th>Byte</th><td>0</td><td>1</td><td>2</td><td>3</td><td>4</td><td>5</td><td>6</td><td>7</td><td>8</td><td>9
 * - bufferlength</td>
 * </tr>
 * <tr>
 *   <th>Value</th><td colspan="4">length</td><td colspan="2">control
 * word</td><td>source id</td><td>target id</td><td>service
 * id</td><td>payload</td>
 * </tr>
 * </table>
 *
 * \par Example
 * An example of a TransportMessage with bufferlength 20
 * <table border=1 bordercolor="black">
 * <tr>
 *   <th>Byte</th><td>0</td><td>1</td><td>2</td><td>3</td><td>4</td><td>5</td><td>6</td><td>7</td><td>8</td><td>9
 * - 19</td>
 * </tr>
 * <tr>
 *   <th>Value</th><td colspan="4">14</td
 * colspan="2"><td>0x0000</td><td>0xF1</td><td>0xAA</td><td>0x31</td><td>payload</td>
 * </tr>
 * </table>
 *
 * \note
 * The service id is already part of the actual payload of the TransportMessage.
 * So a message with just a service id has payload length 1.
 * Don't be confused by the length value stored in the first two bytes of the
 * buffer. It is for internal purposes and for transmission over ethernet.
 */
class TransportMessage
{
public:
    /**
     * TransportMessage related error codes
     * \enum ErrorCode
     */
    enum class ErrorCode : uint8_t
    {
        /** everything OK */
        TP_MSG_OK,
        /** length of TransportMessage was exceeded */
        TP_MSG_LENGTH_EXCEEDED
    };

    /** index to service id from DATA_OFFSET */
    static uint8_t const SERVICE_ID_INDEX = 0U;
    /** offset to actual payload of the TransportMessage */

    static uint16_t const INVALID_ADDRESS = 0xFFFFU;
    /**
     * Default constructor which initializes buffer and length to NULL
     *
     * \warning
     * An instance on TransportMessage must not be used before calling
     * init() and initializing it with a valid buffer.
     */
    TransportMessage();

    TransportMessage(uint8_t buffer[], uint32_t bufferLength);

    /**
     * Provides the TransportMessage with a buffer to work with.
     * \param buffer        pointer to a memory area for this TransportMessage
     * \param bufferLength  length of buffer
     *
     */
    void init(uint8_t buffer[], uint32_t bufferLength);

    /**
     * Returns the source id of the TransportMessage.
     * \pre   fpBuffer != NULL
     * \deprecated
     */
    uint16_t getSourceId() const;

    uint16_t sourceAddress() const;

    /**
     * \pre   fpBuffer != NULL
     */
    void setSourceAddress(uint16_t sourceAddress);

    /**
     * Returns the target id of the TransportMessage.
     * \pre   fpBuffer != NULL
     * \deprecated
     */
    uint16_t getTargetId() const;

    uint16_t targetAddress() const;

    /**
     * \pre   fpBuffer != NULL
     */
    void setTargetAddress(uint16_t targetId);

    /**
     * Returns the service id of the TransportMessage.
     * \pre    fpBuffer != NULL
     * \note
     * The service id is the first byte of the actual payload.
     * \deprecated
     */
    uint8_t getServiceId() const;

    uint8_t serviceId() const;

    /**
     * \pre     fpBuffer != NULL
     * \post    getValidBytes >= 1
     */
    void setServiceId(uint8_t theServiceId);

    /**
     * Returns a pointer to the internal buffer of the TransportMessage.
     */
    uint8_t* getBuffer() const;

    uint32_t getBufferLength() const;

    /**
     * Returns a pointer to the actual payload of the TransportMessage, i.e.
     * service id,...
     * \pre    fpBuffer != NULL
     */
    uint8_t* getPayload();

    /**
     * Read only version of getPayload(), which is used if a TransportMessage
     * is passed as const TransportMessage& parameter.
     * \see getPayload()
     */
    uint8_t const* getPayload() const;

    uint8_t& operator[](uint16_t pos);

    uint8_t const& operator[](uint16_t pos) const;

    /**
     * Returns the length of the TransportMessages payload including service id
     * \pre    fpBuffer != NULL
     * \deprecated
     */
    uint16_t getPayloadLength() const;

    uint16_t payloadLength() const;

    /**
     * \param   length number of bytes that are stored in the TransportMessage
     *          without source and target, i.e. payload including serviceId
     * \pre     length is smaller than getMaxPayloadLength()!
     */
    void setPayloadLength(uint16_t length);

    /**
     * Returns the maximum number of bytes the can be stored in the
     * TransportMessages buffer.
     * \deprecated
     */
    uint16_t getMaxPayloadLength() const;

    uint16_t maxPayloadLength() const;

    /**
     * Appends an amount of data to the payload of the TransportMessage.
     * \param   data   pointer to data to append
     * \param   length number of bytes to append
     * \return
     *          - TP_MSG_OK if data was correctly appended
     *          - TP_MSG_LENGTH_EXCEEDED if data was too long
     *
     * The TransportMessage has internal counter _validBytes which is
     * initialized with 0. Appending data will copy the data to the index
     * _validBytes in the payload of the TransportMessage.
     */
    ErrorCode append(uint8_t const data[], uint16_t length);

    /**
     * Appends one byte to the payload of the TransportMessage.
     * \param   data byte to append
     * \return
     *          - TP_MSG_OK if data was correctly appended
     *          - TP_MSG_LENGTH_EXCEEDED if data was too long
     *
     * _validBytes will be increased by one if one more byte fits into the
     * buffer.
     */
    ErrorCode append(uint8_t data);

    /**
     * Resets _validBytes to 0.
     */
    void resetValidBytes();

    /**
     * Increases _validBytes by n.
     * \param   n value to increase _validBytes by
     * \return
     *          - TP_MSG_OK if valid bytes was increased
     *          - TP_MSG_LENGTH_EXCEEDED if valid bytes would have been too
     * large (valid bytes are set to getMaxPayloadLength()
     */
    ErrorCode increaseValidBytes(uint16_t n);

    /**
     * Returns the number of valid bytes in the payload of this
     * TransportMessage. \deprecated
     */
    uint16_t getValidBytes() const;

    uint16_t validBytes() const;

    /**
     * Returns the number of bytes missing until message is complete.
     */
    uint16_t missingBytes() const;

    /**
     * \return
     *          - true if TransportMessage is complete
     *          - false else
     *
     * This method is used if you know the final length of the TransportMessages
     * payload but data comes in in smaller packages and are appended in pieces.
     */
    bool isComplete() const;

    void setDataProgressListener(IDataProgressListener& listener);

    bool isDataProgressListener(IDataProgressListener const& listener) const;

    void removeDataProgressListener();

    /**
     * Compares two TransportMessages
     * \param   rhs TransportMessage to compare to
     * \return
     *          - true if TransportMessages are equal
     *          - false else
     */
    bool operator==(TransportMessage const& rhs) const;

private:
    /** Pointer to optional IDataProgressListener */
    IDataProgressListener* _dataProgressListener;

    /* internal buffer */
    ::etl::span<uint8_t> _buffer;

    /** source id of TransportMessage */
    uint16_t _sourceAddress;

    /** target id of TransportMessage */
    uint16_t _targetAddress;

    /** total length of payload in bytes */
    uint16_t _payloadLength;

    /** Current number of valid bytes in payload */
    uint16_t _validBytes;

private:
    void notifyDataProgressListener(uint32_t numberOfNewValidBytes);
};

/*
 *
 * inline implementation
 *
 */

inline uint16_t TransportMessage::getSourceId() const { return sourceAddress(); }

inline void TransportMessage::setSourceAddress(uint16_t const sourceAddress)
{
    _sourceAddress = sourceAddress;
}

inline uint16_t TransportMessage::sourceAddress() const { return _sourceAddress; }

inline uint16_t TransportMessage::getTargetId() const { return targetAddress(); }

inline uint16_t TransportMessage::targetAddress() const { return _targetAddress; }

inline void TransportMessage::setTargetAddress(uint16_t const targetId)
{
    _targetAddress = targetId;
}

inline uint8_t TransportMessage::getServiceId() const { return serviceId(); }

inline uint8_t TransportMessage::serviceId() const { return _buffer[SERVICE_ID_INDEX]; }

inline uint8_t* TransportMessage::getBuffer() const { return _buffer.data(); }

inline uint32_t TransportMessage::getBufferLength() const
{
    return static_cast<uint32_t>(_buffer.size());
}

inline uint8_t* TransportMessage::getPayload() { return _buffer.data(); }

inline uint8_t const* TransportMessage::getPayload() const { return _buffer.data(); }

inline uint8_t& TransportMessage::operator[](uint16_t const pos)
{
    return _buffer[static_cast<size_t>(pos)];
}

inline uint8_t const& TransportMessage::operator[](uint16_t const pos) const
{
    return _buffer[static_cast<size_t>(pos)];
}

inline uint16_t TransportMessage::getPayloadLength() const { return payloadLength(); }

inline uint16_t TransportMessage::payloadLength() const { return _payloadLength; }

inline uint16_t TransportMessage::getMaxPayloadLength() const { return maxPayloadLength(); }

inline uint16_t TransportMessage::maxPayloadLength() const
{
    return static_cast<uint16_t>(_buffer.size());
}

inline void TransportMessage::resetValidBytes() { _validBytes = 0U; }

inline uint16_t TransportMessage::getValidBytes() const { return validBytes(); }

inline uint16_t TransportMessage::validBytes() const { return _validBytes; }

inline bool TransportMessage::isComplete() const { return _validBytes >= getPayloadLength(); }

inline uint16_t TransportMessage::missingBytes() const
{
    // validBytes() is always less or equal than payloadLength()
    return payloadLength() - validBytes();
}

} // namespace transport
