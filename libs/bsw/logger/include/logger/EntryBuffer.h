// Copyright 2024 Accenture.

#pragma once

#include <estd/slice.h>

#include <cstring>

namespace logger
{
template<uint8_t MaxBufferSize = 64, class E = uint32_t>
class EntryBuffer
{
public:
    class EntryRef
    {
    public:
        explicit EntryRef(E const idx = E(), uint8_t const* const readPos = nullptr)
        : _idx(idx), _readPos(readPos)
        {}

        EntryRef(EntryRef const& src)            = default;
        EntryRef& operator=(EntryRef const& src) = default;

        E getIndex() const { return _idx; }

        uint8_t const* getReadPointer() const { return _readPos; }

    private:
        friend class EntryBuffer;

        void update(E const idx, uint8_t const* const readPos)
        {
            _idx     = idx;
            _readPos = readPos;
        }

        E _idx;
        uint8_t const* _readPos;
    };

    explicit EntryBuffer(::estd::slice<uint8_t> const& outputBuffer)
    : _bufferStart(outputBuffer.data())
    , _bufferEnd(_bufferStart + outputBuffer.size())
    , _firstEntry(_bufferStart)
    , _write(_bufferStart)
    , _size(outputBuffer.size())
    {}

    void addEntry(::estd::slice<uint8_t const> const& entryBuffer);
    uint8_t getNextEntry(::estd::slice<uint8_t> const& entryBuffer, EntryRef& entryRef) const;

private:
    using SignedIndexType = typename ::estd::make_signed<E>::type;

    template<class T>
    inline void movePointer(T& p, size_t offset) const;

    uint8_t* _bufferStart;
    uint8_t* _bufferEnd;
    uint8_t* _firstEntry;
    uint8_t* _write;
    E _firstEntryIndex = 0U;
    E _writeEntryIndex = 0U;
    size_t _usedSize   = 0U;
    size_t _size;
};

template<uint8_t MaxBufferSize, class E>
void EntryBuffer<MaxBufferSize, E>::addEntry(::estd::slice<uint8_t const> const& entryBuffer)
{
    size_t entrySize   = entryBuffer.size();
    uint8_t const* src = entryBuffer.data();
    if (entrySize >= MaxBufferSize)
    {
        entrySize = MaxBufferSize;
    }
    size_t const neededSize = entrySize + 1U;

    while ((_usedSize + neededSize) > _size)
    {
        ++_firstEntryIndex;
        size_t const removeBufferSize = static_cast<size_t>(*_firstEntry) + 1U;
        movePointer(_firstEntry, removeBufferSize);
        _usedSize -= removeBufferSize;
    }
    *_write = static_cast<uint8_t>(entrySize);
    movePointer(_write, 1U);

    if ((_write + entrySize) > _bufferEnd)
    {
        ptrdiff_t const partSize = _bufferEnd - _write;
        (void)memcpy(_write, src, static_cast<size_t>(partSize));
        src += partSize;
        _write = _bufferStart;
        entrySize -= partSize;
    }
    (void)memcpy(_write, src, entrySize);
    movePointer(_write, entrySize);
    _usedSize += neededSize;
    ++_writeEntryIndex;
}

template<uint8_t MaxBufferSize, class E>
uint8_t EntryBuffer<MaxBufferSize, E>::getNextEntry(
    ::estd::slice<uint8_t> const& entryBuffer, EntryRef& entryRef) const
{
    uint8_t const* readPos = entryRef.getReadPointer();
    E entryIndex           = entryRef.getIndex();

    SignedIndexType const entryDiff = static_cast<SignedIndexType>(entryIndex - _firstEntryIndex);
    if ((readPos == nullptr) || (entryDiff < 0))
    {
        entryIndex = _firstEntryIndex;
        readPos    = _firstEntry;
    }
    if (entryIndex == _writeEntryIndex)
    {
        return 0;
    }

    uint8_t entrySize = *readPos;
    movePointer(readPos, 1U);
    uint8_t const* readUpdate = readPos;
    movePointer(readUpdate, static_cast<size_t>(entrySize));
    if (entrySize > entryBuffer.size())
    {
        entrySize = static_cast<uint8_t>(entryBuffer.size());
    }
    uint8_t copySize = entrySize;
    uint8_t* dest    = entryBuffer.data();

    if ((readPos + copySize) > _bufferEnd)
    {
        ptrdiff_t const partSize = _bufferEnd - readPos;
        (void)memcpy(dest, readPos, static_cast<size_t>(partSize));
        dest += partSize;
        readPos = _bufferStart;
        copySize -= static_cast<uint8_t>(partSize);
    }
    (void)memcpy(dest, readPos, static_cast<size_t>(copySize));
    entryRef.update(entryIndex + 1U, readUpdate);
    return entrySize;
}

template<uint8_t MaxBufferSize, class E>
template<class T>
inline void EntryBuffer<MaxBufferSize, E>::movePointer(T& p, size_t const offset) const
{
    p += offset;
    if (p >= _bufferEnd)
    {
        p -= _size;
    }
}

} // namespace logger
