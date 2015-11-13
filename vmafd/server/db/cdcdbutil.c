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

DWORD
CdcRegDbGetHAMode(
    PBOOL pbHAState
    )
{
    DWORD dwError = 0;
    DWORD dwHAState = 0;
    BOOL bHAState = FALSE;

    if (!pbHAState)
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
    }
    BAIL_ON_VMAFD_ERROR(dwError);

    bHAState = dwHAState? TRUE:FALSE;

    *pbHAState = bHAState;

cleanup:
    return dwError;

error:
    if (pbHAState)
    {
        *pbHAState = 0;
    }

    goto cleanup;
}

DWORD
CdcRegDbSetHAMode(
    BOOL bHAState
    )
{
    DWORD dwError = 0;

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

    dwError = VecsDbCreateContext(&pDbContext);
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

    dwError = VecsDbCreateContext(&pDbContext);
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
    BAIL_ON_VMAFD_ERROR(dwError);

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

    dwError = VecsDbCreateContext(&pDbContext);
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
    BAIL_ON_VMAFD_ERROR(dwError);

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

    dwError = VecsDbCreateContext(&pDbContext);
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

    dwError = VecsDbCreateContext(&pDbContext);
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
                     " IsAlive)"
                     " VALUES ("
                     " :dcName,"
                     " :site,"
                     " :domainName,"
                     " :lastPing,"
                     " :pingResponse,"
                     " :isAlive);";

    if (!pCdcEntry)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VecsDbCreateContext(&pDbContext);
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
                     " IsAlive = :isAlive"
                     " WHERE"
                     " DCName = :dcName AND"
                     " Domain = :domainName;";

    if (!pCdcEntry)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VecsDbCreateContext(&pDbContext);
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

    dwError = VecsDbCreateContext(&pDbContext);
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
    BAIL_ON_VMAFD_ERROR(dwError);

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

    dwError = VecsDbCreateContext(&pDbContext);
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
    PWSTR **pppszEntryNames,
    PDWORD pdwCount
    )
{
    DWORD dwError = 0;
    PVECS_DB_CONTEXT pDbContext = NULL;
    sqlite3_stmt* pDbQuery = NULL;
    PWSTR *ppszEntryNames = NULL;
    DWORD dwCount = 0;
    DWORD dwDCCount = 0;


    if (!pppszEntryNames || !pdwCount)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VecsDbCreateContext(&pDbContext);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = CdcDbGetDCEntryCount(pDbContext, &dwCount);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (dwCount)
    {
        char szQuery[] = " SELECT DCName FROM DCTable LIMIT :count;";
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
                              sizeof(PWSTR)*dwCount,
                              (PVOID*)&ppszEntryNames
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
                                        &ppszEntryNames[dwDCCount]
                                        );
                BAIL_ON_VMAFD_ERROR(dwError);

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
    *pppszEntryNames = ppszEntryNames;

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

    if (pppszEntryNames)
    {
        *pppszEntryNames = NULL;
    }
    if (pdwCount)
    {
        *pdwCount = 0;
    }
    VmAfdFreeStringArrayW(ppszEntryNames, dwDCCount);
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
                     " ORDER BY PingResponse;";

    if (!ppszDCName ||
        IsNullOrEmptyString(pwszClientSiteName) ||
        IsNullOrEmptyString(pwszDomainName)
        )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VecsDbCreateContext(&pDbContext);
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
    BAIL_ON_VMAFD_ERROR(dwError);

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

    char szQuery[] = "SELECT DCName FROM DCTable"
                     " WHERE IsAlive = 1 AND"
                     " Domain = :domainName"
                     " ORDER BY PingResponse;";

    if (!ppszDCName ||
        IsNullOrEmptyString(pwszDomainName)
        )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VecsDbCreateContext(&pDbContext);
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
    BAIL_ON_VMAFD_ERROR(dwError);

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
