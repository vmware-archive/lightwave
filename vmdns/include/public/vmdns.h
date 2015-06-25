/*
* Copyright (c) VMware Inc. 2011  All rights Reserved.
*
* Module Name:  vmdns.h
*
* Abstract: VMware Domain Name Service.
*
* Created on: Sep 18, 2012
*
*/


#ifndef VMDNS_H_
#define VMDNS_H_


#ifdef __cplusplus
extern "C" {
#endif

#include "vmdnstypes.h"

#ifndef TRUE
#define TRUE  1
#endif

#ifndef FALSE
#define FALSE 0
#endif

    // Logging stuff
#define MAX_LOG_MESSAGE_LEN    4096

#ifndef _WIN32
#define VMDNS_NCALRPC_END_POINT "vmdnssvc"
#else
    // note: keep in sync with /vmdns/main/idl/vmdns.idl
#define VMDNS_NCALRPC_END_POINT "VMWareDomainNameService"
#endif

#define VMDNS_RPC_TCP_END_POINT "2015"
#define VMDNS_RPC_TCP_END_POINTW { '2', '0', '1', '5', '0' }
#define VMDNS_MAX_SERVER_ID     255
    // Logging stuff
#define MAX_LOG_MESSAGE_LEN    4096

#include <vmdnstypes.h>

#ifdef _WIN32
#ifdef LIBVMDNSCLIENT_EXPORTS
#define VMDNS_API __declspec(dllexport)
#else
#define VMDNS_API __declspec(dllimport)
#endif
#else
#define VMDNS_API
#endif

typedef struct _VMDNS_SERVER_CONTEXT VMDNS_SRVER_CONTEXT, *PVMDNS_SERVER_CONTEXT;

VMDNS_API
DWORD
VmDnsOpenServerW(
    PCWSTR                  pwszNetworkAddress,
    PCWSTR                  pwszUserName,
    PCWSTR                  pwszDomain,
    PCWSTR                  pwszPassword,
    DWORD                   dwFlags,
    PVOID                   pReserved,
    PVMDNS_SERVER_CONTEXT   *pServerContext
    );

VMDNS_API
DWORD
VmDnsOpenServerA(
    PCSTR                   pszNetworkAddress,
    PCSTR                   pszUserName,
    PCSTR                   pszDomain,
    PCSTR                   pszPassword,
    DWORD                   dwFlags,
    PVOID                   pReserved,
    PVMDNS_SERVER_CONTEXT   *pServerContext
    );

VMDNS_API
VOID
VmDnsCloseServer(
    PVMDNS_SERVER_CONTEXT   pServerContext
    );

VMDNS_API
DWORD
VmDnsGetForwardersA(
    PVMDNS_SERVER_CONTEXT   pServerContext,
    PSTR**                  pppszForwarders,
    PDWORD                  pdwCount
    );

VMDNS_API
DWORD
VmDnsAddForwarderA(
    PVMDNS_SERVER_CONTEXT   pServerContext,
    PSTR                    pszForwarders
    );

VMDNS_API
DWORD
VmDnsDeleteForwarderA(
    PVMDNS_SERVER_CONTEXT   pServerContext,
    PSTR                    pszForwarders
    );

VMDNS_API
DWORD
VmDnsInitializeA(
    PVMDNS_SERVER_CONTEXT   pServerContext,
    PVMDNS_INIT_INFO        pInitInfo
    );

VMDNS_API
DWORD
VmDnsUninitializeA(
    PVMDNS_SERVER_CONTEXT   pServerContext,
    PVMDNS_INIT_INFO        pInitInfo
    );

VMDNS_API
DWORD
VmDnsCreateZoneA(
    PVMDNS_SERVER_CONTEXT   pServerContext,
    PVMDNS_ZONE_INFO        pZoneInfo
    );

VMDNS_API
DWORD
VmDnsUpdateZoneA(
    PVMDNS_SERVER_CONTEXT   pServerContext,
    PVMDNS_ZONE_INFO        pZoneInfo
    );

VMDNS_API
DWORD
VmDnsListZoneA(
    PVMDNS_SERVER_CONTEXT   pServerContext,
    PVMDNS_ZONE_INFO_ARRAY *ppZoneInfo
    );

VMDNS_API
DWORD
VmDnsDeleteZoneA(
    PVMDNS_SERVER_CONTEXT   pServerContext,
    PSTR pszZone
    );

VMDNS_API
DWORD
VmDnsAddRecordA(
    PVMDNS_SERVER_CONTEXT   pServerContext,
    PSTR                    pszZone,
    PVMDNS_RECORD           pRecord
    );

VMDNS_API
DWORD
VmDnsDeleteRecordA(
    PVMDNS_SERVER_CONTEXT   pServerContext,
    PSTR                    pszZone,
    PVMDNS_RECORD           pRecord
    );

VMDNS_API
DWORD
VmDnsQueryRecordsA(
    PVMDNS_SERVER_CONTEXT   pServerContext,
    PSTR                    pszZone,
    PSTR                    pszName,
    VMDNS_RR_TYPE           dwType,
    DWORD                   dwOptions,
    PVMDNS_RECORD_ARRAY *   ppRecordArray
    );

VMDNS_API
DWORD
VmDnsListRecordsA(
    PVMDNS_SERVER_CONTEXT   pServerContext,
    PSTR                    pszZone,
    PVMDNS_RECORD_ARRAY *   ppRecordArray
    );

VMDNS_API
VOID
VmDnsFreeZoneInfo(
    PVMDNS_ZONE_INFO        pZoneInfo
    );

VMDNS_API
VOID
VmDnsFreeZoneInfoArray(
    PVMDNS_ZONE_INFO_ARRAY  pZoneInfoArray
    );

VMDNS_API
VOID
VmDnsFreeRecords(
    PVMDNS_RECORD           pRecord
    );

VMDNS_API
VOID
VmDnsFreeRecordArray(
    PVMDNS_RECORD_ARRAY     pRecordArray
    );

#ifdef __cplusplus
}
#endif

#endif /* VMDNS_H_ */
