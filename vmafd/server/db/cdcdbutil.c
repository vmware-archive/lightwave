/*
 * Copyright © 2012-2016 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the “License”); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an “AS IS” BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */

#include "includes.h"

static
DWORD
CdcDbGetDCEntryCount(
    PVECS_DB_CONTEXT pDbContext,
    PDWORD pdwDCCount
    );

static
DWORD
CdcDbRemoveExistingDC(
    PVECS_DB_CONTEXT pDbContext,
    PCWSTR pszDomainName
    );

static
VOID
VmAfdFreeCdcDbArrayEntries(
    DWORD dwCount,
    PCDC_DB_ENTRY_W *ppEntries
    );

static
DWORD
CdcDbEnumDCGetData(
    sqlite3_stmt* pDbQuery,
    DWORD dwExpectedCount,
    PCDC_DB_ENTRY_W **pppEntries,
    PDWORD pdwCount
    );

static
DWORD
CdcDbEnumDCEntriesSiteOnly(
    PWSTR pwszSite,
    PCDC_DB_ENTRY_ARRAY *ppCdcDbEntryArray
    );

static
DWORD
CdcDbEnumDCEntriesSiteAndActive(
    PWSTR pwszSite,
    PCDC_DB_ENTRY_ARRAY *ppCdcDbEntryArray
    );

static
DWORD
CdcDbEnumDCEntriesOffsite(
    PWSTR pwszSite,
    PCDC_DB_ENTRY_ARRAY *ppCdcDbEntryArray
    );

static
DWORD
CdcDbUpdateServiceStatus(
    PVECS_DB_CONTEXT pDbContext,
    PCDC_DB_ENTRY_W  pCdcDbEntry,
    PVMAFD_HB_INFO_W pHeartbeatInfo
    );

static
DWORD
CdcDbGetHeartbeatStatusCount(
    PVECS_DB_CONTEXT pDbContext,
    PWSTR  pwszDCName,
    PWSTR  pwszDomainName,
    PDWORD pdwCount
    );

DWORD
CdcRegDbGetHAMode(
    PCDC_DB_ENUM_HA_MODE pCdcHAMode
    )
{
    DWORD dwError = 0;
    DWORD dwHAState = 0;
    CDC_DB_ENUM_HA_MODE cdcHAMode = CDC_DB_ENTRY_STATUS_UNDEFINED;

    if (!pCdcHAMode)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdRegGetInteger(
                                VMAFD_REG_VALUE_HA_CONFIG,
                                &dwHAState
                                );
    if (dwError == ERROR_OBJECT_NOT_FOUND ||
        dwError == ERROR_FILE_NOT_FOUND)
    {
        dwError = 0;
        dwHAState = 0;
        //IF the registry setting is not there, we should assume defaultHA mode. Right?
    }
    BAIL_ON_VMAFD_ERROR(dwError);

    cdcHAMode = dwHAState?
                CDC_DB_ENUM_HA_MODE_LEGACY :
                CDC_DB_ENUM_HA_MODE_DEFAULT;

    *pCdcHAMode = cdcHAMode;

cleanup:
    return dwError;

error:
    if (pCdcHAMode)
    {
        *pCdcHAMode = 0;
    }

    goto cleanup;
}

DWORD
CdcRegDbSetHAMode(
    CDC_DB_ENUM_HA_MODE cdcHAMode
    )
{
    DWORD dwError = 0;
    BOOL  bHAState = FALSE;

    if (cdcHAMode == CDC_DB_ENUM_HA_MODE_LEGACY)
    {
        bHAState = TRUE;
    }

    dwError = VmAfdRegSetInteger(
                              VMAFD_REG_VALUE_HA_CONFIG,
                              (DWORD)bHAState
                              );
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    return dwError;

error:
    goto cleanup;
}

DWORD
CdcRegDbGetDomainName(
    PWSTR*   ppwszDomain        /*    OUT */
    )
{
    DWORD dwError = 0;
    PWSTR pwszDomain = NULL;

    if (!ppwszDomain)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdRegGetString(VMAFD_CONFIG_PARAMETER_KEY_PATH,
                               VMAFD_REG_KEY_DOMAIN_NAME,
                               &pwszDomain);
    BAIL_ON_VMAFD_ERROR(dwError);

    *ppwszDomain = pwszDomain;

cleanup:

    return dwError;
error:

    if (ppwszDomain)
    {
        *ppwszDomain = NULL;
    }
    VMAFD_SAFE_FREE_MEMORY(pwszDomain);
    goto cleanup;
}

DWORD
CdcRegDbSetDomainName(
    PCWSTR    pwszDomain        /* IN     */
    )
{
    DWORD dwError = 0;

    if (!pwszDomain)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdRegSetString(VMAFD_CONFIG_PARAMETER_KEY_PATH,
                               VMAFD_REG_KEY_DOMAIN_NAME,
                               (PCWSTR)pwszDomain);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    return dwError;
error:

    goto cleanup;
}

DWORD
CdcRegDbGetSiteName(
    PWSTR*   ppwszSite        /*    OUT */
    )
{
    DWORD dwError = 0;
    PWSTR pwszSite = NULL;

    if (!ppwszSite)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdRegGetString(VMAFD_CONFIG_PARAMETER_KEY_PATH,
                               VMAFD_REG_KEY_SITE_NAME,
                               &pwszSite);
    BAIL_ON_VMAFD_ERROR(dwError);

    *ppwszSite = pwszSite;

cleanup:

    return dwError;
error:

    if (ppwszSite)
    {
        *ppwszSite = NULL;
    }
    VMAFD_SAFE_FREE_MEMORY(pwszSite);
    goto cleanup;
}

DWORD
CdcRegDbSetSiteName(
    PCWSTR    pwszSite        /* IN     */
    )
{
    DWORD dwError = 0;

    if (!pwszSite)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdRegSetString(VMAFD_CONFIG_PARAMETER_KEY_PATH,
                               VMAFD_REG_KEY_SITE_NAME,
                               (PCWSTR)pwszSite);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    return dwError;
error:

    goto cleanup;
}

DWORD
CdcRegDbSetDCNameHA(
    PCWSTR    pwszDCName        /* IN     */
    )
{
    DWORD dwError = 0;

    if (!pwszDCName)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdRegSetString(VMAFD_CONFIG_PARAMETER_KEY_PATH,
                               VMAFD_REG_KEY_DC_NAME_HA,
                               (PCWSTR)pwszDCName);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    return dwError;
error:

    goto cleanup;
}

DWORD
CdcRegDbRemoveDCNameHA(
    )
{
    DWORD dwError = 0;

    dwError = VmAfdRegDeleteValue(
                          VMAFD_CONFIG_PARAMETER_KEY_PATH,
                          VMAFD_REG_KEY_DC_NAME_HA
                          );
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    return dwError;
error:

    goto cleanup;
}


DWORD
CdcRegDbGetRefreshInterval(
    PDWORD pdwRefreshInterval        /* IN */
    )
{
    DWORD dwError = 0;
    DWORD dwValue = 0;

    if (!pdwRefreshInterval)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdRegGetInteger(VMAFD_REG_KEY_DCCACHE_SYNC,
                                &dwValue);
    BAIL_ON_VMAFD_ERROR(dwError);

    *pdwRefreshInterval = dwValue;

cleanup:

    return dwError;
error:

    if (pdwRefreshInterval)
    {
        *pdwRefreshInterval = 0;
    }
    goto cleanup;
}

DWORD
CdcRegDbGetHeartBeatInterval(
    PDWORD pdwHeartBeatInterval        /* IN */
    )
{
    DWORD dwError = 0;
    DWORD dwValue = 0;

    if (!pdwHeartBeatInterval)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdRegGetInteger(VMAFD_REG_KEY_DC_HEARTBEAT,
                                &dwValue);
    BAIL_ON_VMAFD_ERROR(dwError);

    *pdwHeartBeatInterval = dwValue;

cleanup:

    return dwError;
error:

    if (pdwHeartBeatInterval)
    {
        *pdwHeartBeatInterval = 0;
    }
    goto cleanup;
}

DWORD
CdcDbSetHAClientState(
    DWORD dwHAState
    )
{
    DWORD dwError = 0;
    PVECS_DB_CONTEXT pDbContext = NULL;
    sqlite3_stmt* pDbQuery = NULL;

    char szQuery[] = "INSERT INTO AfdProperties ("
                     " Property,"
                     " Value)"
                     " VALUES(\"cdcState\", :state);";

    dwError = VecsDbCreateContext(&pDbContext, VMAFD_DB_MODE_WRITE);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = sqlite3_prepare_v2(
                          pDbContext->pDb,
                          szQuery,
                          -1,
                          &pDbQuery,
                          NULL
                          );
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VecsBindDword(
                        pDbQuery,
                        ":state",
                        dwHAState
                        );
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VecsDbStepSql(pDbQuery);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    if (pDbQuery)
    {
        sqlite3_reset(pDbQuery);
        sqlite3_finalize(pDbQuery);
    }
    if (pDbContext)
    {
        VecsDbReleaseContext(pDbContext);
    }
    return dwError;
error:

    goto cleanup;
}

