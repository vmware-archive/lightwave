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

typedef enum
{
    CDC_DB_ENUM_FILTER_UNDEFINED = 0,
    CDC_DB_ENUM_FILTER_ON_SITE,
    CDC_DB_ENUM_FILTER_ON_SITE_AND_ACTIVE,
    CDC_DB_ENUM_FILTER_OFF_SITE
} CDC_DB_ENUM_FILTER, *PCDC_DB_ENUM_FILTER;

typedef enum
{
    CDC_DB_ENUM_HA_MODE_UNDEFINED = 0,
    CDC_DB_ENUM_HA_MODE_LEGACY,
    CDC_DB_ENUM_HA_MODE_DEFAULT
} CDC_DB_ENUM_HA_MODE, *PCDC_DB_ENUM_HA_MODE;

DWORD
CdcDbInitialize(
    PCSTR     pszDbPath
    );

DWORD
CdcRegDbGetHAMode(
    PCDC_DB_ENUM_HA_MODE pCdcHAMode
    );

DWORD
CdcRegDbSetHAMode(
    CDC_DB_ENUM_HA_MODE cdcHAMode
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
CdcRegDbSetDCNameHA(
    PCWSTR    pwszDCName        /* IN     */
    );

DWORD
CdcRegDbRemoveDCNameHA(
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
CdcDbUpdateDCEntryWithSite(
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
    PCDC_DB_ENTRY_W *ppEntries,
    PDWORD pdwCount
    );

DWORD
CdcDbEnumDCEntriesFiltered(
    DWORD dwFilter,
    PWSTR pwszFilterString,
    PCDC_DB_ENTRY_ARRAY *ppCdcDbEntryArray
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

DWORD
CdcDbUpdateHeartbeatStatus(
    PCDC_DB_ENTRY_W pCdcDbEntry,
    PVMAFD_HB_STATUS_W pHeartbeatStatus
    );

DWORD
CdcDbGetHeartbeatStatus(
    PWSTR pwszDCName,
    PWSTR pwszDomainName,
    PVMAFD_HB_STATUS_W *ppHeartbeatStatus
    );

DWORD
CdcDbGetDCInfo(
    PWSTR pwszDCName,
    PWSTR pwszDomainToUse,
    PCDC_DC_STATUS_INFO_W *ppCdcStatusInfo
    );

DWORD
CdcDbDeleteHeartbeatStatus(
    PCDC_DB_ENTRY_W pCdcDbEntry
    );

VOID
VmAfdFreeCdcDbEntriesW(
    PCDC_DB_ENTRY_W pCdcDbEntry,
    DWORD dwCount
    );

VOID
VmAfdFreeCdcDbEntry(
    PCDC_DB_ENTRY_W pCdcDbEntry
    );

VOID
VmAfdFreeCdcDbEntryArray(
    PCDC_DB_ENTRY_ARRAY pCdcDbEntryArray
    );

#ifdef __cplusplus
}
#endif

#endif /* _CDCDB_H__ */


