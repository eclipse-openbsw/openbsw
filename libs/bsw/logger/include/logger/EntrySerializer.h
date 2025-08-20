// Copyright 2024 Accenture.

#pragma once

#include <etl/span.h>
#include <etl/uncopyable.h>
#include <util/format/IPrintfArgumentReader.h>
#include <util/format/PrintfArgumentReader.h>
#include <util/format/PrintfFormatScanner.h>
#include <util/logger/Logger.h>

#include <cstring>

namespace logger
{
class SectionPredicate;

template<class Timestamp = uint32_t>
class IEntrySerializerCallback : private ::etl::uncopyable
{
public:
    IEntrySerializerCallback();

    virtual void onEntry(
        Timestamp timestamp,
        uint8_t componentIndex,
        ::util::logger::Level level,
        char const* str,
        ::util::format::IPrintfArgumentReader& argReader)
        = 0;
};

template<class T = uint16_t, class Timestamp = uint32_t, class ReadOnlyPredicate = SectionPredicate>
class EntrySerializer
{
public:
    explicit EntrySerializer(ReadOnlyPredicate readOnlyPredicate = ReadOnlyPredicate());

    T serialize(
        ::etl::span<uint8_t> const& destBuffer,
        Timestamp timestamp,
        uint8_t componentIndex,
        ::util::logger::Level level,
        char const* formatString,
        va_list ap) const;
    static void deserialize(
        ::etl::span<uint8_t const> const& srcBuffer, IEntrySerializerCallback<Timestamp>& callback);

private:
    enum
    {
        DATATYPE_UINT8        = static_cast<uint8_t>(::util::format::ParamDatatype::UINT8),
        DATATYPE_SINT16       = static_cast<uint8_t>(::util::format::ParamDatatype::SINT16),
        DATATYPE_UINT16       = static_cast<uint8_t>(::util::format::ParamDatatype::UINT16),
        DATATYPE_SINT32       = static_cast<uint8_t>(::util::format::ParamDatatype::SINT32),
        DATATYPE_UINT32       = static_cast<uint8_t>(::util::format::ParamDatatype::UINT32),
        DATATYPE_SINT64       = static_cast<uint8_t>(::util::format::ParamDatatype::SINT64),
        DATATYPE_UINT64       = static_cast<uint8_t>(::util::format::ParamDatatype::UINT64),
        DATATYPE_VOIDPTR      = static_cast<uint8_t>(::util::format::ParamDatatype::VOIDPTR),
        DATATYPE_CHARPTR      = static_cast<uint8_t>(::util::format::ParamDatatype::CHARPTR),
        DATATYPE_SIZEDCHARPTR = static_cast<uint8_t>(::util::format::ParamDatatype::SIZEDCHARPTR),
        DATATYPE_CHARARRAY    = static_cast<uint8_t>(::util::format::ParamDatatype::COUNT),
        DATATYPE_NONE         = 0xff
    };

    class EntryWriter
    {
    public:
        EntryWriter(
            uint8_t* bufferStart, uint8_t* bufferEnd, ReadOnlyPredicate const& readOnlyPredicate);
        T getSize() const;

        void copyParamVariant(
            ::util::format::PrintfArgumentReader& reader, ::util::format::ParamDatatype datatype);
        void writeParamVariant(
            ::util::format::ParamDatatype datatype, ::util::format::ParamVariant const& variant);
        void writeString(char const* source);
        void writeString(uint8_t typeByte, char const* source, T length);
        void writeData(uint8_t datatype, void const* source, T size);
        void writeBytes(void const* source, T size);

    private:
        uint8_t* _bufferStart;
        uint8_t* _bufferEnd;
        uint8_t* _write;
        ReadOnlyPredicate const& _parentPredicate;
    };

    class EntryReader : public ::util::format::IPrintfArgumentReader
    {
    public:
        EntryReader(uint8_t const* bufferStart, uint8_t const* bufferEnd);

        ::util::format::ParamVariant const*
        readArgument(::util::format::ParamDatatype datatype) override;

        void readBytes(void* dest, T size);

    private:
        void readParamVariant();
        char const* readCharArray();

    private:
        uint8_t const* _bufferStart;
        uint8_t const* _bufferEnd;
        uint8_t const* _read;
        ::util::format::ParamVariant _variant;
        ::util::string::PlainSizedString _plainSizedString;
    };

