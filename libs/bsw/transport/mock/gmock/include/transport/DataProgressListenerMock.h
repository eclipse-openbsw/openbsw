// Copyright 2024 Accenture.

/**
 * Contains
 * \file
 * \ingroup
 */

#ifndef GUARD_16E2FA4F_4F4A_41BD_97AC_4E9F5A4E5A40
#define GUARD_16E2FA4F_4F4A_41BD_97AC_4E9F5A4E5A40

#include "transport/IDataProgressListener.h"

#include <gmock/gmock.h>

namespace transport
{
class DataProgressListenerMock : public IDataProgressListener
{
public:
    DataProgressListenerMock() {}

    MOCK_METHOD2(dataProgressed, void(TransportMessage&, uint32_t));
};

} // namespace transport

#endif /* GUARD_16E2FA4F_4F4A_41BD_97AC_4E9F5A4E5A40 */
