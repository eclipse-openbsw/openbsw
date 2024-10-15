// Copyright 2024 Accenture.

#pragma once

#include <estd/constructor.h>

namespace util
{
namespace spsc
{
/**
 * Models read access to an underlying input stream of elements.
 * \tparam T    Type of elements of underlying input stream.
 */
template<class T>
class IReader
{
    IReader& operator=(IReader const&);

public:
    virtual ~IReader() = default;

    /**
     * Returns whether there are items to read.
     * \note
     * Always check that the reader is not empty before reading,
     * otherwise behaviour is undefined!
     */
    virtual bool empty() const = 0;

    /**
     * Returns a read only reference to the next element.
     * \note
     * If the underlying input is empty, the behaviour is undefined.
     */
    virtual T const& peek() const = 0;

    /**
     * Advances the underlying input stream by one.
     * \note
     * No sanity checks are made, thus calling advance() on an empty
     * IReader results in undefined behaviour.
     */
    virtual void advance() = 0;

    /**
     * Returns the current number of items that can be read.
     */
    virtual size_t size() const = 0;
};

/**
 * Reader is input stream abstraction based on a IReader interface.
 * \tparam T    Type of elements in the input stream.
 */
template<class T>
class Reader
{
    IReader<T>& _reader;

public:
    /**
     * Constructs a Reader from a given IReader<T>.
     */
    Reader(IReader<T>& reader) : _reader(reader) {}

    struct Read
    {
        explicit Read(IReader<T>& reader) : _reader(reader) {}

        ~Read() { _reader.advance(); }

        T const& operator*() const { return _reader.peek(); }

        T const* operator->() const { return &_reader.peek(); }

        IReader<T>& _reader;
    };

    // This will only work if RVO is done!
    Read create() { return Read(_reader); }

    IReader<T>& reader() { return _reader; }

    T read() { return *Read(_reader); }

    template<class Consumer>
    void read(Consumer const& consumer)
    {
        consumer(_reader.peek());
        _reader.advance();
    }

    size_t size() const { return _reader.size(); }

    bool empty() const { return _reader.empty(); }
};

template<class T>
class IWriter
{
    IWriter& operator=(IWriter const&);

public:
    virtual ~IWriter() = default;

    /**
     * Returns whether and item can be written.
     *
     * \note
     * Always check that the writer is not full before inserting an element,
     * otherwise behaviour is undefined!
     */
    virtual bool full() const = 0;

    /**
     * Returns a writable reference to the next elements. Call commit after the elements
     * has been populated to make it available to the reader.
     *
     * \note
     * The underlying stream must not be full otherwise the behaviour is undefined.
     */
    virtual T& next() = 0;

    /**
     * Advances the underlying input stream by one.
     *
     * \note
     * No sanity checks are made, thus calling advance() on a full
     * IWriter results in undefined behaviour.
     */
    virtual void commit() = 0;

    /**
     * Writes a given value of type T.
     *
     * One possible implementation might be:
     * \code{.cpp}
     * next() = value;
     * commit();
     * \endcode
     */
    virtual void write(T const& value) = 0;
};

template<class T>
class Writer
{
    IWriter<T>& _writer;

public:
    Writer(IWriter<T>& writer) : _writer(writer) {}

    struct Write
    {
        explicit Write(IWriter<T>& writer) : _writer(writer) {}

        ~Write() { _writer.commit(); }

        T& operator*() const { return _writer.next(); }

        T* operator->() const { return &_writer.next(); }

        T* memory() const { return &_writer.next(); }

        IWriter<T>& _writer;
    };

    // This will only work if RVO is done!!!
    Write create() { return Write(_writer); }

    ::estd::constructor<T> emplace() { return ::estd::constructor<T>(Write(_writer).memory()); }

    virtual void write(T const& value)
    {
        _writer.next() = value;
        _writer.commit();
    }

    template<class Producer>
    void write(Producer const& producer)
    {
        producer(_writer.next());
        _writer.commit();
    }

    bool full() const { return _writer.full(); }
};

template<class Q>
struct QueueReader : public IReader<typename Q::value_type>
{
public:
    using value_type = typename Q::value_type;

    explicit QueueReader(Q& queue) : receiver(queue) {}

    bool empty() const override { return receiver.empty(); }

    value_type const& peek() const override { return receiver.peek(); }

    void advance() override { receiver.advance(); }

    size_t size() const override { return receiver.size(); }

    typename Q::Receiver receiver;
};

template<class Q>
struct QueueWriter : public IWriter<typename Q::value_type>
{
public:
    using value_type = typename Q::value_type;

    explicit QueueWriter(Q& queue) : sender(queue) {}

    bool full() const override { return sender.full(); }

    value_type& next() override { return sender.next(); }

    void commit() override { sender.write_next(); }

    void write(value_type const& value) override
    {
        sender.next() = value;
        sender.write_next();
    }

    typename Q::Sender sender;
};

} // namespace spsc
} // namespace util