    ReadOnlyPredicate _readOnlyPredicate;
};

class SectionPredicate
{
public:
    explicit SectionPredicate(void const* sectionStart = nullptr, void const* sectionEnd = nullptr);

    inline bool operator()(void const* p) const;

private:
    void const* _sectionStart;
    void const* _sectionEnd;
};

template<class T, class Timestamp, class ReadOnlyPredicate>
EntrySerializer<T, Timestamp, ReadOnlyPredicate>::EntrySerializer(
    ReadOnlyPredicate const readOnlyPredicate)
: _readOnlyPredicate(readOnlyPredicate)
{}

template<class T, class Timestamp, class ReadOnlyPredicate>
T EntrySerializer<T, Timestamp, ReadOnlyPredicate>::serialize(
    ::etl::span<uint8_t> const& destBuffer,
    Timestamp const timestamp,
    uint8_t const componentIndex,
    ::util::logger::Level const level,
    char const* const formatString,
    va_list ap) const
{
    using ::util::format::PrintfArgumentReader;
    using ::util::format::PrintfFormatScanner;
    uint8_t* const dest = destBuffer.data();

    EntryWriter writer(dest, dest + destBuffer.size(), _readOnlyPredicate);
    writer.writeBytes(&timestamp, static_cast<T>(sizeof(timestamp)));
    writer.writeBytes(&componentIndex, static_cast<T>(sizeof(componentIndex)));
    uint8_t const levelByte = static_cast<uint8_t>(level);
    writer.writeBytes(&levelByte, static_cast<T>(sizeof(levelByte)));
    writer.writeString(formatString);
    PrintfArgumentReader argReader(ap);

    for (PrintfFormatScanner scanner(formatString); scanner.hasToken(); scanner.nextToken())
    {
        if (scanner.getTokenType() == ::util::format::TokenType::PARAM)
        {
            if (scanner.getParamInfo()._width == ::util::format::ParamWidthOrPrecision::PARAM)
            {
                writer.copyParamVariant(argReader, ::util::format::ParamDatatype::SINT32);
            }
            if (scanner.getParamInfo()._precision == ::util::format::ParamWidthOrPrecision::PARAM)
            {
                writer.copyParamVariant(argReader, ::util::format::ParamDatatype::SINT32);
            }

            writer.copyParamVariant(argReader, scanner.getParamInfo()._datatype);
        }
    }
    return writer.getSize();
}

template<class T, class Timestamp, class ReadOnlyPredicate>
void EntrySerializer<T, Timestamp, ReadOnlyPredicate>::deserialize(
    ::etl::span<uint8_t const> const& srcBuffer, IEntrySerializerCallback<Timestamp>& callback)
{
    EntryReader reader(srcBuffer.data(), srcBuffer.data() + srcBuffer.size());
    Timestamp timestamp;
    uint8_t componentIdx;
    uint8_t level;
    reader.readBytes(&timestamp, static_cast<T>(sizeof(timestamp)));
    reader.readBytes(&componentIdx, static_cast<T>(sizeof(componentIdx)));
    reader.readBytes(&level, static_cast<T>(sizeof(level)));
    ::util::format::ParamVariant const* const formatArg
        = reader.readArgument(::util::format::ParamDatatype::CHARPTR);
    if (formatArg != nullptr)
    {
        callback.onEntry(
            timestamp,
            componentIdx,
            static_cast<::util::logger::Level>(level),
            formatArg->_charPtrValue,
            reader);
    }
}

template<class T, class Timestamp, class ReadOnlyPredicate>
EntrySerializer<T, Timestamp, ReadOnlyPredicate>::EntryWriter::EntryWriter(
    uint8_t* const bufferStart,
    uint8_t* const bufferEnd,
    ReadOnlyPredicate const& readOnlyPredicate)
: _bufferStart(bufferStart)
, _bufferEnd(bufferEnd)
, _write(bufferStart)
, _parentPredicate(readOnlyPredicate)
{}

template<class T, class Timestamp, class ReadOnlyPredicate>
T EntrySerializer<T, Timestamp, ReadOnlyPredicate>::EntryWriter::getSize() const
{
    return static_cast<T>(_write - _bufferStart);
}

template<class T, class Timestamp, class ReadOnlyPredicate>
void EntrySerializer<T, Timestamp, ReadOnlyPredicate>::EntryWriter::copyParamVariant(
    ::util::format::PrintfArgumentReader& reader, ::util::format::ParamDatatype const datatype)
{
    writeParamVariant(datatype, *reader.readArgument(datatype));
}

template<class T, class Timestamp, class ReadOnlyPredicate>
void EntrySerializer<T, Timestamp, ReadOnlyPredicate>::EntryWriter::writeParamVariant(
    ::util::format::ParamDatatype const datatype, ::util::format::ParamVariant const& variant)
{
    switch (datatype)
    {
        case ::util::format::ParamDatatype::UINT8:
        {
            writeData(
                static_cast<uint8_t>(datatype),
                &variant._uint8Value,
                static_cast<T>(sizeof(variant._uint8Value)));
            break;
        }
        case ::util::format::ParamDatatype::SINT16:
        case ::util::format::ParamDatatype::UINT16:
        {
            writeData(
                static_cast<uint8_t>(datatype),
                &variant._uint16Value,
                static_cast<T>(sizeof(variant._uint16Value)));
            break;
        }
        case ::util::format::ParamDatatype::SINT32:
        case ::util::format::ParamDatatype::UINT32:
        {
            writeData(
                static_cast<uint8_t>(datatype),
                &variant._uint32Value,
                static_cast<T>(sizeof(variant._uint32Value)));
            break;
        }
        case ::util::format::ParamDatatype::SINT64:
        case ::util::format::ParamDatatype::UINT64:
        {
            writeData(
                static_cast<uint8_t>(datatype),
                &variant._uint64Value,
                static_cast<T>(sizeof(variant._uint64Value)));
            break;
        }
        case ::util::format::ParamDatatype::VOIDPTR:
        {
            writeData(
                static_cast<uint8_t>(datatype),
                &variant._voidPtrValue,
                static_cast<T>(sizeof(variant._voidPtrValue)));
            break;
        }
        case ::util::format::ParamDatatype::CHARPTR:
        {
            writeString(variant._charPtrValue);
            break;
        }
        case ::util::format::ParamDatatype::SIZEDCHARPTR:
        {
            writeString(
                static_cast<uint8_t>(::util::format::ParamDatatype::SIZEDCHARPTR),
                variant._sizedCharPtrValue->_data,
                static_cast<T>(variant._sizedCharPtrValue->_length));
            break;
        }
        default:
        {
            writeData(static_cast<uint8_t>(datatype), nullptr, 0U);
            break;
        }
    }
}

template<class T, class Timestamp, class ReadOnlyPredicate>
void EntrySerializer<T, Timestamp, ReadOnlyPredicate>::EntryWriter::writeString(
    char const* const source)
{
    if ((source == nullptr) || (_parentPredicate(source)))
    {
        writeData(
            static_cast<uint8_t>(::util::format::ParamDatatype::CHARPTR),
            &source,
            static_cast<T>(sizeof(source)));
    }
    else
    {
        writeString(
            static_cast<uint8_t>(DATATYPE_CHARARRAY), source, static_cast<T>(strlen(source)));
    }
}

template<class T, class Timestamp, class ReadOnlyPredicate>
void EntrySerializer<T, Timestamp, ReadOnlyPredicate>::EntryWriter::writeString(
    uint8_t const typeByte, char const* const source, T length)
{
    writeBytes(&typeByte, static_cast<T>(sizeof(typeByte)));
    T const completeLength = length + 1U;
    T const headerLength   = static_cast<T>(sizeof(T));

    if ((_write + (headerLength + completeLength)) > _bufferEnd)
    {
        // Add a constant length zero-terminated string to indicate overflow ("<?>" = 4 bytes)
        if ((_write + (headerLength + 5U)) <= _bufferEnd)
        {
            length = static_cast<T>(_bufferEnd - _write) - static_cast<T>(headerLength + 4U);
            writeBytes(&length, static_cast<T>(sizeof(length)));
            writeBytes(source, length);
            writeBytes("<?>", 4U);
            return;
        }
    }
    writeBytes(&completeLength, static_cast<T>(sizeof(completeLength)));
    writeBytes(source, length);
    uint8_t const zeroByte = 0U;
    writeBytes(&zeroByte, static_cast<T>(sizeof(zeroByte)));
}

template<class T, class Timestamp, class ReadOnlyPredicate>
void EntrySerializer<T, Timestamp, ReadOnlyPredicate>::EntryWriter::writeData(
    uint8_t const datatype, void const* const source, T const size)
{
    writeBytes(&datatype, static_cast<T>(sizeof(uint8_t)));
    writeBytes(source, size);
}

template<class T, class Timestamp, class ReadOnlyPredicate>
void EntrySerializer<T, Timestamp, ReadOnlyPredicate>::EntryWriter::writeBytes(
    void const* const source, T const size)
{
    if (source == nullptr)
    {
        return;
    }
    if (_write < _bufferEnd)
    {
        (void)memcpy(
            _write,
            source,
            ((_write + size) <= _bufferEnd) ? static_cast<size_t>(size) : _bufferEnd - _write);
    }
    _write += size;
}

template<class T, class Timestamp, class ReadOnlyPredicate>
EntrySerializer<T, Timestamp, ReadOnlyPredicate>::EntryReader::EntryReader(
    uint8_t const* const bufferStart, uint8_t const* const bufferEnd)
: ::util::format::IPrintfArgumentReader()
, _bufferStart(bufferStart)
, _bufferEnd(bufferEnd)
, _read(bufferStart)
, _variant()
, _plainSizedString()
{}

template<class T, class Timestamp, class ReadOnlyPredicate>
::util::format::ParamVariant const*
    EntrySerializer<T, Timestamp, ReadOnlyPredicate>::EntryReader::readArgument(
        ::util::format::ParamDatatype /*datatype*/)
{
    readParamVariant();
    return (_read <= _bufferEnd) ? &_variant : nullptr;
}

template<class T, class Timestamp, class ReadOnlyPredicate>
void EntrySerializer<T, Timestamp, ReadOnlyPredicate>::EntryReader::readBytes(
    void* const dest, T const size)
{
    if (_read < _bufferEnd)
    {
        (void)memcpy(
            dest,
            _read,
            ((_read + size) <= _bufferEnd) ? static_cast<size_t>(size) : _bufferEnd - _read);
    }
    _read += size;
}

template<class T, class Timestamp, class ReadOnlyPredicate>
void EntrySerializer<T, Timestamp, ReadOnlyPredicate>::EntryReader::readParamVariant()
{
    uint8_t datatype = static_cast<uint8_t>(DATATYPE_NONE);
    readBytes(&datatype, static_cast<T>(sizeof(datatype)));

    switch (datatype)
    {
        case DATATYPE_UINT8:
        {
            readBytes(&_variant._uint8Value, static_cast<T>(sizeof(_variant._uint8Value)));
            break;
        }
        case DATATYPE_SINT16:
        case DATATYPE_UINT16:
        {
            readBytes(&_variant._uint16Value, static_cast<T>(sizeof(_variant._uint16Value)));
            break;
        }
        case DATATYPE_SINT32:
        case DATATYPE_UINT32:
        {
            readBytes(&_variant._uint32Value, static_cast<T>(sizeof(_variant._uint32Value)));
            break;
        }
        case DATATYPE_SINT64:
        case DATATYPE_UINT64:
        {
            readBytes(&_variant._uint64Value, static_cast<T>(sizeof(_variant._uint64Value)));
            break;
        }
        case DATATYPE_VOIDPTR:
        {
            readBytes(&_variant._voidPtrValue, static_cast<T>(sizeof(_variant._voidPtrValue)));
            break;
        }
        case DATATYPE_CHARPTR:
        {
            readBytes(&_variant._charPtrValue, static_cast<T>(sizeof(_variant._charPtrValue)));
            break;
        }
        case DATATYPE_CHARARRAY:
        {
            _variant._charPtrValue = readCharArray();
            break;
        }
        case DATATYPE_SIZEDCHARPTR:
        {
            _variant._sizedCharPtrValue = &_plainSizedString;
            _plainSizedString._data     = readCharArray();
            _plainSizedString._length   = strnlen(
                _plainSizedString._data,
                _bufferEnd - reinterpret_cast<uint8_t const*>(_plainSizedString._data));
            break;
        }
        default:
        {
            _variant._voidPtrValue = nullptr;
            break;
        }
    }
}

template<class T, class Timestamp, class ReadOnlyPredicate>
char const* EntrySerializer<T, Timestamp, ReadOnlyPredicate>::EntryReader::readCharArray()
{
    T length = 0U;
    readBytes(&length, static_cast<T>(sizeof(length)));
    char const* const string = reinterpret_cast<char const*>(_read);
    _read += length;
    return string;
}

template<class Timestamp>
IEntrySerializerCallback<Timestamp>::IEntrySerializerCallback() : ::etl::uncopyable()
{}

inline bool SectionPredicate::operator()(void const* const p) const
{
    return (p >= _sectionStart) && (p < _sectionEnd);
}

} // namespace logger
