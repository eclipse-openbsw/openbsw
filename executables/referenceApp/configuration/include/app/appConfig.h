// Copyright 2025 Accenture.

#pragma once

#include <async/TaskInitializer.h>

#ifdef PLATFORM_SUPPORT_TRANSPORT
static constexpr uint16_t LOGICAL_ADDRESS = 0x0600U;
#endif // PLATFORM_SUPPORT_TRANSPORT

#define safety_task_stackSize 2048U

#ifdef PLATFORM_SUPPORT_ETHERNET
namespace doip
{
static uint8_t const SOCKET_GROUP        = 0U;
static uint16_t const ANNOUNCE_WAIT_TOUT = 500U;
static uint16_t const ANNOUNCE_INTERVAL  = 500U;
static uint8_t const NUM_ANNOUNCEMENTS   = 3U;

static uint16_t const INITIAL_INACTIVITY_TIMEOUT = 2000U;
static uint32_t const GENERAL_INACTIVITY_TIMEOUT = 300000U;
static uint16_t const ALIVE_CHECK_TIMEOUT        = 500U;
static uint32_t const MAX_PAYLOAD_LENGTH         = 456789U;

static uint32_t const MAX_DATA_SIZE_LOGICAL_REQUESTS = 4095U;

static uint8_t const VIN_LENGTH = 17U;
static uint8_t const GID_LENGTH = 6U;
static uint8_t const EID_LENGTH = 6U;

static uint8_t const MAC_LENGTH = 6U;

static size_t const NUM_DIAGNOSTICSENDJOBS = 6U;
static size_t const NUM_PROTOCOLSENDJOBS   = 8U;

static size_t const NUM_UNICAST  = 4U;
static size_t const NUM_SOCKETS  = 1U;
static size_t const NUM_REQUESTS = 15U;

} // namespace doip
#endif // PLATFORM_SUPPORT_ETHERNET
