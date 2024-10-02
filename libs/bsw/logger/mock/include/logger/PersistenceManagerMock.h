// Copyright 2024 Accenture.

#ifndef GUARD_99A827D3_0EEC_47C4_85DA_5C2EE7705E46
#define GUARD_99A827D3_0EEC_47C4_85DA_5C2EE7705E46

#include <logger/IPersistenceManager.h>

#include <gmock/gmock.h>

namespace logger
{
class PersistenceManagerMock : public IPersistenceManager
{
public:
    MOCK_CONST_METHOD1(writeMapping, bool(::estd::slice<uint8_t const> const&));
    MOCK_CONST_METHOD1(readMapping, ::estd::slice<uint8_t const>(::estd::slice<uint8_t>));
};

} // namespace logger

#endif // GUARD_99A827D3_0EEC_47C4_85DA_5C2EE7705E46
