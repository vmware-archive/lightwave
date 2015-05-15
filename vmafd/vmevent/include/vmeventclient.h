#ifndef _REPOCLIENT_H_
#define _REPOCLIENT_H_


#include <vmeventcommon.h>
#include <vmevent_h.h>

#ifdef __cplusplus
extern "C" {
#endif

DWORD
EventLogInitialize();

DWORD
EventLogAdd(
	RP_PCSTR pszServerName,
	RP_PCSTR pszServerEndpoint,
	DWORD dwEventID,
        DWORD dwEventType,
        RP_PCSTR pszMessage
	);
DWORD
EventLogInitEnumEventsHandle(
	RP_PCSTR pszServerName,
	RP_PCSTR pszServerEndpoint,
	PDWORD pdwHandle
	);

DWORD
EventLogEnumEvents(
	RP_PCSTR pszServerName,
	RP_PCSTR pszServerEndpoint,
	DWORD    dwHandle,
	DWORD    dwStartIndex,
	DWORD    dwNumPackages,
	PEVENTLOG_CONTAINER * ppEventContainer
	);


#ifdef __cplusplus
};
#endif


#endif // _REPOCLIENT_H_
