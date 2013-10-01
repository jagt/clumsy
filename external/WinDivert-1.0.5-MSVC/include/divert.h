/*
 * divert.h
 * (C) 2012, all rights reserved,
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __DIVERT_H
#define __DIVERT_H

#ifndef DIVERT_KERNEL
#include <windows.h>
#endif      /* DIVERT_KERNEL */

#ifndef DIVERTEXPORT
#define DIVERTEXPORT    __declspec(dllimport)
#endif      /* DIVERTEXPORT */

#ifdef __MINGW32__
#define __in
#define __out
#define __out_opt
#define __inout
#include <stdint.h>
#define INT8    int8_t
#define UINT8   uint8_t
#define INT16   int16_t
#define UINT16  uint16_t
#define INT32   int32_t
#define UINT32  uint32_t
#define INT64   int64_t
#define UINT64  uint64_t
#endif      /* __MINGW32__ */

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************************/
/* DIVERT API                                                               */
/****************************************************************************/

/*
 * Divert address.
 */
typedef struct
{
    UINT32 IfIdx;                       /* Packet's interface index. */
    UINT32 SubIfIdx;                    /* Packet's sub-interface index. */
    UINT8  Direction;                   /* Packet's direction. */
} DIVERT_ADDRESS, *PDIVERT_ADDRESS;

#define DIVERT_DIRECTION_OUTBOUND       0
#define DIVERT_DIRECTION_INBOUND        1

/*
 * Old names (deprecated).
 */
#define DIVERT_PACKET_DIRECTION_OUTBOUND                \
    DIVERT_DIRECTION_OUTBOUND
#define DIVERT_PACKET_DIRECTION_INBOUND                 \
    DIVERT_DIRECTION_INBOUND

/*
 * Divert layers.
 */
typedef enum
{
    DIVERT_LAYER_NETWORK = 0,           /* Network layer. */
    DIVERT_LAYER_NETWORK_FORWARD = 1    /* Network layer (forwarded packets) */
} DIVERT_LAYER, *PDIVERT_LAYER;

/*
 * Divert flags.
 */
#define DIVERT_FLAG_SNIFF               1
#define DIVERT_FLAG_DROP                2

/*
 * Divert parameters.
 */
typedef enum
{
    DIVERT_PARAM_QUEUE_LEN  = 0,        /* Packet queue length. */
    DIVERT_PARAM_QUEUE_TIME = 1         /* Packet queue time. */
} DIVERT_PARAM, *PDIVERT_PARAM;
#define DIVERT_PARAM_MAX                DIVERT_PARAM_QUEUE_TIME

/*
 * Open a divert handle.
 */
extern DIVERTEXPORT HANDLE DivertOpen(
    __in        const char *filter,
    __in        DIVERT_LAYER layer,
    __in        INT16 priority,
    __in        UINT64 flags);

/*
 * Receive (read) a packet from a divert handle.
 */
extern DIVERTEXPORT BOOL DivertRecv(
    __in        HANDLE handle,
    __out       PVOID pPacket,
    __in        UINT packetLen,
    __out_opt   PDIVERT_ADDRESS pAddr,
    __out_opt   UINT *readLen);

/*
 * Send (write/inject) a packet to a divert handle.
 */
extern DIVERTEXPORT BOOL DivertSend(
    __in        HANDLE handle,
    __in        PVOID pPacket,
    __in        UINT packetLen,
    __in        PDIVERT_ADDRESS pAddr,
    __out_opt   UINT *writeLen);

/*
 * Close a divert handle.
 */
extern DIVERTEXPORT BOOL DivertClose(
    __in        HANDLE handle);

/*
 * Set a divert handle parameter.
 */
extern DIVERTEXPORT BOOL DivertSetParam(
    __in        HANDLE handle,
    __in        DIVERT_PARAM param,
    __in        UINT64 value);

/*
 * Get a divert handle parameter.
 */
