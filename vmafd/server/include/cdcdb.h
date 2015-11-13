/*
 * Copyright (c) VMware Inc.  All rights Reserved.
 */

/*
 * Module Name: VMware Certificate Service
 *
 * Filename: vmcadb.h
 *
 * Abstract:
 *
 * VMware Certificate Service Database
 *
 */

#ifndef _CDCDB_H__
#define _CDCDB_H__

#ifdef __cplusplus
extern "C" {
#endif

DWORD
CdcDbInitialize(
    PCSTR     pszDbPath
    );

DWORD
CdcRegDbGetHAMode(
    PBOOL    pbHAState
    );

DWORD
CdcRegDbSetHAMode(
    BOOL    bHAState
    );

DWORD
CdcRegDbGetDomainName(
    PWSTR*   ppwszDomain        /*    OUT */
    );

DWORD
CdcRegDbSetDomainName(
    PCWSTR    pwszDomain        /* IN     */
    );

DWORD
CdcRegDbGetSiteName(
    PWSTR*   ppwszSite        /*    OUT */
    );

DWORD
CdcRegDbSetSiteName(
    PCWSTR    pwszSite        /* IN     */
    );

DWORD
CdcRegDbGetRefreshInterval(
    PDWORD pdwRefreshInterval        /* IN */
    );

DWORD
CdcRegDbGetHeartBeatInterval(
    PDWORD pdwHeartBeatInterval        /* IN */
    );

DWORD
CdcDbSetHAClientState(
    DWORD dwHAState
    );

DWORD
CdcDbGetHAClientState(
    PDWORD pdwHAState
    );

DWORD
CdcDbGetAffinitizedDC(
    PCWSTR           pwszDomainName,
    PDWORD           pdwAffinitizedSince,
    PCDC_DC_INFO_W*  ppAffinitizedDC
    );

DWORD
CdcDbAddAffinitizedDC(
    PCWSTR          pszAffinitizedDC,
    PCWSTR          pszDomainName
    );

DWORD
CdcDbAddDCEntry(
    PCDC_DB_ENTRY_W  pCdcEntry
    );

DWORD
CdcDbUpdateDCEntry(
    PCDC_DB_ENTRY_W  pCdcEntry
    );

DWORD
CdcDbIsDCAlive(
    PCDC_DC_INFO_W pDcInfo,
    BOOL* pbIsAlive
    );

DWORD
CdcDbDeleteDCEntry(
    PCWSTR pszDCName
    );

DWORD
CdcDbEnumDCEntries(
    PWSTR **pppszEntryNames,
    PDWORD pdwCount
    );

BOOL
CdcIsAffinitizedDC(
    PCWSTR pszDCName,
    PCWSTR pszDomainName
    );

DWORD
CdcDbGetClosestDCOnSite(
     PCWSTR pwszClientSiteName,
     PCWSTR pwszDomainName,
     PWSTR *ppszDCName
     );

DWORD
CdcDbGetClosestDC(
     PCWSTR pwszDomainName,
     PWSTR *ppszDCName
     );

#ifdef __cplusplus
}
#endif

#endif /* _CDCDB_H__ */


