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

#ifndef _DCCADB_H__
#define _DCCADB_H__

#ifdef __cplusplus
extern "C" {
#endif

DWORD
CdcDbInitialize(
    PCSTR     pszDbPath
    );

DWORD
DccaDbGetHAClientState(
    PBOOL    pbHAState
    );

DWORD
DccaDbSetHAClientState(
    BOOL    bHAState
    );

DWORD
DccaDbGetDomainName(
    PWSTR*   ppwszDomain        /*    OUT */
    );

DWORD
DccaDbSetDomainName(
    PWSTR    pwszDomain        /* IN     */
    );

DWORD
DccaDbGetDCName(
    PWSTR*   ppwszDCName    /*    OUT */
    );

DWORD
DccaDbSetDCName(
    PWSTR    pwszDCName     /* IN     */
    );

DWORD
DccaDbGetRefreshInterval(
    PDWORD pdwRefreshInterval        /* IN */
    );

DWORD
DccaDbGetHeartBeatInterval(
    PDWORD pdwHeartBeatInterval        /* IN */
    );

DWORD
DccaDbAddDCEntry(
    PDCCA_DC_ENTRY_W pDccaEntry
    );

DWORD
DccaDbDeleteDCEntry(
    PWSTR pszDCName
    );

DWORD
DccaDbEnumDCEntries(
    //TODO: Replace this with actual DC entry struct
    PWSTR **pppszEntryNames,
    PDWORD pdwCount
    );

#ifdef __cplusplus
}
#endif

#endif /* _DCCADB_H__ */