DWORD
CdcDbGetHAClientState(
    PDWORD pdwHAState
    )
{
    DWORD dwError = 0;
    DWORD dwHAState = 0;
    PVECS_DB_CONTEXT pDbContext = NULL;
    sqlite3_stmt* pDbQuery = NULL;
    DWORD dwCount = 0;

    char szQuery[] = "SELECT Value from AfdProperties"
                     " WHERE Property = \"cdcState\"";

    if (!pdwHAState)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VecsDbCreateContext(&pDbContext, VMAFD_DB_MODE_READ);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = sqlite3_prepare_v2(
                          pDbContext->pDb,
                          szQuery,
                          -1,
                          &pDbQuery,
                          NULL
                          );
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VecsDbStepSql(pDbQuery);
    if (dwError == SQLITE_ROW)
    {
        dwError = VecsDBGetColumnInt(
                                pDbQuery,
                                "Value",
                                &dwHAState
                                );
        BAIL_ON_VMAFD_ERROR(dwError);
        ++dwCount;
    }
    else if (dwError == SQLITE_DONE || !dwCount)
    {
        dwError = ERROR_OBJECT_NOT_FOUND;
    }
    BAIL_ON_VMAFD_ERROR_NO_LOG(dwError);

    *pdwHAState = dwHAState;

cleanup:

    if (pDbQuery)
    {
        sqlite3_reset(pDbQuery);
        sqlite3_finalize(pDbQuery);
    }
    if (pDbContext)
    {
        VecsDbReleaseContext(pDbContext);
    }
    return dwError;
error:
    if (pdwHAState)
    {
        *pdwHAState = 0;
    }

    goto cleanup;
}

DWORD
CdcDbGetAffinitizedDC(
    PCWSTR           pwszDomainName,
    PDWORD           pdwAffinitizedSince,
    PCDC_DC_INFO_W*  ppAffinitizedDC
    )
{
    DWORD dwError = 0;
    PCDC_DC_INFO_W pAffinitizedDC = NULL;
    DWORD dwAffinitizedSince = 0;
    PVECS_DB_CONTEXT pDbContext = NULL;
    sqlite3_stmt* pDbQuery = NULL;
    DWORD dwCount = 0;

    char szQuery[] = "SELECT * from DCTable NATURAL JOIN AffinitizedDC"
                     " WHERE Domain = :domainName;";

    if (!ppAffinitizedDC)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VecsDbCreateContext(&pDbContext, VMAFD_DB_MODE_READ);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = sqlite3_prepare_v2(
                          pDbContext->pDb,
                          szQuery,
                          -1,
                          &pDbQuery,
                          NULL
                          );
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VecsBindWideString(
                            pDbQuery,
                            ":domainName",
                            IsNullOrEmptyString(pwszDomainName)?
                            NULL:
                            pwszDomainName
                            );
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VecsDbStepSql(pDbQuery);

    if (dwError == SQLITE_ROW)
    {
        dwError = VmAfdAllocateMemory(
                              sizeof(CDC_DC_INFO_W),
                              (PVOID*)&pAffinitizedDC
                              );
        BAIL_ON_VMAFD_ERROR(dwError);

        dwError = VecsDBGetColumnString(
                                pDbQuery,
                                "DCName",
                                &pAffinitizedDC->pszDCName
                                );
        BAIL_ON_VMAFD_ERROR(dwError);

        dwError = VecsDBGetColumnString(
                                pDbQuery,
                                "Site",
                                &pAffinitizedDC->pszDcSiteName
                                );
        BAIL_ON_VMAFD_ERROR(dwError);

        dwError = VecsDBGetColumnString(
                                pDbQuery,
                                "Domain",
                                &pAffinitizedDC->pszDomainName
                                );
        BAIL_ON_VMAFD_ERROR(dwError);

        dwError = VecsDBGetColumnInt(
                                pDbQuery,
                                "AffinitizedSince",
                                &dwAffinitizedSince
                                );
        BAIL_ON_VMAFD_ERROR(dwError);

        ++dwCount;
    }
    else if (dwError == SQLITE_DONE || !dwCount)
    {
        dwError = ERROR_OBJECT_NOT_FOUND;
    }
    BAIL_ON_VMAFD_ERROR_NO_LOG(dwError);

    *ppAffinitizedDC = pAffinitizedDC;
    *pdwAffinitizedSince = dwAffinitizedSince;

cleanup:

    if (pDbQuery)
    {
        sqlite3_reset(pDbQuery);
        sqlite3_finalize(pDbQuery);
    }
    if (pDbContext)
    {
        VecsDbReleaseContext(pDbContext);
    }
    return dwError;
error:

    if (pAffinitizedDC)
    {
        VmAfdFreeDomainControllerInfoW(pAffinitizedDC);
    }
    if (pdwAffinitizedSince)
    {
        *pdwAffinitizedSince = 0;
    }
    *ppAffinitizedDC = NULL;
    goto cleanup;
}

DWORD
CdcDbAddAffinitizedDC(
    PCWSTR          pszAffinitizedDC,
    PCWSTR          pszDomainName
    )
{
    DWORD dwError = 0;
    PVECS_DB_CONTEXT pDbContext = NULL;
    sqlite3_stmt* pDbQuery = NULL;

    char szQuery[] = "INSERT INTO AffinitizedDC("
                     " DCID,"
                     " Domain)"
                     " VALUES ("
                     " (SELECT DCID from DCTable WHERE"
                     " DCName = :dcName AND"
                     " Domain = :domainName),"
                     " :domainName);";

    if (IsNullOrEmptyString(pszAffinitizedDC))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VecsDbCreateContext(&pDbContext, VMAFD_DB_MODE_WRITE);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (!pDbContext->bInTx)
    {
        dwError = VecsDbBeginTransaction(pDbContext->pDb);
        BAIL_ON_VMAFD_ERROR(dwError);

        pDbContext->bInTx = TRUE;
    }

    dwError = CdcDbRemoveExistingDC(
                              pDbContext,
                              pszDomainName
                              );
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = sqlite3_prepare_v2(
                          pDbContext->pDb,
                          szQuery,
                          -1,
                          &pDbQuery,
                          NULL
                          );
    BAIL_ON_VMAFD_ERROR(dwError);


    dwError = VecsBindWideString(
                            pDbQuery,
                            ":dcName",
                            pszAffinitizedDC
                            );
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VecsBindWideString(
                            pDbQuery,
                            ":domainName",
                            pszDomainName
                            );
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VecsDbStepSql(pDbQuery);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VecsDbCommitTransaction(pDbContext->pDb);
    BAIL_ON_VMAFD_ERROR(dwError);

    pDbContext->bInTx = FALSE;

cleanup:

    if (pDbQuery)
    {
        sqlite3_reset(pDbQuery);
        sqlite3_finalize(pDbQuery);
    }
    if (pDbContext)
    {
        VecsDbReleaseContext(pDbContext);
    }
    return dwError;
error:

    if (pDbContext && pDbContext->bInTx)
    {
        VecsDbRollbackTransaction(pDbContext->pDb);
        pDbContext->bInTx = FALSE;
    }

    goto cleanup;
}

