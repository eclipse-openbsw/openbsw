// Copyright 2025 Accenture.

#pragma once

#include <etl/delegate.h>
#include <etl/intrusive_list.h>
#include <etl/variant.h>
#include <util/buffer/LinkedBuffer.h>

namespace storage
{

class StorageJob : public ::etl::bidirectional_link<0>
{
public:
    struct Result
    {
        struct Init
        {};

        struct Error
        {};

        struct DataLoss
        {};

        struct Success
        {};
    };

    using ResultType
        = ::etl::variant<Result::Init, Result::Error, Result::DataLoss, Result::Success>;

    struct Type
    {
        struct None
        {};

        class Write
        {
        public:
            using BufferType = ::util::buffer::LinkedBuffer<uint8_t const>;

            explicit Write(BufferType& buffer, size_t const offset)
            : _buffer(buffer), _offset(offset)
            {}

            BufferType& getBuffer() const { return _buffer; }

            size_t getOffset() const { return _offset; }

        private:
            BufferType& _buffer;
            size_t const _offset;
        };

        class Read
        {
        public:
            using BufferType = ::util::buffer::LinkedBuffer<uint8_t>;

            explicit Read(BufferType& buffer, size_t const offset)
            : _buffer(buffer), _offset(offset), _readSize(0U)
            {}

            BufferType& getBuffer() const { return _buffer; }

            size_t getOffset() const { return _offset; }

            size_t getReadSize() const { return _readSize; }

            void setReadSize(size_t const readSize) { _readSize = readSize; }

        private:
            BufferType& _buffer;
            size_t const _offset;
            size_t _readSize;
        };
    };

    using JobDoneCallback = ::etl::delegate<void(StorageJob&)>;

    explicit StorageJob() : _result(Result::Init()), _job(Type::None()), _id(0U) {}

    StorageJob(StorageJob const&)            = delete;
    StorageJob& operator=(StorageJob const&) = delete;

    void init(uint32_t const id, JobDoneCallback const callback)
    {
        _id       = id;
        _result   = Result::Init();
        _callback = callback;
    }

    void initWrite(Type::Write::BufferType& buffer) { initWrite(buffer, 0U); }

    void initWrite(Type::Write::BufferType& buffer, size_t const offset)
    {
        (void)_job.emplace<Type::Write>(buffer, offset);
    }

    void initRead(Type::Read::BufferType& buffer) { initRead(buffer, 0U); }

    void initRead(Type::Read::BufferType& buffer, size_t const offset)
    {
        (void)_job.emplace<Type::Read>(buffer, offset);
    }

    ResultType getResult() const { return _result; }

    template<typename T>
    bool hasResult() const
    {
        return ::etl::holds_alternative<T>(_result);
    }

    template<typename T>
    bool is() const
    {
        return ::etl::holds_alternative<T>(_job);
    }

    uint32_t getId() const { return _id; }

    Type::Read& getRead() { return ::etl::get<Type::Read>(_job); }

    Type::Write& getWrite() { return ::etl::get<Type::Write>(_job); }

    // set the result and run the callback
    // NOTE: this method is to be used by IStorage classes only
    void sendResult(ResultType const result)
    {
        _result = result;
        if (_callback)
        {
            _callback(*this);
        }
    }

private:
    ResultType _result;
    ::etl::variant<Type::None, Type::Write, Type::Read> _job;
    JobDoneCallback _callback;
    uint32_t _id;
};

} // namespace storage
