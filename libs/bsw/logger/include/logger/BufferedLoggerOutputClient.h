// Copyright 2024 Accenture.

#ifndef GUARD_A08407C6_467B_4866_A28F_58D0978186FB
#define GUARD_A08407C6_467B_4866_A28F_58D0978186FB

#include "logger/BufferedLoggerOutput.h"
#include "logger/IBufferedLoggerOutputClient.h"
#include "logger/IEntryOutput.h"
#include "logger/ILoggerListener.h"

namespace logger
{
template<class T>
class BufferedLoggerOutputClient
: public IBufferedLoggerOutputClient<typename T::EntryIndexType, typename T::TimestampType>
{
public:
    explicit BufferedLoggerOutputClient(T& bufferedLoggerOutput);

    void addListener(ILoggerListener& listener) override;
    void removeListener(ILoggerListener& listener) override;
    bool outputEntry(
        IEntryOutput<typename T::EntryIndexType, typename T::TimestampType>& output) override;

private:
    T& _bufferedLoggerOutput;
    typename T::EntryRefType _entryRef;
};

template<class T>
BufferedLoggerOutputClient<T>::BufferedLoggerOutputClient(T& bufferedLoggerOutput)
: IBufferedLoggerOutputClient<typename T::EntryIndexType, typename T::TimestampType>()
, _bufferedLoggerOutput(bufferedLoggerOutput)
, _entryRef()
{}

template<class T>
void BufferedLoggerOutputClient<T>::addListener(ILoggerListener& listener)
{
    _bufferedLoggerOutput.addListener(listener);
}

template<class T>
void BufferedLoggerOutputClient<T>::removeListener(ILoggerListener& listener)
{
    _bufferedLoggerOutput.removeListener(listener);
}

template<class T>
bool BufferedLoggerOutputClient<T>::outputEntry(
    IEntryOutput<typename T::EntryIndexType, typename T::TimestampType>& output)
{
    return _bufferedLoggerOutput.outputEntry(output, _entryRef);
}

} // namespace logger

#endif // GUARD_A08407C6_467B_4866_A28F_58D0978186FB