extern DIVERTEXPORT BOOL DivertGetParam(
    __in        HANDLE handle,
    __in        DIVERT_PARAM param,
    __out       UINT64 *pValue);

/****************************************************************************/
/* DIVERT HELPER API                                                        */
/****************************************************************************/

#ifndef DIVERT_NO_HELPER_API

/*
 * IPv4/IPv6/ICMP/ICMPv6/TCP/UDP header definitions.
 */
typedef struct
{
    UINT8  HdrLength:4;
    UINT8  Version:4;
    UINT8  TOS;
    UINT16 Length;
    UINT16 Id;
    UINT16 FragOff0;
    UINT8  TTL;
    UINT8  Protocol;
    UINT16 Checksum;
    UINT32 SrcAddr;
    UINT32 DstAddr;
} DIVERT_IPHDR, *PDIVERT_IPHDR;

#define DIVERT_IPHDR_GET_FRAGOFF(hdr)                       \
    (((hdr)->FragOff0) & 0xFF1F)
#define DIVERT_IPHDR_GET_MF(hdr)                            \
    ((((hdr)->FragOff0) & 0x0020) != 0)
#define DIVERT_IPHDR_GET_DF(hdr)                            \
    ((((hdr)->FragOff0) & 0x0040) != 0)
#define DIVERT_IPHDR_GET_RESERVED(hdr)                      \
    ((((hdr)->FragOff0) & 0x0080) != 0)

#define DIVERT_IPHDR_SET_FRAGOFF(hdr, val)                  \
    do                                                      \
    {                                                       \
        (hdr)->FragOff0 = (((hdr)->FragOff0) & 0x00E0) |    \
            ((val) & 0xFF1F);                               \
    }                                                       \
    while (FALSE)
#define DIVERT_IPHDR_SET_MF(hdr, val)                       \
    do                                                      \
    {                                                       \
        (hdr)->FragOff0 = (((hdr)->FragOff0) & 0xFFDF) |    \
            (((val) & 0x0001) << 5);                        \
    }                                                       \
    while (FALSE)
#define DIVERT_IPHDR_SET_DF(hdr, val)                       \
    do                                                      \
    {                                                       \
        (hdr)->FragOff0 = (((hdr)->FragOff0) & 0xFFBF) |    \
            (((val) & 0x0001) << 6);                        \
    }                                                       \
    while (FALSE)
#define DIVERT_IPHDR_SET_RESERVED(hdr, val)                 \
    do                                                      \
    {                                                       \
        (hdr)->FragOff0 = (((hdr)->FragOff0) & 0xFF7F) |    \
            (((val) & 0x0001) << 7);                        \
    }                                                       \
    while (FALSE)

typedef struct
{
    UINT8  TrafficClass0:4;
    UINT8  Version:4;
    UINT8  FlowLabel0:4;
    UINT8  TrafficClass1:4;
    UINT16 FlowLabel1;
    UINT16 Length;
    UINT8  NextHdr;
    UINT8  HopLimit;
    UINT32 SrcAddr[4];
    UINT32 DstAddr[4];
} DIVERT_IPV6HDR, *PDIVERT_IPV6HDR;

#define DIVERT_IPV6HDR_GET_TRAFFICCLASS(hdr)                \
    ((((hdr)->TrafficClass0) << 4) | ((hdr)->TrafficClass1))
#define DIVERT_IPV6HDR_GET_FLOWLABEL(hdr)                   \
    ((((UINT32)(hdr)->FlowLabel0) << 16) | ((UINT32)(hdr)->FlowLabel1))

#define DIVERT_IPV6HDR_SET_TRAFFICCLASS(hdr, val)           \
    do                                                      \
    {                                                       \
        (hdr)->TrafficClass0 = ((UINT8)(val) >> 4);         \
        (hdr)->TrafficClass1 = (UINT8)(val);                \
    }                                                       \
    while (FALSE)
