/*
 * Copyright (C) 2014 VMware, Inc. All rights reserved.
 *
 * Module   : vmeventclient.h
 *
 * Abstract :
 *
 */
#ifndef _REPOCLIENT_H_
#define _REPOCLIENT_H_


#include <vmeventcommon.h>

#ifdef __cplusplus
extern "C" {
#endif

DWORD
EventLogInitialize();

DWORD
EventLogAdd(
	RP_PCSTR pszServerName,
	DWORD dwEventID,
    DWORD dwEventType,
    RP_PCSTR pszMessage
	);

DWORD
EventLogInitEnumEventsHandle(
	RP_PCSTR pszServerName,
	PDWORD pdwHandle
	);

DWORD
EventLogEnumEvents(
	RP_PCSTR pszServerName,
	DWORD    dwHandle,
	DWORD    dwStartIndex,
	DWORD    dwNumPackages,
	PEVENTLOG_CONTAINER * ppEventContainer
	);


#ifdef __cplusplus
};
#endif


#endif // _REPOCLIENT_H_
