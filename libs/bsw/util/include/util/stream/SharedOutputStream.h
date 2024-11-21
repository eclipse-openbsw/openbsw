// Copyright 2024 Accenture.

#ifndef GUARD_98F5AF49_0973_45BE_BE94_376049C23D95
#define GUARD_98F5AF49_0973_45BE_BE94_376049C23D95

#include "util/stream/ISharedOutputStream.h"

namespace util
{
namespace stream
{
/**
 * The ISharedOutputStream class implementation.
 *
 */
class SharedOutputStream : public ISharedOutputStream
{
public:
    explicit SharedOutputStream(IOutputStream& strm);

    IOutputStream& startOutput(IContinuousUser* user) override;
    void endOutput(IContinuousUser* user) override;
    void releaseContinuousUser(IContinuousUser& user) override;

private:
    IOutputStream& _stream;
    IContinuousUser* _user;
};

} // namespace stream
} // namespace util

#endif /* GUARD_98F5AF49_0973_45BE_BE94_376049C23D95 */
