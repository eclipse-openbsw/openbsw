// Copyright 2025 Accenture.

#include "tcp/socket/AbstractSocket.h"

namespace tcp
{
AbstractSocket::AbstractSocket() : _dataListener(nullptr), _sendNotificationListener(nullptr) {}

} // namespace tcp