#define DIVERT_IPV6HDR_SET_FLOWLABEL(hdr, val)              \
    do                                                      \
    {                                                       \
        (hdr)->FlowLabel0 = (UINT8)((val) >> 16);           \
        (hdr)->FlowLabel1 = (UINT16)(val);                  \
    }                                                       \
    while (FALSE)

typedef struct
{
    UINT8  Type;
    UINT8  Code;
    UINT16 Checksum;
    UINT32 Body;
} DIVERT_ICMPHDR, *PDIVERT_ICMPHDR;

typedef struct
{
    UINT8  Type;
    UINT8  Code;
    UINT16 Checksum;
    UINT32 Body;
} DIVERT_ICMPV6HDR, *PDIVERT_ICMPV6HDR;

typedef struct
{
    UINT16 SrcPort;
    UINT16 DstPort;
    UINT32 SeqNum;
    UINT32 AckNum;
    UINT16 Reserved1:4;
    UINT16 HdrLength:4;
    UINT16 Fin:1;
    UINT16 Syn:1;
    UINT16 Rst:1;
    UINT16 Psh:1;
    UINT16 Ack:1;
    UINT16 Urg:1;
    UINT16 Reserved2:2;
    UINT16 Window;
    UINT16 Checksum;
    UINT16 UrgPtr;
} DIVERT_TCPHDR, *PDIVERT_TCPHDR;

typedef struct
{
    UINT16 SrcPort;
    UINT16 DstPort;
    UINT16 Length;
    UINT16 Checksum;
} DIVERT_UDPHDR, *PDIVERT_UDPHDR;

/*
 * Flags for DivertHelperCalcChecksums()
 */
#define DIVERT_HELPER_NO_IP_CHECKSUM        1
#define DIVERT_HELPER_NO_ICMP_CHECKSUM      2
#define DIVERT_HELPER_NO_ICMPV6_CHECKSUM    4
#define DIVERT_HELPER_NO_TCP_CHECKSUM       8
#define DIVERT_HELPER_NO_UDP_CHECKSUM       16

/*
 * Parse IPv4/IPv6/ICMP/ICMPv6/TCP/UDP headers from a raw packet.
 */
extern DIVERTEXPORT BOOL DivertHelperParsePacket(
    __in        PVOID pPacket,
    __in        UINT packetLen,
    __out_opt   PDIVERT_IPHDR *ppIpHdr,
    __out_opt   PDIVERT_IPV6HDR *ppIpv6Hdr,
    __out_opt   PDIVERT_ICMPHDR *ppIcmpHdr,
    __out_opt   PDIVERT_ICMPV6HDR *ppIcmpv6Hdr,
    __out_opt   PDIVERT_TCPHDR *ppTcpHdr,
    __out_opt   PDIVERT_UDPHDR *ppUdpHdr,
    __out_opt   PVOID *ppData,
    __out_opt   UINT *pDataLen);

/*
 * Old name (deprecated).
 */
#define DivertHelperParse(p, pl, ip, ip6, icmp, icmp6, tcp, udp, d, dl)     \
    DivertHelperParsePacket((p), (pl), (ip), (ip6), (icmp), (icmp6), (tcp), \
        (udp), (d), (dl))

/*
 * Parse an IPv4 address.
 */
extern DIVERTEXPORT BOOL DivertHelperParseIPv4Address(
    __in        const char *addrStr,
    __out_opt   UINT32 *pAddr);

/*
 * Parse an IPv6 address.
 */
extern DIVERTEXPORT BOOL DivertHelperParseIPv6Address(
    __in        const char *addrStr,
    __out_opt   UINT32 *pAddr);

/*
 * Calculate IPv4/IPv6/ICMP/ICMPv6/TCP/UDP checksums.
 */
extern DIVERTEXPORT UINT DivertHelperCalcChecksums(
    __inout     PVOID pPacket, 
    __in        UINT packetLen,
    __in        UINT64 flags);

#endif      /* DIVERT_NO_HELPER_API */

#ifdef __cplusplus
}
#endif

#endif      /* __DIVERT_H */
