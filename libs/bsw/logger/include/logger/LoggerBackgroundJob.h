// Copyright 2024 Accenture.

#ifndef GUARD_ED1F6B5A_28B1_4380_AD4B_B444BF60FCB7
#define GUARD_ED1F6B5A_28B1_4380_AD4B_B444BF60FCB7

#include "logger/BufferedLoggerOutput.h"
#include "logger/IEntryOutput.h"

#include <bsp/background/AbstractBackgroundJob.h>
#include <bsp/background/IBackgroundJobHandler.h>
#include <util/logger/LevelInfo.h>

namespace logger
{
template<class T>
class LoggerBackgroundJob : public bsp::AbstractBackgroundJob
{
public:
    LoggerBackgroundJob(
        bsp::IBackgroundJobHandler& backgroundJobHandler,
        T& entryBuffer,
        IEntryOutput<typename T::EntryIndexType, typename T::TimestampType>& entryOutput);
    virtual ~LoggerBackgroundJob();

    void init();
    void shutdown();

    void flush();

    bool execute() override;

private:
    bsp::IBackgroundJobHandler& _backgroundJobHandler;
    T& _entryBuffer;
    IEntryOutput<typename T::EntryIndexType, typename T::TimestampType>& _entryOutput;
    typename T::EntryRefType _entryRef;
};

template<class T>
LoggerBackgroundJob<T>::LoggerBackgroundJob(
    bsp::IBackgroundJobHandler& backgroundJobHandler,
    T& entryBuffer,
    IEntryOutput<typename T::EntryIndexType, typename T::TimestampType>& entryOutput)
: bsp::AbstractBackgroundJob()
, _backgroundJobHandler(backgroundJobHandler)
, _entryBuffer(entryBuffer)
, _entryOutput(entryOutput)
, _entryRef()
{}

template<class T>
LoggerBackgroundJob<T>::~LoggerBackgroundJob()
{}

template<class T>
void LoggerBackgroundJob<T>::init()
{
    (void)_backgroundJobHandler.addJob(*this);
}

template<class T>
void LoggerBackgroundJob<T>::shutdown()
{
    (void)_backgroundJobHandler.removeJob(*this);
}

template<class T>
void LoggerBackgroundJob<T>::flush()
{
    bool iterate = true;

    while (iterate)
    {
        iterate = _entryBuffer.outputEntry(_entryOutput, _entryRef);
    }
}

template<class T>
bool LoggerBackgroundJob<T>::execute()
{
    (void)_entryBuffer.outputEntry(_entryOutput, _entryRef);
    return true;
}

} /* namespace logger */

#endif // GUARD_ED1F6B5A_28B1_4380_AD4B_B444BF60FCB7
