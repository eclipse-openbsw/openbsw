// Copyright 2025 Accenture.

#pragma once

#include <platform/estdint.h>

#include <assert.h>

typedef unsigned char bool_t;
#ifndef FALSE
#define FALSE 0 /**< boolean 'false' value */
#endif
#ifndef TRUE
#define TRUE 1
#endif

// for unit test
#ifdef BYTE_ORDER
#undef BYTE_ORDER
#endif

#define BYTE_ORDER LITTLE_ENDIAN

#define LWIP_CHKSUM_ALGORITHM 3

#define MEM_ALIGNMENT 16

#define PACK_STRUCT_STRUCT

/* Mapped LWIP_PLATFORM_DIAG to Logger in openBSW */
#define LWIP_PLATFORM_DIAG(x) log_lwipInfo x

#define LWIP_PLATFORM_ASSERT(x) assert(x)

#define LWIP_NO_STDINT_H   0
#define LWIP_NO_INTTYPES_H 0

#define U16_F "d"
#define S16_F "d"
#define X8_F  "x"
#define X16_F "x"
#define U32_F "d"
#define S32_F "d"
#define X32_F "x"
#define SZT_F "d"

#ifndef CHECKSUM_CHECK_IP
#define CHECKSUM_CHECK_IP 1
#endif

#ifndef CHECKSUM_GEN_IP
#define CHECKSUM_GEN_IP 0
#endif

#ifndef CHECKSUM_CHECK_ICMP
#define CHECKSUM_CHECK_ICMP 1
#endif

#ifndef CHECKSUM_GEN_ICMP
#define CHECKSUM_GEN_ICMP 0
#endif

#ifndef CHECKSUM_CHECK_UDP
#define CHECKSUM_CHECK_UDP 1
#endif

#ifndef CHECKSUM_GEN_UDP
#define CHECKSUM_GEN_UDP 0
#endif

#ifndef CHECKSUM_CHECK_TCP
#define CHECKSUM_CHECK_TCP 1
#endif

#ifndef CHECKSUM_GEN_TCP
#define CHECKSUM_GEN_TCP 0
#endif
