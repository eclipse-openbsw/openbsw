// Copyright 2025 Accenture.

#pragma once

// #include "lwip/config.h"
#include <sys/errno.h>

#include <platform/estdint.h>

#include <stdlib.h>

#define BROADCAST_NETIF_CHECK 1

//  ------------------- PREDEFINED, SHALL NOT BE CHANGED -------------------

#define TCP_LISTEN_BACKLOG                     0
#define LWIP_NETIF_STATUS_CALLBACK             1
#define LWIP_NETIF_LINK_CALLBACK               1
#define SYS_LIGHTWEIGHT_PROT                   1
#define LWIP_ALLOW_MEM_FREE_FROM_OTHER_CONTEXT 0
#define LWIP_DNS                               0
#define NO_SYS                                 1
#define NO_SYS_NO_TIMERS                       0
#define IP_FRAG                                0
#define TCP_QUEUE_OOSEQ                        0
#define LWIP_NETCONN                           0
#define LWIP_SOCKET                            0
#define IP_REASSEMBLY                          0
#define LWIP_IGMP                              1
#define LWIP_BROADCAST_PING                    1

//  ------------------- PREDEFINED, MAY BE CHANGED -------------------
#ifndef LWIP_IPV6
#define LWIP_IPV6 0
#endif

#ifndef LWIP_IPSEC
#define LWIP_IPSEC 1
#endif

#ifndef LWIP_RAW
#define LWIP_RAW 1
#endif

#ifndef LWIP_ARP
#define LWIP_ARP 1
#endif

#ifndef ETH_PAD_SIZE
#define ETH_PAD_SIZE   0
#define PBUF_LINK_HLEN 20
#endif

#ifndef LWIP_AUTOIP_RAND
#define LWIP_AUTOIP_RAND(netif) 1
#endif

#ifndef LWIP_RAND
#define LWIP_RAND() rand()
#endif

#ifndef LWIP_AUTOIP_CREATE_SEED_ADDR
#define LWIP_AUTOIP_CREATE_SEED_ADDR(netif) htonl(auto_ip_create_seed_addr())
#endif

#ifndef LWIP_CREATE_SEED
#define LWIP_CREATE_SEED(netif) \
    srand((netif->hwaddr[3] << 16) & (netif->hwaddr[4] << 8) & netif->hwaddr[5])
#endif

#ifndef LWIP_HOOK_IP4_ROUTE_SRC
#define LWIP_HOOK_IP4_ROUTE_SRC(src, dest) custom_ip_route_src(dest, src)
#endif

#ifndef SO_REUSE
#define SO_REUSE 1
#endif

#ifndef LWIP_NETIF_SPECIFIC_TTL
#define LWIP_NETIF_SPECIFIC_TTL 0
#endif

#ifndef LWIP_DHCP_AUTOIP_COOP
#define LWIP_DHCP_AUTOIP_COOP 1
#endif

#ifndef LWIP_DHCP
#define LWIP_DHCP 1
#endif

#ifndef LWIP_AUTOIP
#define LWIP_AUTOIP 1
#endif

#ifndef LWIP_DHCP_AUTOIP_COOP_TRIES
#define LWIP_DHCP_AUTOIP_COOP_TRIES 2
#endif

#ifndef MEMP_NUM_SYS_TIMEOUT
#define MEMP_NUM_SYS_TIMEOUT 6
#endif

#ifndef MEM_SIZE
#define MEM_SIZE (8 * 1024)
#endif

#ifndef MEMP_NUM_PBUF
#define MEMP_NUM_PBUF 0
#endif

#ifndef MEMP_NUM_TCP_SEG
#define MEMP_NUM_TCP_SEG 64 // at least as big as TCP_SND_QUEUELEN
#endif

#ifndef RXFRAME_MBOX_SIZE
#define RXFRAME_MBOX_SIZE 256
#endif

#ifndef MEMP_NUM_TCPIP_MSG_INPKT
#define MEMP_NUM_TCPIP_MSG_INPKT 16
#endif

#ifndef PBUF_POOL_BUFSIZE
#define PBUF_POOL_BUFSIZE 0x100 // = 256
#endif

#ifndef TCP_MSS
#define TCP_MSS 1460
#endif

#ifndef TCP_OVERSIZE
#define TCP_OVERSIZE (256)
#endif

#ifndef TCP_SND_BUF
#define TCP_SND_BUF \
    (2 * TCP_MSS) // lwip_sanity_check: WARNING: TCP_SND_BUF must be at least as much as (2 *
                  // TCP_MSS) for things to work smoothly
#endif

#ifndef MEMP_NUM_UDP_PCB
#define MEMP_NUM_UDP_PCB 8
#endif

#ifndef MEMP_NUM_TCP_PCB
#define MEMP_NUM_TCP_PCB 8 // needs ~ 160 bytes each PCB
#endif

#ifndef MEMP_NUM_TCP_PCB_LISTEN
#define MEMP_NUM_TCP_PCB_LISTEN 10
#endif

#ifndef TCP_WND
#define TCP_WND (2 * TCP_MSS)
#endif

#ifndef TCP_SND_QUEUELEN
#define TCP_SND_QUEUELEN 32
#endif

#ifndef TCP_SYNMAXRTX
#define TCP_SYNMAXRTX 1
#endif

#ifndef PBUF_POOL_SIZE
#define PBUF_POOL_SIZE 0
#endif

#ifndef LWIP_NETIF_HOSTNAME
#define LWIP_NETIF_HOSTNAME 1
#endif

#ifndef ARP_TABLE_SIZE
#define ARP_TABLE_SIZE 127
#endif

#ifndef ARP_QUEUEING
#define ARP_QUEUEING 1
#endif

