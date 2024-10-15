// Copyright 2025 Accenture.

#pragma once

#include <assert.h>

typedef unsigned char bool_t;
#ifndef FALSE
#define FALSE ((bool_t)0) /**< boolean 'false' value */
#endif
#ifndef TRUE
#define TRUE ((bool_t)(!FALSE)) /**< boolean 'true' value */
#endif

#define LWIP_CHKSUM_ALGORITHM 3

#define PACK_STRUCT_STRUCT

#define LWIP_PLATFORM_DIAG(x) // printf x;

#define LWIP_PLATFORM_ASSERT(x) assert(x)

#define LWIP_NO_STDINT_H   0
#define LWIP_NO_INTTYPES_H 0

// Checksum settings

#ifndef CHECKSUM_CHECK_IP
#define CHECKSUM_CHECK_IP 1
#endif

#ifndef CHECKSUM_GEN_IP
#define CHECKSUM_GEN_IP 1
#endif

#ifndef CHECKSUM_CHECK_ICMP
#define CHECKSUM_CHECK_ICMP 1
#endif

#ifndef CHECKSUM_GEN_ICMP
#define CHECKSUM_GEN_ICMP 1
#endif

#ifndef CHECKSUM_CHECK_UDP
#define CHECKSUM_CHECK_UDP 1
#endif

#ifndef CHECKSUM_GEN_UDP
#define CHECKSUM_GEN_UDP 1
#endif

#ifndef CHECKSUM_CHECK_TCP
#define CHECKSUM_CHECK_TCP 1
#endif

#ifndef CHECKSUM_GEN_TCP
#define CHECKSUM_GEN_TCP 1
#endif
