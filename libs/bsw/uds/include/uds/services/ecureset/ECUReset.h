// Copyright 2024 Accenture.

#pragma once

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

