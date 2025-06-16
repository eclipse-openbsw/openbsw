// Copyright 2025 Accenture.

#pragma once

#include <storage/LinkedBuffer.h>

#include <estd/forward_list.h>
#include <estd/functional.h>
#include <estd/variant.h>

namespace storage
{

class StorageJob : public ::estd::forward_list_node<StorageJob>
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
        = ::estd::variant<Result::Init, Result::Error, Result::DataLoss, Result::Success>;

    struct Type
    {
        struct None
        {};

        class Write
        {
        public:
            using BufferType = ::util::LinkedBuffer<uint8_t const>;

            explicit Write(BufferType& buffer, size_t const offset)
            : _buffer(buffer), _offset(offset)
            {}

            BufferType& getBuffer() const { return _buffer; }

            size_t getOffset() const { return _offset; }

        private:
            BufferType& _buffer;
            size_t _offset;
        };

        class Read
        {
        public:
            using BufferType = ::util::LinkedBuffer<uint8_t>;

            explicit Read(BufferType& buffer, size_t const offset)
            : _buffer(buffer), _offset(offset), _readSize(0U)
            {}

            BufferType& getBuffer() const { return _buffer; }

            size_t getOffset() const { return _offset; }

            size_t getReadSize() const { return _readSize; }

            void setReadSize(size_t const readSize) { _readSize = readSize; }

        private:
            BufferType& _buffer;
            size_t _offset;
            size_t _readSize;
        };
    };

    using JobDoneCallback = ::estd::function<void(StorageJob&)>;

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
        (void)_job.emplace<Type::Write>().construct(buffer, offset);
    }

    void initRead(Type::Read::BufferType& buffer) { initRead(buffer, 0U); }

    void initRead(Type::Read::BufferType& buffer, size_t const offset)
    {
        (void)_job.emplace<Type::Read>().construct(buffer, offset);
    }

    ResultType getResult() const { return _result; }

    uint32_t getId() const { return _id; }

    template<typename T>
    bool is() const
    {
        return _job.is<T>();
    }

    Type::Read& getRead() { return _job.get<Type::Read>(); }

    Type::Write& getWrite() { return _job.get<Type::Write>(); }

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
    ::estd::variant<Type::None, Type::Write, Type::Read> _job;
    JobDoneCallback _callback;
    uint32_t _id;
};

} // namespace storage