BOOL
CdcIsAffinitizedDC(
    PCWSTR pszDCName,
    PCWSTR pszDomainName
    )
{
    DWORD dwError = 0;
    PVECS_DB_CONTEXT pDbContext = NULL;
    sqlite3_stmt* pDbQuery = NULL;
    BOOL bIsAffinitized = FALSE;

    char szQuery[] = " SELECT * FROM DCTable"
                     " WHERE DCID = ("
                     " SELECT DCID from AffinitizedDC"
                     " WHERE Domain = :domainName)"
                     " AND DCName = :dcName;";

    if (IsNullOrEmptyString(pszDCName) ||
        IsNullOrEmptyString(pszDomainName)
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VecsDbCreateContext(&pDbContext, VMAFD_DB_MODE_READ);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = sqlite3_prepare_v2(
                          pDbContext->pDb,
                          szQuery,
                          -1,
                          &pDbQuery,
                          NULL
                          );
    BAIL_ON_VMAFD_ERROR(dwError);


    dwError = VecsBindWideString(
                            pDbQuery,
                            ":dcName",
                            pszDCName
                            );
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VecsBindWideString(
                           pDbQuery,
                           ":domainName",
                           pszDomainName
                           );
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VecsDbStepSql(pDbQuery);

    if (dwError == SQLITE_ROW)
    {
        bIsAffinitized = TRUE;
        dwError = 0;
    }
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    if (pDbQuery)
    {
        sqlite3_reset(pDbQuery);
        sqlite3_finalize(pDbQuery);
    }
    if (pDbContext)
    {
        VecsDbReleaseContext(pDbContext);
    }
    return bIsAffinitized;
error:

    goto cleanup;
}

DWORD
CdcDbAddDCEntry(
    PCDC_DB_ENTRY_W  pCdcEntry
    )
{
    DWORD dwError = 0;
    PVECS_DB_CONTEXT pDbContext = NULL;
    sqlite3_stmt* pDbQuery = NULL;

    char szQuery[] = "INSERT INTO DCTable("
                     " DCName,"
                     " Site,"
                     " Domain,"
                     " LastPing,"
                     " PingResponse,"
                     " PingError,"
                     " IsAlive)"
                     " VALUES ("
                     " :dcName,"
                     " :site,"
                     " :domainName,"
                     " :lastPing,"
                     " :pingResponse,"
                     " :pingError,"
                     " :isAlive);";

    if (!pCdcEntry)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VecsDbCreateContext(&pDbContext, VMAFD_DB_MODE_WRITE);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = sqlite3_prepare_v2(
                          pDbContext->pDb,
                          szQuery,
                          -1,
                          &pDbQuery,
                          NULL
                          );
    BAIL_ON_VMAFD_ERROR(dwError);


    dwError = VecsBindWideString(
                            pDbQuery,
                            ":dcName",
                            pCdcEntry->pszDCName
                            );
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VecsBindWideString(
                            pDbQuery,
                            ":site",
                            pCdcEntry->pszSiteName
                            );
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VecsBindWideString(
                            pDbQuery,
                            ":domainName",
                            pCdcEntry->pszDomainName
                            );
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VecsBindDword(
                        pDbQuery,
                        ":lastPing",
                        pCdcEntry->dwLastPing
                        );
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VecsBindDword(
                         pDbQuery,
                         ":pingResponse",
                         pCdcEntry->dwPingTime
                         );
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VecsBindDword(
                         pDbQuery,
                         ":pingError",
                         pCdcEntry->dwLastError
                         );
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VecsBindDword(
                        pDbQuery,
                        ":isAlive",
                        pCdcEntry->bIsAlive
                        );
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VecsDbStepSql(pDbQuery);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    if (pDbQuery)
    {
        sqlite3_reset(pDbQuery);
        sqlite3_finalize(pDbQuery);
    }
    if (pDbContext)
    {
        VecsDbReleaseContext(pDbContext);
    }
    return dwError;
error:

    goto cleanup;
}

DWORD
CdcDbUpdateDCEntry(
    PCDC_DB_ENTRY_W  pCdcEntry
    )
{
    DWORD dwError = 0;
    PVECS_DB_CONTEXT pDbContext = NULL;
    sqlite3_stmt* pDbQuery = NULL;

    char szQuery[] = "UPDATE DCTable"
                     " SET LastPing = :lastPing,"
                     " PingResponse = :pingResponse,"
                     " PingError = :pingError,"
                     " IsAlive = :isAlive"
                     " WHERE"
                     " DCName = :dcName AND"
                     " Domain = :domainName;";

    if (!pCdcEntry)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VecsDbCreateContext(&pDbContext, VMAFD_DB_MODE_WRITE);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = sqlite3_prepare_v2(
                          pDbContext->pDb,
                          szQuery,
                          -1,
                          &pDbQuery,
                          NULL
                          );
    BAIL_ON_VMAFD_ERROR(dwError);


    dwError = VecsBindWideString(
                            pDbQuery,
                            ":dcName",
                            pCdcEntry->pszDCName
                            );
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VecsBindWideString(
                            pDbQuery,
                            ":domainName",
                            pCdcEntry->pszDomainName
                            );
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VecsBindDword(
                        pDbQuery,
                        ":lastPing",
                        pCdcEntry->dwLastPing
                        );
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VecsBindDword(
                         pDbQuery,
                         ":pingResponse",
                         pCdcEntry->dwPingTime
                         );
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VecsBindDword(
                         pDbQuery,
                         ":pingError",
                         pCdcEntry->dwLastError
                         );
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VecsBindDword(
                        pDbQuery,
                        ":isAlive",
                        pCdcEntry->bIsAlive
                        );
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VecsDbStepSql(pDbQuery);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    if (pDbQuery)
    {
        sqlite3_reset(pDbQuery);
        sqlite3_finalize(pDbQuery);
    }
    if (pDbContext)
    {
        VecsDbReleaseContext(pDbContext);
    }
    return dwError;
error:

    goto cleanup;
}

DWORD
CdcDbUpdateDCEntryWithSite(
    PCDC_DB_ENTRY_W  pCdcEntry
    )
{
    DWORD dwError = 0;
    PVECS_DB_CONTEXT pDbContext = NULL;
    sqlite3_stmt* pDbQuery = NULL;

    char szQuery[] = "UPDATE DCTable"
                     " SET LastPing = :lastPing,"
                     " PingResponse = :pingResponse,"
                     " PingError = :pingError,"
                     " IsAlive = :isAlive,"
                     " Site = :sitename"
                     " WHERE"
                     " DCName = :dcName AND"
                     " Domain = :domainName;";

    if (!pCdcEntry)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VecsDbCreateContext(&pDbContext, VMAFD_DB_MODE_WRITE);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = sqlite3_prepare_v2(
                          pDbContext->pDb,
                          szQuery,
                          -1,
                          &pDbQuery,
                          NULL
                          );
    BAIL_ON_VMAFD_ERROR(dwError);


    dwError = VecsBindWideString(
                            pDbQuery,
                            ":dcName",
                            pCdcEntry->pszDCName
                            );
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VecsBindWideString(
                            pDbQuery,
                            ":domainName",
                            pCdcEntry->pszDomainName
                            );
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VecsBindWideString(
                            pDbQuery,
                            ":sitename",
                            pCdcEntry->pszSiteName
                            );
    BAIL_ON_VMAFD_ERROR(dwError);


    dwError = VecsBindDword(
                        pDbQuery,
                        ":lastPing",
                        pCdcEntry->dwLastPing
                        );
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VecsBindDword(
                         pDbQuery,
                         ":pingResponse",
                         pCdcEntry->dwPingTime
                         );
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VecsBindDword(
                         pDbQuery,
                         ":pingError",
                         pCdcEntry->dwLastError
                         );
    BAIL_ON_VMAFD_ERROR(dwError);


    dwError = VecsBindDword(
                        pDbQuery,
                        ":isAlive",
                        pCdcEntry->bIsAlive
                        );
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VecsDbStepSql(pDbQuery);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    if (pDbQuery)
    {
        sqlite3_reset(pDbQuery);
        sqlite3_finalize(pDbQuery);
    }
    if (pDbContext)
    {
        VecsDbReleaseContext(pDbContext);
    }
    return dwError;
error:

    goto cleanup;
}

DWORD
CdcDbIsDCAlive(
    PCDC_DC_INFO_W pDcInfo,
    BOOL* pbIsAlive
    )
{
    DWORD dwError = 0;
    PVECS_DB_CONTEXT pDbContext = NULL;
    sqlite3_stmt* pDbQuery = NULL;
    BOOL bIsAlive = FALSE;
    DWORD dwIsAlive = 0;
    DWORD dwCount = 0;

    char szQuery[] = " SELECT IsAlive FROM DCTable"
                     " WHERE DCName = :dcName AND"
                     " Domain = :domainName;";

    if (!pbIsAlive ||
        !pDcInfo
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VecsDbCreateContext(&pDbContext, VMAFD_DB_MODE_READ);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = sqlite3_prepare_v2(
                          pDbContext->pDb,
                          szQuery,
                          -1,
                          &pDbQuery,
                          NULL
                          );
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VecsBindWideString(
                            pDbQuery,
                            ":dcName",
                            pDcInfo->pszDCName
                            );
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VecsBindWideString(
                            pDbQuery,
                            ":domainName",
                            pDcInfo->pszDomainName
                            );
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VecsDbStepSql(pDbQuery);
    if (dwError == SQLITE_ROW)
    {
        dwError = VecsDBGetColumnInt(
                                pDbQuery,
                                "IsAlive",
                                &dwIsAlive
                                );
        BAIL_ON_VMAFD_ERROR(dwError);
        ++dwCount;
    }
    else if (dwError == SQLITE_DONE || !dwCount)
    {
        dwError = ERROR_OBJECT_NOT_FOUND;
    }
    BAIL_ON_VMAFD_ERROR_NO_LOG(dwError);

    bIsAlive = dwIsAlive?TRUE:FALSE;

    *pbIsAlive = bIsAlive;

cleanup:

    if (pDbQuery)
    {
        sqlite3_finalize(pDbQuery);
    }
    if (pDbContext)
    {
        VecsDbReleaseContext(pDbContext);
    }
    return dwError;
error:

    if (pbIsAlive)
    {
        *pbIsAlive = FALSE;
    }
    goto cleanup;
}

DWORD
CdcDbDeleteDCEntry(
    PCWSTR pszDCName
    )
{
    DWORD dwError = 0;
    PVECS_DB_CONTEXT pDbContext = NULL;
    sqlite3_stmt* pDbQuery = NULL;

    char szQuery[] = " DELETE FROM DCTable"
                     " WHERE DCName = :dcName";

    if (IsNullOrEmptyString(pszDCName))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VecsDbCreateContext(&pDbContext, VMAFD_DB_MODE_WRITE);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = sqlite3_prepare_v2(
                          pDbContext->pDb,
                          szQuery,
                          -1,
                          &pDbQuery,
                          NULL
                          );
    BAIL_ON_VMAFD_ERROR(dwError);


    dwError = VecsBindWideString(
                            pDbQuery,
                            ":dcName",
                            pszDCName
                            );
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VecsDbStepSql(pDbQuery);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    if (pDbQuery)
    {
        sqlite3_reset(pDbQuery);
        sqlite3_finalize(pDbQuery);
    }
    if (pDbContext)
    {
        VecsDbReleaseContext(pDbContext);
    }
    return dwError;
error:

    goto cleanup;
}

DWORD
CdcDbEnumDCEntries(
    PCDC_DB_ENTRY_W *ppEntries,
    PDWORD pdwCount
    )
{
    DWORD dwError = 0;
    PVECS_DB_CONTEXT pDbContext = NULL;
    sqlite3_stmt* pDbQuery = NULL;
    PCDC_DB_ENTRY_W pEntries = NULL;
    DWORD dwCount = 0;
    DWORD dwDCCount = 0;


    if (!ppEntries || !pdwCount)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VecsDbCreateContext(&pDbContext, VMAFD_DB_MODE_READ);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = CdcDbGetDCEntryCount(pDbContext, &dwCount);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (dwCount)
    {
        char szQuery[] = " SELECT DCName, Site, Domain, LastPing,"
                         " PingResponse, PingError, IsAlive"
                         " FROM DCTable LIMIT :count;";
        DWORD dwDbStatus = 0;

        dwError = sqlite3_prepare_v2(
                              pDbContext->pDb,
                              szQuery,
                              -1,
                              &pDbQuery,
                              NULL
                              );
        BAIL_ON_VMAFD_ERROR(dwError);

        dwError = VecsBindDword(
                            pDbQuery,
                            ":count",
                            dwCount
                            );
        BAIL_ON_VMAFD_ERROR(dwError);

        dwError = VmAfdAllocateMemory(
                              sizeof(CDC_DB_ENTRY_W)*dwCount,
                              (PVOID*)&pEntries
                              );
        BAIL_ON_VMAFD_ERROR(dwError);

        do
        {
            dwDbStatus = VecsDbStepSql(pDbQuery);

            if (dwDbStatus == SQLITE_ROW)
            {
                dwError = VecsDBGetColumnString(
                                        pDbQuery,
                                        "DCName",
                                        &pEntries[dwDCCount].pszDCName
                                        );
                BAIL_ON_VMAFD_ERROR(dwError);

                dwError = VecsDBGetColumnString(
                                        pDbQuery,
                                        "Site",
                                        &pEntries[dwDCCount].pszSiteName
                                        );
                BAIL_ON_VMAFD_ERROR(dwError);

                dwError = VecsDBGetColumnString(
                                       pDbQuery,
                                       "Domain",
                                       &pEntries[dwDCCount].pszDomainName
                                       );

                dwError = VecsDBGetColumnInt(
                                       pDbQuery,
                                       "LastPing",
                                       &pEntries[dwDCCount].dwLastPing
                                       );
                BAIL_ON_VMAFD_ERROR(dwError);

                dwError = VecsDBGetColumnInt(
                                       pDbQuery,
                                       "PingResponse",
                                       &pEntries[dwDCCount].dwPingTime
                                       );
                BAIL_ON_VMAFD_ERROR(dwError);

                dwError = VecsDBGetColumnInt(
                                       pDbQuery,
                                       "PingError",
                                       &pEntries[dwDCCount].dwLastError
                                       );
                BAIL_ON_VMAFD_ERROR(dwError);


                dwError = VecsDBGetColumnInt(
                                       pDbQuery,
                                       "IsAlive",
                                       &pEntries[dwDCCount].bIsAlive
                                       );
                BAIL_ON_VMAFD_ERROR(dwError);

                pEntries[dwDCCount].cdcEntryStatus = CDC_DB_ENTRY_STATUS_UPDATE;

                dwDCCount++;
            }
            else if (dwDbStatus != SQLITE_DONE)
            {
                dwError = dwDbStatus;
                BAIL_ON_VMAFD_ERROR(dwError);
            }
        } while (dwDbStatus == SQLITE_ROW && dwDCCount < dwCount);
    }

    *pdwCount = dwDCCount;
    *ppEntries = pEntries;

cleanup:

    if (pDbQuery)
    {
        sqlite3_reset(pDbQuery);
        sqlite3_finalize(pDbQuery);
    }
    if (pDbContext)
    {
        VecsDbReleaseContext(pDbContext);
    }
    return dwError;
error:

    if (ppEntries)
    {
        *ppEntries = NULL;
    }
    if (pdwCount)
    {
        *pdwCount = 0;
    }
    if (pEntries)
    {
        VmAfdFreeCdcDbEntriesW(pEntries, dwDCCount);
    }
    goto cleanup;
}

DWORD
CdcDbEnumDCEntriesFiltered(
    CDC_DB_ENUM_FILTER cdcDbEnumFilter,
    PWSTR pwszFilterString,
    PCDC_DB_ENTRY_ARRAY *ppCdcDbEntryArray
    )
{
    DWORD dwError = 0;
    PCDC_DB_ENTRY_ARRAY pCdcDbEntryArray = NULL;

    if (!ppCdcDbEntryArray || IsNullOrEmptyString(pwszFilterString))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    switch(cdcDbEnumFilter)
    {
        case CDC_DB_ENUM_FILTER_ON_SITE_AND_ACTIVE:
          dwError = CdcDbEnumDCEntriesSiteAndActive(
                                    pwszFilterString,
                                    &pCdcDbEntryArray
                                    );
          break;

        case CDC_DB_ENUM_FILTER_ON_SITE:
          dwError = CdcDbEnumDCEntriesSiteOnly(
                                    pwszFilterString,
                                    &pCdcDbEntryArray
                                    );
          break;

        case CDC_DB_ENUM_FILTER_OFF_SITE:
          dwError = CdcDbEnumDCEntriesOffsite(
                                    pwszFilterString,
                                    &pCdcDbEntryArray
                                    );
          break;

        default:
          dwError = ERROR_INVALID_PARAMETER;
          break;
    }
    BAIL_ON_VMAFD_ERROR(dwError);


    *ppCdcDbEntryArray = pCdcDbEntryArray;
cleanup:

    return dwError;
error:

    if (pCdcDbEntryArray)
    {
        VmAfdFreeCdcDbEntryArray(pCdcDbEntryArray);
    }
    goto cleanup;
}

DWORD
CdcDbGetClosestDCOnSite(
     PCWSTR pwszClientSiteName,
     PCWSTR pwszDomainName,
     PWSTR  *ppszDCName
     )
{
    DWORD dwError = 0;
    PVECS_DB_CONTEXT pDbContext = NULL;
    sqlite3_stmt* pDbQuery = NULL;
    PWSTR pwszDCName = NULL;
    DWORD dwCount = 0;

    char szQuery[] = "SELECT DCName FROM DCTable"
                     " WHERE IsAlive = 1 AND"
                     " Site = :site AND"
                     " Domain = :domainName"
                     " ORDER BY PingResponse,RANDOM()"
                     " LIMIT 1;";

    if (!ppszDCName ||
        IsNullOrEmptyString(pwszClientSiteName) ||
        IsNullOrEmptyString(pwszDomainName)
        )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VecsDbCreateContext(&pDbContext, VMAFD_DB_MODE_READ);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = sqlite3_prepare_v2(
                              pDbContext->pDb,
                              szQuery,
                              -1,
                              &pDbQuery,
                              NULL
                              );
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VecsBindWideString(
                              pDbQuery,
                              ":site",
                              pwszClientSiteName
                              );
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VecsBindWideString(
                             pDbQuery,
                             ":domainName",
                             pwszDomainName
                             );
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VecsDbStepSql(pDbQuery);

    if (dwError == SQLITE_ROW)
    {
        dwError = VecsDBGetColumnString(
                                  pDbQuery,
                                  "DCName",
                                  &pwszDCName
                                  );
        BAIL_ON_VMAFD_ERROR(dwError);
        ++dwCount;
    }
    else if (dwError == SQLITE_DONE || !dwCount)
    {
        dwError = ERROR_OBJECT_NOT_FOUND;
    }
    BAIL_ON_VMAFD_ERROR_NO_LOG(dwError);

    *ppszDCName = pwszDCName;

cleanup:

    if (pDbQuery)
    {
        sqlite3_reset(pDbQuery);
        sqlite3_finalize(pDbQuery);
    }
    if (pDbContext)
    {
        VecsDbReleaseContext(pDbContext);
    }
    return dwError;
error:

    if (ppszDCName)
    {
        *ppszDCName = NULL;
    }
    VMAFD_SAFE_FREE_MEMORY(pwszDCName);
    goto cleanup;
}

DWORD
CdcDbGetClosestDC(
     PCWSTR pwszDomainName,
     PWSTR *ppszDCName
     )
{
    DWORD dwError = 0;
    PVECS_DB_CONTEXT pDbContext = NULL;
    sqlite3_stmt* pDbQuery = NULL;
    PWSTR pwszDCName = NULL;
    DWORD dwCount = 0;
    DWORD dwHeartbeat = 0;
    time_t tMinusLastState = 0;

    char szQuery[] = "SELECT DCName FROM DCTable"
                     " WHERE IsAlive = 1 AND"
                     " Domain = :domainName"
                     //" LastPing > :time"
                     " ORDER BY PingResponse, RANDOM()"
                     " LIMIT 1;";

    if (!ppszDCName ||
        IsNullOrEmptyString(pwszDomainName)
        )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VecsDbCreateContext(&pDbContext, VMAFD_DB_MODE_READ);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = sqlite3_prepare_v2(
                              pDbContext->pDb,
                              szQuery,
                              -1,
                              &pDbQuery,
                              NULL
                              );
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VecsBindWideString(
                             pDbQuery,
                             ":domainName",
                             pwszDomainName
                             );
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = CdcRegDbGetHeartBeatInterval(&dwHeartbeat);
    if (dwError)
    {
        dwHeartbeat = CDC_DEFAULT_HEARTBEAT;
        dwError = 0;
    }

    tMinusLastState = time(NULL) - dwHeartbeat;

    /*
     * TODO: Commenting this out till we fix CdcUpdateAndPing to wake up statemachine
     *dwError = VecsBindDword(
     *                    pDbQuery,
     *                    ":time",
     *                    tMinusLastState
     *                    );
     *BAIL_ON_VMAFD_ERROR(dwError);
     */

    dwError = VecsDbStepSql(pDbQuery);

    if (dwError == SQLITE_ROW)
    {
        dwError = VecsDBGetColumnString(
                                  pDbQuery,
                                  "DCName",
                                  &pwszDCName
                                  );
        BAIL_ON_VMAFD_ERROR(dwError);
        ++dwCount;
    }
    else if (dwError == SQLITE_DONE || !dwCount)
    {
        dwError = ERROR_OBJECT_NOT_FOUND;
    }
    BAIL_ON_VMAFD_ERROR_NO_LOG(dwError);

    *ppszDCName = pwszDCName;

cleanup:

    if (pDbQuery)
    {
        sqlite3_reset(pDbQuery);
        sqlite3_finalize(pDbQuery);
    }
    if (pDbContext)
    {
        VecsDbReleaseContext(pDbContext);
    }
    return dwError;
error:

    if (ppszDCName)
    {
        *ppszDCName = NULL;
    }
    VMAFD_SAFE_FREE_MEMORY(pwszDCName);
    goto cleanup;
}

DWORD
CdcDbUpdateHeartbeatStatus(
    PCDC_DB_ENTRY_W pCdcDbEntry,
    PVMAFD_HB_STATUS_W pHeartbeatStatus
    )
{
    DWORD dwError = 0;
    DWORD dwIndex = 0;
    PVECS_DB_CONTEXT pDbContext = NULL;

    if (!pCdcDbEntry ||!pHeartbeatStatus)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VecsDbCreateContext(&pDbContext, VMAFD_DB_MODE_WRITE);
    BAIL_ON_VMAFD_ERROR(dwError);

    for (; dwIndex < pHeartbeatStatus->dwCount; ++dwIndex)
    {
        PVMAFD_HB_INFO_W pHeartbeatInfoArrCur =
                                 &pHeartbeatStatus->pHeartbeatInfoArr[dwIndex];

        dwError = CdcDbUpdateServiceStatus(
                                  pDbContext,
                                  pCdcDbEntry,
                                  pHeartbeatInfoArrCur
                                  );
        BAIL_ON_VMAFD_ERROR(dwError);
    }

cleanup:

    if (pDbContext)
    {
        VecsDbReleaseContext(pDbContext);
    }
    return dwError;
error:

    goto cleanup;
}

DWORD
CdcDbDeleteHeartbeatStatus(
    PCDC_DB_ENTRY_W pCdcDbEntry
    )
{
    DWORD dwError = 0;
    PVECS_DB_CONTEXT pDbContext = NULL;

    sqlite3_stmt* pDbQuery = NULL;

    char szQuery[] = "DELETE FROM DCServiceStatus"
                     " WHERE DCID = ("
                     " SELECT DCID from DCTable"
                     " WHERE DCName = :dcName"
                     " AND Domain = :domainName);";

    if (!pCdcDbEntry)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }


    dwError = VecsDbCreateContext(&pDbContext, VMAFD_DB_MODE_WRITE);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = sqlite3_prepare_v2(
                              pDbContext->pDb,
                              szQuery,
                              -1,
                              &pDbQuery,
                              NULL
                              );
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VecsBindWideString(
                             pDbQuery,
                             ":dcName",
                             pCdcDbEntry->pszDCName
                             );
    BAIL_ON_VMAFD_ERROR(dwError);


    dwError = VecsBindWideString(
                             pDbQuery,
                             ":domainName",
                             pCdcDbEntry->pszDomainName
                             );
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VecsDbStepSql(pDbQuery);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    if (pDbQuery)
    {
        sqlite3_reset(pDbQuery);
        sqlite3_finalize(pDbQuery);
    }
    if (pDbContext)
    {
        VecsDbReleaseContext(pDbContext);
    }

    return dwError;
error:

    goto cleanup;
}

DWORD
CdcDbGetHeartbeatStatus(
    PWSTR pwszDCName,
    PWSTR pwszDomainName,
    PVMAFD_HB_STATUS_W *ppHeartbeatStatus
    )
{
    DWORD dwError = 0;

    PVECS_DB_CONTEXT pDbContext = NULL;
    PVMAFD_HB_STATUS_W pHeartbeatStatus = NULL;
    PVMAFD_HB_INFO_W   pHeartbeatInfoArr = NULL;
    sqlite3_stmt* pDbQuery = NULL;
    DWORD dwCount  = 0;
    DWORD dwExpectedCount = 0;
    DWORD dwDbStatus = 0;

    char szQuery[] = "SELECT * FROM DCServiceStatus"
                     " WHERE DCID = ("
                     " SELECT DCID from DCTable"
                     " WHERE DCName = :dcName"
                     " AND Domain = :domainName);";

    if (IsNullOrEmptyString(pwszDCName) ||
        IsNullOrEmptyString(pwszDomainName) ||
        !ppHeartbeatStatus
        )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VecsDbCreateContext(&pDbContext, VMAFD_DB_MODE_READ);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateMemory(
                            sizeof(VMAFD_HB_STATUS_W),
                            (PVOID)&pHeartbeatStatus
                            );
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = CdcDbGetHeartbeatStatusCount(
                                      pDbContext,
                                      pwszDCName,
                                      pwszDomainName,
                                      &dwExpectedCount
                                      );
    BAIL_ON_VMAFD_ERROR(dwError);

    if (dwExpectedCount)
    {
        dwError = VmAfdAllocateMemory(
                              sizeof(VMAFD_HB_INFO_W)*dwExpectedCount,
                              (PVOID)&pHeartbeatInfoArr
                              );
        BAIL_ON_VMAFD_ERROR(dwError);

        dwError = sqlite3_prepare_v2(
                                  pDbContext->pDb,
                                  szQuery,
                                  -1,
                                  &pDbQuery,
                                  NULL
                                  );
        BAIL_ON_VMAFD_ERROR(dwError);

        dwError = VecsBindWideString(
                                 pDbQuery,
                                 ":dcName",
                                 pwszDCName
                                 );
        BAIL_ON_VMAFD_ERROR(dwError);

        dwError = VecsBindWideString(
                                 pDbQuery,
                                 ":domainName",
                                 pwszDomainName
                                 );
        BAIL_ON_VMAFD_ERROR(dwError);

        do
        {
            dwDbStatus = VecsDbStepSql(pDbQuery);

            if (dwDbStatus == SQLITE_ROW)
            {
                PVMAFD_HB_INFO_W pHeartbeatInfoArrCur =
                                 &pHeartbeatInfoArr[dwCount];

                dwError = VecsDBGetColumnString(
                                          pDbQuery,
                                          "ServiceName",
                                          &pHeartbeatInfoArrCur->pszServiceName
                                          );
                BAIL_ON_VMAFD_ERROR(dwError);

                dwError = VecsDBGetColumnInt(
                                          pDbQuery,
                                          "Port",
                                          &pHeartbeatInfoArrCur->dwPort
                                          );
                BAIL_ON_VMAFD_ERROR(dwError);

                dwError = VecsDBGetColumnInt(
                                          pDbQuery,
                                          "IsAlive",
                                          &pHeartbeatInfoArrCur->bIsAlive
                                          );
                BAIL_ON_VMAFD_ERROR(dwError);

                dwError = VecsDBGetColumnInt(
                                          pDbQuery,
                                          "LastHeartbeat",
                                          &pHeartbeatInfoArrCur->dwLastHeartbeat
                                          );
                BAIL_ON_VMAFD_ERROR(dwError);
                ++dwCount;
            }
            else if (dwDbStatus != SQLITE_DONE)
            {
                dwError = dwDbStatus;
                BAIL_ON_VMAFD_ERROR(dwError);
            }
        } while (dwDbStatus == SQLITE_ROW && dwCount < dwExpectedCount);
    }

    pHeartbeatStatus->pHeartbeatInfoArr = pHeartbeatInfoArr;
    pHeartbeatInfoArr = NULL;
    pHeartbeatStatus->dwCount = dwCount;

    *ppHeartbeatStatus = pHeartbeatStatus;

cleanup:

    if (pDbQuery)
    {
        sqlite3_reset(pDbQuery);
        sqlite3_finalize(pDbQuery);
    }
    if (pDbContext)
    {
        VecsDbReleaseContext(pDbContext);
    }
    return dwError;
error:

    if (ppHeartbeatStatus)
    {
        *ppHeartbeatStatus = NULL;
    }
    if (pHeartbeatStatus)
    {
        VmAfdFreeHbStatusW(pHeartbeatStatus);
    }
    if (pHeartbeatInfoArr)
    {
        VmAfdFreeHbInfoArrayW(pHeartbeatInfoArr, dwCount);
    }
    goto cleanup;
}

DWORD
CdcDbGetDCInfo(
    PWSTR pwszDCName,
    PWSTR pwszDomainName,
    PCDC_DC_STATUS_INFO_W *ppCdcStatusInfo
    )
{
    DWORD dwError = 0;

    PVECS_DB_CONTEXT pDbContext = NULL;
    PCDC_DC_STATUS_INFO_W pCdcStatusInfo = NULL;
    sqlite3_stmt* pDbQuery = NULL;
    DWORD dwCount  = 0;

    char szQuery[] = "SELECT Site, LastPing, PingResponse,"
                     " PingError, IsAlive FROM DCTable"
                     " WHERE DCID = ("
                     " SELECT DCID from DCTable"
                     " WHERE DCName = :dcName"
                     " AND Domain = :domainName);";

    if (IsNullOrEmptyString(pwszDCName) ||
        IsNullOrEmptyString(pwszDomainName) ||
        !ppCdcStatusInfo
        )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VecsDbCreateContext(&pDbContext, VMAFD_DB_MODE_READ);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateMemory(
                            sizeof(CDC_DC_STATUS_INFO_W),
                            (PVOID)&pCdcStatusInfo
                            );
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = sqlite3_prepare_v2(
                              pDbContext->pDb,
                              szQuery,
                              -1,
                              &pDbQuery,
                              NULL
                              );
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VecsBindWideString(
                             pDbQuery,
                             ":dcName",
                             pwszDCName
                             );
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VecsBindWideString(
                             pDbQuery,
                             ":domainName",
                             pwszDomainName
                             );
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VecsDbStepSql(pDbQuery);

    if (dwError == SQLITE_ROW)
    {
        dwError = VecsDBGetColumnString(
                                  pDbQuery,
                                  "Site",
                                  &pCdcStatusInfo->pwszSiteName
                                  );
        BAIL_ON_VMAFD_ERROR(dwError);

        dwError = VecsDBGetColumnInt(
                                  pDbQuery,
                                  "LastPing",
                                  &pCdcStatusInfo->dwLastPing
                                  );
        BAIL_ON_VMAFD_ERROR(dwError);

        dwError = VecsDBGetColumnInt(
                                  pDbQuery,
                                  "PingResponse",
                                  &pCdcStatusInfo->dwLastResponseTime
                                  );
        BAIL_ON_VMAFD_ERROR(dwError);

        dwError = VecsDBGetColumnInt(
                                  pDbQuery,
                                  "PingError",
                                  &pCdcStatusInfo->dwLastError
                                  );
        BAIL_ON_VMAFD_ERROR(dwError);

        dwError = VecsDBGetColumnInt(
                                 pDbQuery,
                                 "IsAlive",
                                 &pCdcStatusInfo->bIsAlive
                                 );
        BAIL_ON_VMAFD_ERROR(dwError);
        ++dwCount;
    }
    else if (dwError == SQLITE_DONE)
    {
        dwError = ERROR_OBJECT_NOT_FOUND;
        BAIL_ON_VMAFD_ERROR_NO_LOG(dwError);
    }

    *ppCdcStatusInfo = pCdcStatusInfo;

cleanup:

    if (pDbQuery)
    {
        sqlite3_reset(pDbQuery);
        sqlite3_finalize(pDbQuery);
    }
    if (pDbContext)
    {
        VecsDbReleaseContext(pDbContext);
    }
    return dwError;
error:

    if (ppCdcStatusInfo)
    {
        *ppCdcStatusInfo = NULL;
    }
    if (pCdcStatusInfo)
    {
        VmAfdFreeCdcStatusInfoW(pCdcStatusInfo);
    }
    goto cleanup;
}

VOID
VmAfdFreeCdcDbEntriesW(
    PCDC_DB_ENTRY_W pCdcDbEntry,
    DWORD dwCount
    )
{
    DWORD dwIndex = 0;

    if (pCdcDbEntry)
    {
        for (;dwIndex<dwCount;++dwIndex)
        {
            VMAFD_SAFE_FREE_MEMORY(pCdcDbEntry[dwIndex].pszDCName);
            VMAFD_SAFE_FREE_MEMORY(pCdcDbEntry[dwIndex].pszSiteName);
            VMAFD_SAFE_FREE_MEMORY(pCdcDbEntry[dwIndex].pszDomainName);
        }
        VMAFD_SAFE_FREE_MEMORY(pCdcDbEntry);
    }
}

VOID
VmAfdFreeCdcDbEntry(
    PCDC_DB_ENTRY_W pCdcDbEntry
    )
{
    if (pCdcDbEntry)
    {
        VMAFD_SAFE_FREE_MEMORY(pCdcDbEntry->pszDCName);
        VMAFD_SAFE_FREE_MEMORY(pCdcDbEntry->pszSiteName);
        VMAFD_SAFE_FREE_MEMORY(pCdcDbEntry->pszDomainName);
    }
    VMAFD_SAFE_FREE_MEMORY(pCdcDbEntry);
}

VOID
VmAfdFreeCdcDbEntryArray(
    PCDC_DB_ENTRY_ARRAY pCdcDbEntryArray
    )
{
    if (pCdcDbEntryArray)
    {

        VmAfdFreeCdcDbArrayEntries(
                      pCdcDbEntryArray->dwCount,
                      pCdcDbEntryArray->pCdcDbEntries
                      );
        VMAFD_SAFE_FREE_MEMORY(pCdcDbEntryArray);
    }
}

static
DWORD
CdcDbGetDCEntryCount(
    PVECS_DB_CONTEXT pDbContext,
    PDWORD pdwDCCount
    )
{
    DWORD dwError = 0;
    sqlite3_stmt* pDbQuery = NULL;
    DWORD dwDCCount = 0;

    char szQuery[] = "SELECT COUNT(*)"
                     " FROM DCTable";

    if (!pDbContext || !pdwDCCount)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    dwError = sqlite3_prepare_v2(
                pDbContext->pDb,
                szQuery,
                -1,
                &pDbQuery,
                NULL);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VecsDbStepSql(pDbQuery);
    if (dwError == SQLITE_ROW)
    {
        dwError = 0;
        dwDCCount = sqlite3_column_int(pDbQuery, 0);
    }
    BAIL_ON_VMAFD_ERROR (dwError);

    *pdwDCCount = dwDCCount;
cleanup:
    if (pDbQuery)
    {
      sqlite3_finalize (pDbQuery);
    }

    return dwError;
error:
    if (pdwDCCount)
    {
        *pdwDCCount = 0;
    }

    goto cleanup;
}

static
DWORD
CdcDbRemoveExistingDC(
    PVECS_DB_CONTEXT pDbContext,
    PCWSTR pszDomainName
    )
{
    DWORD dwError = 0;
    sqlite3_stmt* pDbQuery = NULL;
    char szQuery[] = "DELETE FROM AffinitizedDC WHERE"
                     " Domain = :domainName;";

    if (!pDbContext ||
        IsNullOrEmptyString(pszDomainName)
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if (pDbContext->dbOpenMode != VMAFD_DB_MODE_WRITE)
    {
        dwError = ERROR_INVALID_ACCESS;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = sqlite3_prepare_v2(
                pDbContext->pDb,
                szQuery,
                -1,
                &pDbQuery,
                NULL);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VecsBindWideString(
                            pDbQuery,
                            ":domainName",
                            pszDomainName
                            );
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VecsDbStepSql(pDbQuery);
    BAIL_ON_VMAFD_ERROR (dwError);

cleanup:

    if (pDbQuery)
    {
        sqlite3_finalize(pDbQuery);
    }

    return dwError;
error:

    goto cleanup;
}

static
DWORD
CdcDbGetDCEntryCountSiteOnly(
    PVECS_DB_CONTEXT pDbContext,
    PWSTR pwszSiteName,
    PDWORD pdwDCCount
    )
{
    DWORD dwError = 0;
    sqlite3_stmt* pDbQuery = NULL;
    DWORD dwDCCount = 0;

    char szQuery[] = "SELECT COUNT(*)"
                     " FROM DCTable"
                     " WHERE Site = :site;";

    if (!pDbContext || !pdwDCCount)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    dwError = sqlite3_prepare_v2(
                pDbContext->pDb,
                szQuery,
                -1,
                &pDbQuery,
                NULL);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VecsBindWideString(
                            pDbQuery,
                            ":site",
                            pwszSiteName
                            );
    BAIL_ON_VMAFD_ERROR(dwError);


    dwError = VecsDbStepSql(pDbQuery);
    if (dwError == SQLITE_ROW)
    {
        dwError = 0;
        dwDCCount = sqlite3_column_int(pDbQuery, 0);
    }
    BAIL_ON_VMAFD_ERROR (dwError);

    *pdwDCCount = dwDCCount;
cleanup:
    if (pDbQuery)
    {
      sqlite3_finalize (pDbQuery);
    }

    return dwError;
error:
    if (pdwDCCount)
    {
        *pdwDCCount = 0;
    }

    goto cleanup;
}

static
DWORD
CdcDbGetDCEntryCountSiteAndActive(
    PVECS_DB_CONTEXT pDbContext,
    PWSTR pwszSiteName,
    PDWORD pdwDCCount
    )
{
    DWORD dwError = 0;
    sqlite3_stmt* pDbQuery = NULL;
    DWORD dwDCCount = 0;

    char szQuery[] = " SELECT COUNT(*) FROM DCTable"
                     " WHERE Site = :site"
                     " OR DCID IN ("
                     " SELECT DCID FROM AffinitizedDC);";

    if (!pDbContext || !pdwDCCount)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    dwError = sqlite3_prepare_v2(
                pDbContext->pDb,
                szQuery,
                -1,
                &pDbQuery,
                NULL);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VecsBindWideString(
                            pDbQuery,
                            ":site",
                            pwszSiteName
                            );
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VecsDbStepSql(pDbQuery);
    if (dwError == SQLITE_ROW)
    {
        dwError = 0;
        dwDCCount = sqlite3_column_int(pDbQuery, 0);
    }
    BAIL_ON_VMAFD_ERROR (dwError);

    *pdwDCCount = dwDCCount;
cleanup:
    if (pDbQuery)
    {
      sqlite3_finalize (pDbQuery);
    }

    return dwError;
error:
    if (pdwDCCount)
    {
        *pdwDCCount = 0;
    }

    goto cleanup;
}

static
DWORD
CdcDbGetDCEntryCountOffSite(
    PVECS_DB_CONTEXT pDbContext,
    PWSTR pwszSiteName,
    PDWORD pdwDCCount
    )
{
    DWORD dwError = 0;
    sqlite3_stmt* pDbQuery = NULL;
    DWORD dwDCCount = 0;

    char szQuery[] = "SELECT COUNT(*)"
                     " FROM DCTable"
                     " WHERE Site != :site;";

    if (!pDbContext || !pdwDCCount)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    dwError = sqlite3_prepare_v2(
                pDbContext->pDb,
                szQuery,
                -1,
                &pDbQuery,
                NULL);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VecsBindWideString(
                            pDbQuery,
                            ":site",
                            pwszSiteName
                            );
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VecsDbStepSql(pDbQuery);
    if (dwError == SQLITE_ROW)
    {
        dwError = 0;
        dwDCCount = sqlite3_column_int(pDbQuery, 0);
    }
    BAIL_ON_VMAFD_ERROR (dwError);

    *pdwDCCount = dwDCCount;
cleanup:
    if (pDbQuery)
    {
      sqlite3_finalize (pDbQuery);
    }

    return dwError;
error:
    if (pdwDCCount)
    {
        *pdwDCCount = 0;
    }

    goto cleanup;
}

static
VOID
VmAfdFreeCdcDbArrayEntries(
    DWORD dwCount,
    PCDC_DB_ENTRY_W *ppEntries
    )
{
    DWORD dwIndex = 0;

    if (ppEntries)
    {
        for (;dwIndex<dwCount;++dwIndex)
        {
            VmAfdFreeCdcDbEntry(ppEntries[dwIndex]);
        }
    }

    VMAFD_SAFE_FREE_MEMORY(ppEntries);
}

static
DWORD
CdcDbEnumDCGetData(
    sqlite3_stmt* pDbQuery,
    DWORD dwExpectedCount,
    PCDC_DB_ENTRY_W **pppEntries,
    PDWORD pdwCount
    )
{
    DWORD dwError = 0;
    DWORD dwDbStatus = 0;
    DWORD dwDCCount = 0;
    PCDC_DB_ENTRY_W *ppEntries = NULL;

    if (!pppEntries)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if (dwExpectedCount)
    {
        dwError = VmAfdAllocateMemory(
                              sizeof(PCDC_DB_ENTRY_W)*dwExpectedCount,
                              (PVOID)&ppEntries
                              );
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    do
    {
        dwDbStatus = VecsDbStepSql(pDbQuery);

        if (dwDbStatus == SQLITE_ROW)
        {

            dwError = VmAfdAllocateMemory(
                                    sizeof(CDC_DB_ENTRY_W),
                                    (PVOID)&ppEntries[dwDCCount]
                                    );
            BAIL_ON_VMAFD_ERROR(dwError);

            dwError = VecsDBGetColumnString(
                                    pDbQuery,
                                    "DCName",
                                    &ppEntries[dwDCCount]->pszDCName
                                    );
            BAIL_ON_VMAFD_ERROR(dwError);

            dwError = VecsDBGetColumnString(
                                    pDbQuery,
                                    "Site",
                                    &ppEntries[dwDCCount]->pszSiteName
                                    );
            BAIL_ON_VMAFD_ERROR(dwError);

            dwError = VecsDBGetColumnString(
                                   pDbQuery,
                                   "Domain",
                                   &ppEntries[dwDCCount]->pszDomainName
                                   );

            dwError = VecsDBGetColumnInt(
                                   pDbQuery,
                                   "LastPing",
                                   &ppEntries[dwDCCount]->dwLastPing
                                   );
            BAIL_ON_VMAFD_ERROR(dwError);

            dwError = VecsDBGetColumnInt(
                                   pDbQuery,
                                   "PingResponse",
                                   &ppEntries[dwDCCount]->dwPingTime
                                   );
            BAIL_ON_VMAFD_ERROR(dwError);

            dwError = VecsDBGetColumnInt(
                                   pDbQuery,
                                   "PingError",
                                   &ppEntries[dwDCCount]->dwLastError
                                   );
            BAIL_ON_VMAFD_ERROR(dwError);


            dwError = VecsDBGetColumnInt(
                                   pDbQuery,
                                   "IsAlive",
                                   &ppEntries[dwDCCount]->bIsAlive
                                   );
            BAIL_ON_VMAFD_ERROR(dwError);

            dwDCCount++;
        }
        else if (dwDbStatus != SQLITE_DONE)
        {
            dwError = dwDbStatus;
            BAIL_ON_VMAFD_ERROR(dwError);
        }
    } while (dwDbStatus == SQLITE_ROW && dwDCCount < dwExpectedCount);

    *pppEntries = ppEntries;
    *pdwCount = dwDCCount;

cleanup:
    return dwError;

error:

    if (pppEntries)
    {
        *pppEntries = NULL;
    }
    if (pdwCount)
    {
        *pdwCount = 0;
    }
    if (ppEntries)
    {
        VmAfdFreeCdcDbArrayEntries(dwDCCount, ppEntries);
    }

    goto cleanup;
}

static
DWORD
CdcDbEnumDCEntriesSiteOnly(
    PWSTR pwszSite,
    PCDC_DB_ENTRY_ARRAY *ppCdcDbEntryArray
    )
{
    DWORD dwError = 0;
    PVECS_DB_CONTEXT pDbContext = NULL;
    sqlite3_stmt* pDbQuery = NULL;
    PCDC_DB_ENTRY_ARRAY pCdcDbEntryArray = NULL;
    PCDC_DB_ENTRY_W *ppEntries = NULL;
    DWORD dwCount = 0;
    DWORD dwDCCount = 0;


    if (!ppCdcDbEntryArray)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdAllocateMemory(
                              sizeof(CDC_DB_ENTRY_ARRAY),
                              (PVOID)&pCdcDbEntryArray
                              );
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VecsDbCreateContext(&pDbContext, VMAFD_DB_MODE_READ);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = CdcDbGetDCEntryCountSiteOnly(pDbContext,pwszSite, &dwCount);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (dwCount)
    {
        char szQuery[] = " SELECT * FROM DCTable"
                         " WHERE Site = :site"
                         " LIMIT :count;";


        dwError = sqlite3_prepare_v2(
                              pDbContext->pDb,
                              szQuery,
                              -1,
                              &pDbQuery,
                              NULL
                              );
        BAIL_ON_VMAFD_ERROR(dwError);

        dwError = VecsBindWideString(
                            pDbQuery,
                            ":site",
                            pwszSite
                            );
        BAIL_ON_VMAFD_ERROR(dwError);

        dwError = VecsBindDword(
                            pDbQuery,
                            ":count",
                            dwCount
                            );
        BAIL_ON_VMAFD_ERROR(dwError);

        dwError = CdcDbEnumDCGetData(pDbQuery, dwCount, &ppEntries, &dwDCCount);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    pCdcDbEntryArray->dwCount = dwDCCount;
    pCdcDbEntryArray->pCdcDbEntries = ppEntries;
    ppEntries = NULL;

    *ppCdcDbEntryArray = pCdcDbEntryArray;


cleanup:

    if (pDbQuery)
    {
        sqlite3_reset(pDbQuery);
        sqlite3_finalize(pDbQuery);
    }
    if (pDbContext)
    {
        VecsDbReleaseContext(pDbContext);
    }
    if (ppEntries)
    {
        VmAfdFreeCdcDbArrayEntries(dwCount, ppEntries);
    }
    return dwError;
error:

    if (ppCdcDbEntryArray)
    {
        *ppCdcDbEntryArray = NULL;
    }
    if (pCdcDbEntryArray)
    {
        VmAfdFreeCdcDbEntryArray(pCdcDbEntryArray);
    }
    goto cleanup;
}

static
DWORD
CdcDbEnumDCEntriesSiteAndActive(
    PWSTR pwszSite,
    PCDC_DB_ENTRY_ARRAY *ppCdcDbEntryArray
    )
{
    DWORD dwError = 0;
    PVECS_DB_CONTEXT pDbContext = NULL;
    sqlite3_stmt* pDbQuery = NULL;
    PCDC_DB_ENTRY_ARRAY pCdcDbEntryArray = NULL;
    PCDC_DB_ENTRY_W *ppEntries = NULL;
    DWORD dwCount = 0;
    DWORD dwDCCount = 0;

    if (!ppCdcDbEntryArray)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdAllocateMemory(
                          sizeof(CDC_DB_ENTRY_ARRAY),
                          (PVOID)&pCdcDbEntryArray
                          );
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VecsDbCreateContext(&pDbContext, VMAFD_DB_MODE_READ);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = CdcDbGetDCEntryCountSiteAndActive(pDbContext, pwszSite,&dwCount);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (dwCount)
    {
        char szQuery[] = " SELECT * FROM DCTable"
                         " WHERE Site = :site"
                         " OR DCID IN ("
                         " SELECT DCID FROM AffinitizedDC)"
                         " LIMIT :count;";

        dwError = sqlite3_prepare_v2(
                              pDbContext->pDb,
                              szQuery,
                              -1,
                              &pDbQuery,
                              NULL
                              );
        BAIL_ON_VMAFD_ERROR(dwError);

        dwError = VecsBindWideString(
                            pDbQuery,
                            ":site",
                            pwszSite
                            );
        BAIL_ON_VMAFD_ERROR(dwError);

        dwError = VecsBindDword(
                            pDbQuery,
                            ":count",
                            dwCount
                            );
        BAIL_ON_VMAFD_ERROR(dwError);

        dwError = CdcDbEnumDCGetData(pDbQuery, dwCount, &ppEntries, &dwDCCount);
        BAIL_ON_VMAFD_ERROR(dwError);

    }

    pCdcDbEntryArray->dwCount = dwDCCount;
    pCdcDbEntryArray->pCdcDbEntries = ppEntries;
    ppEntries = NULL;

    *ppCdcDbEntryArray = pCdcDbEntryArray;

cleanup:

    if (pDbQuery)
    {
        sqlite3_reset(pDbQuery);
        sqlite3_finalize(pDbQuery);
    }
    if (pDbContext)
    {
        VecsDbReleaseContext(pDbContext);
    }
    if (ppEntries)
    {
        VmAfdFreeCdcDbArrayEntries(dwCount, ppEntries);
    }
    return dwError;
error:

    if (ppCdcDbEntryArray)
    {
        *ppCdcDbEntryArray = NULL;
    }
    if (pCdcDbEntryArray)
    {
        VmAfdFreeCdcDbEntryArray(pCdcDbEntryArray);
    }
    goto cleanup;
}

static
DWORD
CdcDbEnumDCEntriesOffsite(
    PWSTR pwszSite,
    PCDC_DB_ENTRY_ARRAY *ppCdcDbEntryArray
    )
{
    DWORD dwError = 0;
    PVECS_DB_CONTEXT pDbContext = NULL;
    sqlite3_stmt* pDbQuery = NULL;
    PCDC_DB_ENTRY_ARRAY pCdcDbEntryArray = NULL;
    PCDC_DB_ENTRY_W *ppEntries = NULL;
    DWORD dwCount = 0;
    DWORD dwDCCount = 0;

    if (!ppCdcDbEntryArray)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdAllocateMemory(
                              sizeof(CDC_DB_ENTRY_ARRAY),
                              (PVOID)&pCdcDbEntryArray
                              );
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VecsDbCreateContext(&pDbContext, VMAFD_DB_MODE_READ);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = CdcDbGetDCEntryCountOffSite(pDbContext,pwszSite, &dwCount);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (dwCount)
    {
        char szQuery[] = " SELECT * FROM DCTable"
                         " WHERE Site != :site"
                         " LIMIT :count;";

        dwError = sqlite3_prepare_v2(
                              pDbContext->pDb,
                              szQuery,
                              -1,
                              &pDbQuery,
                              NULL
                              );
        BAIL_ON_VMAFD_ERROR(dwError);

        dwError = VecsBindWideString(
                            pDbQuery,
                            ":site",
                            pwszSite
                            );
        BAIL_ON_VMAFD_ERROR(dwError);

        dwError = VecsBindDword(
                            pDbQuery,
                            ":count",
                            dwCount
                            );
        BAIL_ON_VMAFD_ERROR(dwError);

        dwError = CdcDbEnumDCGetData(pDbQuery, dwCount, &ppEntries, &dwDCCount);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    pCdcDbEntryArray->dwCount = dwDCCount;
    pCdcDbEntryArray->pCdcDbEntries = ppEntries;
    ppEntries = NULL;

    *ppCdcDbEntryArray = pCdcDbEntryArray;

cleanup:

    if (pDbQuery)
    {
        sqlite3_reset(pDbQuery);
        sqlite3_finalize(pDbQuery);
    }
    if (pDbContext)
    {
        VecsDbReleaseContext(pDbContext);
    }
    if (ppEntries)
    {
        VmAfdFreeCdcDbArrayEntries(dwCount, ppEntries);
    }
    return dwError;
error:

    if (ppCdcDbEntryArray)
    {
        *ppCdcDbEntryArray = NULL;
    }
    if (pCdcDbEntryArray)
    {
        VmAfdFreeCdcDbEntryArray(pCdcDbEntryArray);
    }
    goto cleanup;
}

static
DWORD
CdcDbUpdateServiceStatus(
    PVECS_DB_CONTEXT pDbContext,
    PCDC_DB_ENTRY_W  pCdcDbEntry,
    PVMAFD_HB_INFO_W pHeartbeatInfo
    )
{
    DWORD dwError = 0;

    sqlite3_stmt* pDbQuery = NULL;

    char szQuery[] = "INSERT OR REPLACE INTO DCServiceStatus("
                     " DCID,"
                     " ServiceName,"
                     " Port,"
                     " IsAlive,"
                     " LastHeartbeat)"
                     " VALUES ("
                     " (SELECT DCID from DCTable WHERE"
                     " DCName = :dcName AND"
                     " Domain = :domainName),"
                     " :serviceName,"
                     " :port,"
                     " :isAlive,"
                     " :lastHeartbeat);";

    if (!pDbContext ||
        !pCdcDbEntry ||
        !pHeartbeatInfo
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if (pDbContext->dbOpenMode != VMAFD_DB_MODE_WRITE)
    {
        dwError = ERROR_INVALID_ACCESS;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = sqlite3_prepare_v2(
                          pDbContext->pDb,
                          szQuery,
                          -1,
                          &pDbQuery,
                          NULL
                          );
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VecsBindWideString(
                            pDbQuery,
                            ":dcName",
                            pCdcDbEntry->pszDCName
                            );
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VecsBindWideString(
                            pDbQuery,
                            ":domainName",
                            pCdcDbEntry->pszDomainName
                            );
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VecsBindWideString(
                            pDbQuery,
                            ":serviceName",
                            pHeartbeatInfo->pszServiceName
                            );
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VecsBindDword(
                          pDbQuery,
                          ":port",
                          pHeartbeatInfo->dwPort
                          );
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VecsBindDword(
                          pDbQuery,
                          ":isAlive",
                          pHeartbeatInfo->bIsAlive
                          );
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VecsBindDword(
                          pDbQuery,
                          ":lastHeartbeat",
                          pHeartbeatInfo->dwLastHeartbeat
                          );
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VecsDbStepSql(pDbQuery);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    if (pDbQuery)
    {
        sqlite3_finalize (pDbQuery);
    }
    return dwError;
error:

    goto cleanup;
}

static
DWORD
CdcDbGetHeartbeatStatusCount(
    PVECS_DB_CONTEXT pDbContext,
    PWSTR  pwszDCName,
    PWSTR  pwszDomainName,
    PDWORD pdwCount
    )
{
    DWORD dwError = 0;
    DWORD dwCount = 0;
    sqlite3_stmt* pDbQuery = NULL;

    char szQuery[] = "SELECT COUNT(*) FROM("
                     "SELECT * FROM DCServiceStatus"
                     " WHERE DCID = ("
                     " SELECT DCID from DCTable"
                     " WHERE DCName = :dcName"
                     " AND Domain = :domainName));";

    if (!pDbContext || !pdwCount)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = sqlite3_prepare_v2(
                pDbContext->pDb,
                szQuery,
                -1,
                &pDbQuery,
                NULL);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VecsBindWideString(
                            pDbQuery,
                            ":dcName",
                            pwszDCName
                            );
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VecsBindWideString(
                            pDbQuery,
                            ":domainName",
                            pwszDomainName
                            );
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VecsDbStepSql(pDbQuery);
    if (dwError == SQLITE_ROW)
    {
        dwError = 0;
        dwCount = sqlite3_column_int(pDbQuery, 0);
    }
    BAIL_ON_VMAFD_ERROR (dwError);

    *pdwCount = dwCount;
cleanup:

    if (pDbQuery)
    {
      sqlite3_finalize (pDbQuery);
    }

    return dwError;
error:

    if (pdwCount)
    {
        *pdwCount = 0;
    }
    goto cleanup;
}

