// Copyright 2024 Accenture.

#ifndef GUARD_5E5D97C6_78AD_4943_B76D_F51AEA0FE906
#define GUARD_5E5D97C6_78AD_4943_B76D_F51AEA0FE906

#include <util/stream/SharedOutputStream.h>
#include <util/stream/StdoutStream.h>
#include <util/stream/TaggedSharedOutputStream.h>

#include <estd/functional.h>
#include <estd/string.h>

namespace console
{
class StdioConsoleInput
{
public:
    using OnLineProcessed = ::estd::function<void(void)>;

    using OnLineReceived = ::estd::function<void(
        ::util::stream::ISharedOutputStream& outputStream,
        ::estd::string const& line,
        OnLineProcessed const& onLineProcessed)>;

public:
    StdioConsoleInput(StdioConsoleInput const&)     = delete;
    StdioConsoleInput& operator=(StdioConsoleInput) = delete;

    explicit StdioConsoleInput(char const* prefix, char const* suffix);

    void init(OnLineReceived const& onLineReceived);

    void shutdown();

    void run();

private:
    void onLineReceived(
        ::util::stream::ISharedOutputStream& outputStream,
        ::estd::string const&,
        OnLineProcessed const& callback);
    void onLineProcessed();

private:
    ::util::stream::StdoutStream fStdoutStream;
    ::util::stream::SharedOutputStream fSharedOutputStream;
    ::util::stream::TaggedSharedOutputStream fTaggedSharedOutputStream;

    ::estd::declare::string<128> fLine;
    OnLineReceived fOnLineReceived;
    bool fIsSuspended;
};

} /* namespace console */

#endif /* GUARD_5E5D97C6_78AD_4943_B76D_F51AEA0FE906 */