#ifndef LWIP_SUPPORT_CUSTOM_PBUF
#define LWIP_SUPPORT_CUSTOM_PBUF 1
#endif

#ifndef ETHARP_SUPPORT_VLAN
#define ETHARP_SUPPORT_VLAN 1
#endif

#ifndef LWIP_HOOK_VLAN_CHECK
#define LWIP_HOOK_VLAN_CHECK(netif, ethhdr, vlan) 1
#endif

#define LWIP_HOOK_VLAN_SET(netif, p, src, dst, eth_type) vlanForNetif(netif)

#ifndef LWIP_SUPPRESS_IGMP_PKTS
#define LWIP_SUPPRESS_IGMP_PKTS 1
#endif

#define TFTP_MAX_FILENAME_LEN 64
#define TFTP_MAX_MODE_LEN     9

//  ------------------- DEBUG -------------------

#define LWIP_DEBUG

#ifndef ETHARP_DEBUG
#define ETHARP_DEBUG LWIP_DBG_OFF
#endif
#ifndef NETIF_DEBUG
#define NETIF_DEBUG LWIP_DBG_ON
#endif
#ifndef PBUF_DEBUG
#define PBUF_DEBUG LWIP_DBG_OFF
#endif
#ifndef API_LIB_DEBUG
#define API_LIB_DEBUG LWIP_DBG_OFF
#endif
#ifndef API_MSG_DEBUG
#define API_MSG_DEBUG LWIP_DBG_OFF
#endif
#ifndef SOCKETS_DEBUG
#define SOCKETS_DEBUG LWIP_DBG_ON
#endif
#ifndef ICMP_DEBUG
#define ICMP_DEBUG LWIP_DBG_OFF
#endif
#ifndef IGMP_DEBUG
#define IGMP_DEBUG LWIP_DBG_OFF
#endif
#ifndef INET_DEBUG
#define INET_DEBUG LWIP_DBG_OFF
#endif
#ifndef IP_DEBUG
#define IP_DEBUG LWIP_DBG_OFF
#endif
#ifndef IP_REASS_DEBUG
#define IP_REASS_DEBUG LWIP_DBG_OFF
#endif
#ifndef RAW_DEBUG
#define RAW_DEBUG LWIP_DBG_OFF
#endif
#ifndef MEM_DEBUG
#define MEM_DEBUG LWIP_DBG_OFF
#endif
#ifndef MEMP_DEBUG
#define MEMP_DEBUG LWIP_DBG_OFF
#endif
#ifndef SYS_DEBUG
#define SYS_DEBUG LWIP_DBG_OFF
#endif
#ifndef TCP_DEBUG
#define TCP_DEBUG LWIP_DBG_OFF
#endif
#ifndef TCP_INPUT_DEBUG
#define TCP_INPUT_DEBUG LWIP_DBG_OFF
#endif
#ifndef TCP_FR_DEBUG
#define TCP_FR_DEBUG LWIP_DBG_OFF
#endif
#ifndef TCP_RTO_DEBUG
#define TCP_RTO_DEBUG LWIP_DBG_OFF
#endif
#ifndef TCP_CWND_DEBUG
#define TCP_CWND_DEBUG LWIP_DBG_OFF
#endif
#ifndef TCP_WND_DEBUG
#define TCP_WND_DEBUG LWIP_DBG_OFF
#endif
#ifndef TCP_OUTPUT_DEBUG
#define TCP_OUTPUT_DEBUG LWIP_DBG_OFF
#endif
#ifndef TCP_RST_DEBUG
#define TCP_RST_DEBUG LWIP_DBG_OFF
#endif
#ifndef TCP_QLEN_DEBUG
#define TCP_QLEN_DEBUG LWIP_DBG_OFF
#endif
#ifndef UDP_DEBUG
#define UDP_DEBUG LWIP_DBG_OFF
#endif
#ifndef TCPIP_DEBUG
#define TCPIP_DEBUG LWIP_DBG_OFF
#endif
#ifndef PPP_DEBUG
#define PPP_DEBUG LWIP_DBG_OFF
#endif
#ifndef SLIP_DEBUG
#define SLIP_DEBUG LWIP_DBG_OFF
#endif
#ifndef DHCP_DEBUG
#define DHCP_DEBUG LWIP_DBG_OFF
#endif
#ifndef AUTOIP_DEBUG
#define AUTOIP_DEBUG LWIP_DBG_OFF
#endif
#ifndef SNMP_MSG_DEBUG
#define SNMP_MSG_DEBUG LWIP_DBG_OFF
#endif
#ifndef SNMP_MIB_DEBUG
#define SNMP_MIB_DEBUG LWIP_DBG_OFF
#endif
#ifndef DNS_DEBUG
#define DNS_DEBUG LWIP_DBG_OFF
#endif
#ifndef TFTP_DEBUG
#define TFTP_DEBUG LWIP_DBG_OFF
#endif
#ifndef DHCP_COARSE_TIMER_SECS
#define DHCP_COARSE_TIMER_SECS 1
#endif

#define LWIP_STATS_LARGE 1

//  ------------------- EXTERNAL DECLARATIONS -------------------
// Since we cannot include ip4_addr.h here to avoid circular dependencies,
// we forward declare the necessary type ip4_addr.
struct ip4_addr;
extern struct netif*
custom_ip_route_src(struct ip4_addr const* const, struct ip4_addr const* const);

extern int32_t vlanForNetif(void const* const lwipNi);
extern uint32_t auto_ip_create_seed_addr(void);

void log_lwipError(char const* message);
void log_lwipInfo(char const* message, ...);

#include "arch/sys_arch.h"
