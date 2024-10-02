// Copyright 2024 Accenture.

#ifndef GUARD_436099F1_E64D_4D8C_995D_A4999C6CE0B4
#define GUARD_436099F1_E64D_4D8C_995D_A4999C6CE0B4

#include "estd/uncopyable.h"
#include "uds/base/Service.h"

namespace uds
{
class ECUReset : public Service
{
    UNCOPYABLE(ECUReset);

public:
    ECUReset();
};

} // namespace uds

#endif // GUARD_436099F1_E64D_4D8C_995D_A4999C6CE0B4
