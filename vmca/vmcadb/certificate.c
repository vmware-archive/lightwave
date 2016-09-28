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



/*
 * Module Name: VMware Certificate Server
 *
 * Filename: application.c
 *
 * Abstract:
 *
 * VMware Certificate Server Database
 *
 * Certificate persistence
 *
 */

#include "includes.h"
#include <vmca.h>

#ifdef _WIN32
#pragma warning(disable : 4996 4995)
#endif

static
DWORD
VmcaDbGetCertID(
    PVMCA_DB_CONTEXT pDbContext,
    PCWSTR pwszSerial,
    PCWSTR pwszIssuerName,
    PDWORD pdwCertId
    );

static
DWORD
VmcaDbGetRevokedCertsCount(
    PVMCA_DB_CONTEXT pDbContext,
    PDWORD           pdwCount
    );

//TODO: This should change to something
//like in VmAfd.
static
DWORD
VmcaDbGetWStringAtIndex(
    sqlite3_stmt* pDbQuery,
    DWORD dwColumnIndex,
    PWSTR *ppszValue
    );


static
DWORD
VmcaDbGetColumnBlobAtIndex(
    sqlite3_stmt* pSqlStatement,
    DWORD dwColumnIndex,
    PBYTE* ppszValue,
    PDWORD pdwLen
    );


DWORD
VmcaDbAddCertificate(
    PVMCA_DB_CONTEXT           pDbContext,
    PVMCA_DB_CERTIFICATE_ENTRY pCertEntry
    )
{
    DWORD dwError = 0;
    BOOLEAN bInTx = FALSE;
    sqlite3_stmt* pDbQuery = NULL;
    int index = 1;

    if (!pDbContext || !pCertEntry)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    // TODO : Validate Cert Entry

    if (!pDbContext->pAddCertQuery)
    {
        CHAR szQuery[] =
                "INSERT INTO CertTable "
                "(CommonName,"
                " AltNames,"
                " OrgName,"
                " OrgUnitName,"
                " IssuerName,"
                " Country,"
                " Serial,"
                " ValidFrom,"
                " ValidUntil,"
                " CertSize,"
                " CertBlob) "
                "VALUES( ?1, ?2, ?3, ?4, ?5, ?6, ?7, ?8, ?9, ?10, ?11);";

        dwError = sqlite3_prepare_v2(
                        pDbContext->pDb,
                        szQuery,
                        -1,
                        &pDbContext->pAddCertQuery,
                        NULL);
        BAIL_ON_VMCA_ERROR(dwError);
    }

    pDbQuery = pDbContext->pAddCertQuery;

    if (!pDbContext->bInTx)
    {
        dwError = VmcaDbBeginTransaction(pDbContext->pDb);
        BAIL_ON_VMCA_ERROR(dwError);

        bInTx = TRUE;
    }

    dwError = sqlite3_bind_text16(
                            pDbQuery,
                            index++,
                            pCertEntry->pwszCommonName,
                            -1,
                            SQLITE_TRANSIENT);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = sqlite3_bind_text16(
                            pDbQuery,
                            index++,
                            pCertEntry->pwszAltNames,
                            -1,
                            SQLITE_TRANSIENT);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = sqlite3_bind_text16(
                            pDbQuery,
                            index++,
                            pCertEntry->pwszOrgName,
                            -1,
                            SQLITE_TRANSIENT);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = sqlite3_bind_text16(
                            pDbQuery,
                            index++,
                            pCertEntry->pwszOrgUnitName,
                            -1,
                            SQLITE_TRANSIENT);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = sqlite3_bind_text16(
                            pDbQuery,
                            index++,
                            pCertEntry->pwszIssuerName,
                            -1,
                            SQLITE_TRANSIENT);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = sqlite3_bind_text16(
                            pDbQuery,
                            index++,
                            pCertEntry->pwszCountryName,
                            -1,
                            SQLITE_TRANSIENT);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = sqlite3_bind_text16(
                            pDbQuery,
                            index++,
                            pCertEntry->pwszSerial,
                            -1,
                            SQLITE_TRANSIENT);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = sqlite3_bind_text16(
                            pDbQuery,
                            index++,
                            pCertEntry->pwszTimeValidFrom,
                            -1,
                            SQLITE_TRANSIENT);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = sqlite3_bind_text16(
                            pDbQuery,
                            index++,
                            pCertEntry->pwszTimeValidTo,
                            -1,
                            SQLITE_TRANSIENT);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = sqlite3_bind_int(
                            pDbQuery,
                            index++,
                            pCertEntry->dwCertSize);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = sqlite3_bind_blob(
                        pDbQuery,
                        index++,
                        pCertEntry->pCertBlob,
                        pCertEntry->dwCertSize,
                        SQLITE_TRANSIENT);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VmcaDbStepSql(pDbQuery);
    BAIL_ON_VMCA_ERROR(dwError);

    if (!pDbContext->bInTx && bInTx)
    {
        dwError = VmcaDbCommitTransaction(pDbContext->pDb);
        BAIL_ON_VMCA_ERROR(dwError);
    }

cleanup:

    if (pDbQuery)
    {
        sqlite3_finalize(pDbQuery);
        pDbQuery = NULL;
        pDbContext->pAddCertQuery = NULL;
    }

    return dwError;

error:

    if (!pDbContext->bInTx && bInTx)
    {
        VmcaDbRollbackTransaction(pDbContext->pDb);
    }

    goto cleanup;
}

DWORD
VmcaDbGetTotalCertificateCount(
    PVMCA_DB_CONTEXT pDbContext,
    PDWORD           pdwNumCerts,
    DWORD            dwFilter
    )
{
    DWORD dwError = 0;
    DWORD dwNumRows = 0;

    PSTR szRevokedQuery = "select count(*) from CertTable "
                          " left outer join RevokedCertsTable "
                          "         ON CertTable.ID = RevokedCertsTable.CertID "
                          " where RevokedReason is not null;";
    PSTR szActiveQuery  = "select count(*) from CertTable "
                          " left outer join RevokedCertsTable "
                          "         ON CertTable.ID = RevokedCertsTable.CertID "
                          " where RevokedReason is null;";
    PSTR szAllQuery     = "select count(*) from CertTable;";

    PSTR szQuery = szAllQuery;
    if( dwFilter == VMCA_CERTIFICATE_ALL) {
        szQuery = szAllQuery;
    }

    if ( dwFilter == VMCA_CERTIFICATE_ACTIVE) {
        szQuery = szActiveQuery;
    }

    if( dwFilter == VMCA_CERTIFICATE_REVOKED) {
        szQuery = szRevokedQuery;
    }

    dwError = sqlite3_prepare_v2(
                        pDbContext->pDb,
                        szQuery,
                        -1,
                        &pDbContext->pTotalCertCountQuery,
                        NULL);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VmcaDbStepSql(pDbContext->pTotalCertCountQuery);
    if (dwError == SQLITE_ROW)
    {
        assert(sqlite3_column_count(pDbContext->pTotalCertCountQuery) == 1);

        dwNumRows = sqlite3_column_int(pDbContext->pTotalCertCountQuery, 0);

        dwError = ERROR_SUCCESS;
    }
    BAIL_ON_VMCA_ERROR(dwError);

    *pdwNumCerts = dwNumRows;
cleanup:

    sqlite3_reset(pDbContext->pTotalCertCountQuery);

    return dwError;

error:

    *pdwNumCerts = 0;

    goto cleanup;
}

DWORD
VmcaDbQueryAllCertificates(
    PVMCA_DB_CONTEXT            pDbContext,
    PVMCA_DB_CERTIFICATE_ENTRY* ppCertEntryArray,
    PDWORD                      pdwCount
    )
{
    DWORD dwError = 0;
    PVMCA_DB_CERTIFICATE_ENTRY pCertEntryArray = NULL;
    DWORD iEntry = 0;
    DWORD dwTotalEntries = 0;
    DWORD dwEntriesAvailable = 0;

#ifndef _WIN32
    VMCA_DB_COLUMN_VALUE commonName =
    {
        .data.ppwszValue   = NULL,
        .dataLength    = 0
    };
    VMCA_DB_COLUMN_VALUE altNames =
    {
        .data.ppwszValue   = NULL,
        .dataLength    = 0
    };
    VMCA_DB_COLUMN_VALUE orgName =
    {
        .data.ppwszValue   = NULL,
        .dataLength    = 0
    };
    VMCA_DB_COLUMN_VALUE orgUnitName =
    {
        .data.ppwszValue   = NULL,
        .dataLength    = 0
    };
    VMCA_DB_COLUMN_VALUE country =
    {
        .data.ppwszValue   = NULL,
        .dataLength    = 0
    };
    VMCA_DB_COLUMN_VALUE issuer =
    {
        .data.ppwszValue   = NULL,
        .dataLength    = 0
    };
    VMCA_DB_COLUMN_VALUE serial =
    {
        .data.ppwszValue   = NULL,
        .dataLength    = 0
    };
    VMCA_DB_COLUMN_VALUE certSize =
    {
        .data.ppdwValue   = NULL,
        .dataLength    = 0
    };
    VMCA_DB_COLUMN_VALUE validFrom =
    {
        .data.ppwszValue   = NULL,
        .dataLength    = 0
    };
    VMCA_DB_COLUMN_VALUE validUntil =
    {
        .data.ppwszValue   = NULL,
        .dataLength    = 0
    };
    VMCA_DB_COLUMN_VALUE certBlob =
    {
        .data.ppValue   = NULL,
        .dataLength    = 0
    };

    VMCA_DB_COLUMN_VALUE revoked =
    {
        .data.ppdwValue   = NULL,
        .dataLength    = 0
    };



    VMCA_DB_COLUMN columns[] =
    {
        {
            .pszName = "CommonName",
            .columnType = VMCA_DB_COLUMN_TYPE_STRING,
            .pValue = &commonName
        },
        {
            .pszName = "AltNames",
            .columnType = VMCA_DB_COLUMN_TYPE_STRING,
            .pValue = &altNames
        },
        {
            .pszName = "OrgName",
            .columnType = VMCA_DB_COLUMN_TYPE_STRING,
            .pValue = &orgName
        },
        {
            .pszName = "OrgUnitName",
            .columnType = VMCA_DB_COLUMN_TYPE_STRING,
            .pValue = &orgUnitName
        },
        {
            .pszName = "IssuerName",
            .columnType = VMCA_DB_COLUMN_TYPE_STRING,
            .pValue = &issuer
        },
        {
            .pszName = "Country",
            .columnType = VMCA_DB_COLUMN_TYPE_STRING,
            .pValue = &country
        },
        {
            .pszName = "Serial",
            .columnType = VMCA_DB_COLUMN_TYPE_STRING,
            .pValue = &serial
        },
        {
            .pszName = "CertSize",
            .columnType = VMCA_DB_COLUMN_TYPE_INT32,
            .pValue = &certSize
        },
        {
            .pszName = "ValidFrom",
            .columnType = VMCA_DB_COLUMN_TYPE_STRING,
            .pValue = &validFrom
        },
        {
            .pszName = "ValidUntil",
            .columnType = VMCA_DB_COLUMN_TYPE_STRING,
            .pValue = &validUntil
        },
        {
            .pszName = "CertBlob",
            .columnType = VMCA_DB_COLUMN_TYPE_BLOB,
            .pValue = &certBlob
        },
         {
            .pszName = "RevokedReason",
            .columnType = VMCA_DB_COLUMN_TYPE_INT32,
            .pValue = &revoked
        },


    };

#else

    VMCA_DB_COLUMN_VALUE commonName =
    {
        VMCA_DB_INIT(.data.ppwszValue, NULL),
        VMCA_DB_INIT(.dataLength,0)
    };
    VMCA_DB_COLUMN_VALUE altNames =
    {
        VMCA_DB_INIT(.data.ppwszValue, NULL),
        VMCA_DB_INIT(.dataLength    , 0)
    };
    VMCA_DB_COLUMN_VALUE orgName =
    {
        VMCA_DB_INIT(.data.ppwszValue, NULL),
        VMCA_DB_INIT(.dataLength    , 0)
    };
    VMCA_DB_COLUMN_VALUE orgUnitName =
    {
        VMCA_DB_INIT(.data.ppwszValue   , NULL),
        VMCA_DB_INIT(.dataLength    , 0)
    };
    VMCA_DB_COLUMN_VALUE country =
    {
        VMCA_DB_INIT(.data.ppwszValue   , NULL),
        VMCA_DB_INIT(.dataLength    , 0)
    };
    VMCA_DB_COLUMN_VALUE issuer =
    {
        VMCA_DB_INIT(.data.ppwszValue   , NULL),
        VMCA_DB_INIT(.dataLength    , 0)
    };
    VMCA_DB_COLUMN_VALUE serial =
    {
        VMCA_DB_INIT(.data.ppwszValue   , NULL),
        VMCA_DB_INIT(.dataLength    , 0)
    };
    VMCA_DB_COLUMN_VALUE certSize =
    {
        VMCA_DB_INIT(.data.ppdwValue   , NULL),
        VMCA_DB_INIT(.dataLength    , 0)
    };
    VMCA_DB_COLUMN_VALUE validFrom =
    {
        VMCA_DB_INIT(.data.ppwszValue   , NULL),
        VMCA_DB_INIT(.dataLength    , 0)
    };
    VMCA_DB_COLUMN_VALUE validUntil =
    {
        VMCA_DB_INIT(.data.ppwszValue   , NULL),
        VMCA_DB_INIT(.dataLength    , 0)
    };
    VMCA_DB_COLUMN_VALUE certBlob =
    {
        VMCA_DB_INIT(.data.ppValue   , NULL),
        VMCA_DB_INIT(.dataLength    , 0)
    };

    VMCA_DB_COLUMN_VALUE revoked =
    {
        VMCA_DB_INIT(.data.ppdwValue   , NULL),
        VMCA_DB_INIT(.dataLength    , 0)
    };

    VMCA_DB_COLUMN columns[] =
    {
        {
            VMCA_DB_INIT(.pszName , "CommonName"),
            VMCA_DB_INIT(.columnType , VMCA_DB_COLUMN_TYPE_STRING),
            VMCA_DB_INIT(.pValue , &commonName)
        },
        {
            VMCA_DB_INIT(.pszName , "AltNames"),
            VMCA_DB_INIT(.columnType , VMCA_DB_COLUMN_TYPE_STRING),
            VMCA_DB_INIT(.pValue , &altNames),
        },
        {
            VMCA_DB_INIT(.pszName , "OrgName"),
            VMCA_DB_INIT(.columnType , VMCA_DB_COLUMN_TYPE_STRING),
            VMCA_DB_INIT(.pValue , &orgName)
        },
        {
            VMCA_DB_INIT(.pszName , "OrgUnitName"),
            VMCA_DB_INIT(.columnType , VMCA_DB_COLUMN_TYPE_STRING),
            VMCA_DB_INIT(.pValue , &orgUnitName),
        },
        {
            VMCA_DB_INIT(.pszName , "IssuerName"),
            VMCA_DB_INIT(.columnType , VMCA_DB_COLUMN_TYPE_STRING),
            VMCA_DB_INIT(.pValue , &issuer)
        },
        {
            VMCA_DB_INIT(.pszName , "Country"),
            VMCA_DB_INIT(.columnType , VMCA_DB_COLUMN_TYPE_STRING),
            VMCA_DB_INIT(.pValue , &country)
        },
        {
            VMCA_DB_INIT(.pszName , "Serial"),
            VMCA_DB_INIT(.columnType , VMCA_DB_COLUMN_TYPE_STRING),
            VMCA_DB_INIT(.pValue , &serial)
        },
        {
            VMCA_DB_INIT(.pszName , "CertSize"),
            VMCA_DB_INIT(.columnType , VMCA_DB_COLUMN_TYPE_INT32),
            VMCA_DB_INIT(.pValue , &certSize)
        },
        {
            VMCA_DB_INIT(.pszName , "ValidFrom"),
            VMCA_DB_INIT(.columnType , VMCA_DB_COLUMN_TYPE_STRING),
            VMCA_DB_INIT(.pValue , &validFrom)
        },
        {
            VMCA_DB_INIT(.pszName , "ValidUntil"),
            VMCA_DB_INIT(.columnType , VMCA_DB_COLUMN_TYPE_STRING),
            VMCA_DB_INIT(.pValue , &validUntil)
        },
        {
            VMCA_DB_INIT(.pszName , "CertBlob"),
            VMCA_DB_INIT(.columnType , VMCA_DB_COLUMN_TYPE_BLOB),
            VMCA_DB_INIT(.pValue , &certBlob)
        },
         {
            VMCA_DB_INIT(.pszName , "RevokedReason"),
            VMCA_DB_INIT(.columnType , VMCA_DB_COLUMN_TYPE_INT32),
            VMCA_DB_INIT(.pValue , &revoked)
        },
    };

#endif //_WIN32

    if (!pDbContext->pQueryAllCertificates)
    {
        CHAR szQuery[] = "select "
                                 "CommonName,"
                                 "AltNames,"
                                 "OrgName,"
                                 "OrgUnitName,"
                                 "IssuerName,"
                                 "Country,"
                                 "Serial,"
                                 "ValidFrom,"
                                 "ValidUntil,"
                                 "CertSize,"
                                 "CertBlob,"
                                 "case when RevokedReason is null then 0 else 1 end as RevokedReason "
                         " from CertTable "
                         " left outer join RevokedCertsTable "
                          "         ON CertTable.ID = RevokedCertsTable.CertID ;";

        dwError = sqlite3_prepare_v2(
                        pDbContext->pDb,
                        szQuery,
                        -1,
                        &pDbContext->pQueryAllCertificates,
                        NULL);
        BAIL_ON_VMCA_ERROR(dwError);
    }

    do
    {
        dwError = VmcaDbStepSql(pDbContext->pQueryAllCertificates);

        if (dwError == SQLITE_ROW)
        {
            PVMCA_DB_CERTIFICATE_ENTRY pCertEntry = NULL;
            PDWORD   pdwCertSize     = NULL;
           // PWSTR pwszValidFrom  = NULL;
            //PWSTR pwszValidUntil = NULL;

            if (!dwEntriesAvailable)
            {
                PVMCA_DB_CERTIFICATE_ENTRY pNewArray = NULL;
                DWORD dwNumIncr    = 10;
                DWORD dwNumEntries = dwTotalEntries + dwNumIncr;

                dwError = VMCAAllocateMemory(
                                sizeof(*pNewArray) * dwNumEntries,
                                (PVOID*)&pNewArray);
                BAIL_ON_VMCA_ERROR(dwError);

                if (pCertEntryArray)
                {
                    memcpy( (PBYTE)pNewArray,
                            (PBYTE)pCertEntryArray,
                            sizeof(*pCertEntry) * dwTotalEntries);

                    VMCAFreeMemory(pCertEntryArray);
                }

                pCertEntryArray      = pNewArray;
                dwTotalEntries     += dwNumIncr;
                dwEntriesAvailable += dwNumIncr;
            }

            pCertEntry = &pCertEntryArray[iEntry++];
            dwEntriesAvailable--;

            commonName.data.ppwszValue   = &pCertEntry->pwszCommonName;
            commonName.dataLength        = 0;

            altNames.data.ppwszValue     = &pCertEntry->pwszAltNames;
            altNames.dataLength          = 0;

            orgName.data.ppwszValue      = &pCertEntry->pwszOrgName;
            orgName.dataLength           = 0;

            orgUnitName.data.ppwszValue  = &pCertEntry->pwszOrgUnitName;
            orgUnitName.dataLength       = 0;

            issuer.data.ppwszValue       = &pCertEntry->pwszIssuerName;
            issuer.dataLength            = 0;

            serial.data.ppwszValue       = &pCertEntry->pwszSerial;
            serial.dataLength            = 0;

            country.data.ppwszValue      = &pCertEntry->pwszCountryName;
            country.dataLength           = 0;

            pdwCertSize                  = &pCertEntry->dwCertSize;
            certSize.data.ppdwValue      = &pdwCertSize;
            certSize.dataLength          = sizeof(pCertEntry->dwCertSize);

            validFrom.data.ppwszValue       = &pCertEntry->pwszTimeValidFrom;
            validFrom.dataLength            = 0;

            validUntil.data.ppwszValue       = &pCertEntry->pwszTimeValidTo;
            validUntil.dataLength            = 0;

            certBlob.data.ppValue       = &pCertEntry->pCertBlob;
            certBlob.dataLength         = 0;

            dwError = VmcaDbFillValues(
                            pDbContext->pQueryAllCertificates,
                            &columns[0],
                            sizeof(columns)/sizeof(columns[0]));
            BAIL_ON_VMCA_ERROR(dwError);
            pCertEntry->dwCertSize = (DWORD)certBlob.dataLength;
        }
        else if (dwError == ERROR_SUCCESS)
        {
            break;
        }
        BAIL_ON_VMCA_ERROR(dwError);

    } while (dwError != SQLITE_DONE);

    *ppCertEntryArray = pCertEntryArray;
    *pdwCount = iEntry;

cleanup:

    if (pDbContext->pQueryAllCertificates)
    {
        sqlite3_reset(pDbContext->pQueryAllCertificates);
    }

    return dwError;

error:

    *ppCertEntryArray = NULL;
    *pdwCount = 0;

    if (pCertEntryArray)
    {
        VmcaDbFreeCertEntryArray(pCertEntryArray, dwTotalEntries);
    }

    goto cleanup;
}

static
PCSTR VMCAGetDbEnumQuery(DWORD dwStatus)
{
    PCSTR szQuery;
    if (dwStatus == VMCA_DB_CERTIFICATE_STATUS_ALL)
    {
        szQuery=
            " select  "
            "      CommonName, "
            "      AltNames, "
            "      OrgName, "
            "      OrgUnitName, "
            "      IssuerName, "
            "      Country, "
            "      CertTable.Serial, "
            "      ValidFrom, "
            "      ValidUntil, "
            "      CertSize, "
            "      CertBlob, "
            "      case when RevokedReason is null then 0 else 1 end as RevokedReason "
            " from CertTable "
            " left outer join RevokedCertsTable "
            "      on CertTable.ID = RevokedCertsTable.CertID "
            "limit ?1 offset ?2;";
    }
    else if (dwStatus == VMCA_DB_CERTIFICATE_STATUS_REVOKED)
    {
        szQuery=
            " select  "
            "      CommonName, "
            "      AltNames, "
            "      OrgName, "
            "      OrgUnitName, "
            "      IssuerName, "
            "      Country, "
            "      CertTable.Serial, "
            "      ValidFrom, "
            "      ValidUntil, "
            "      CertSize, "
            "      CertBlob, "
            "      case when RevokedReason is null then 0 else 1 end as RevokedReason "
            " from CertTable "
            " left outer join RevokedCertsTable "
            "      on CertTable.ID = RevokedCertsTable.CertID "
            " where RevokedReason is not null "
            "limit ?1 offset ?2;";
    }
    else if (dwStatus == VMCA_DB_CERTIFICATE_STATUS_ACTIVE)
    {
        szQuery=
            " select  "
            "      CommonName, "
            "      AltNames, "
            "      OrgName, "
            "      OrgUnitName, "
            "      IssuerName, "
            "      Country, "
            "      CertTable.Serial, "
            "      ValidFrom, "
            "      ValidUntil, "
            "      CertSize, "
            "      CertBlob, "
            "      case when RevokedReason is null then 0 else 1 end as RevokedReason "
            " from CertTable "
            " left outer join RevokedCertsTable "
            "      on CertTable.ID = RevokedCertsTable.CertID "
            " where RevokedReason is null "
            "limit ?1 offset ?2;";
    }
    else if (dwStatus == VMCA_DB_CERTIFICATE_STATUS_EXPIRED)
    {
        szQuery=
            " select  "
            "      CommonName, "
            "      AltNames, "
            "      OrgName, "
            "      OrgUnitName, "
            "      IssuerName, "
            "      Country, "
            "      CertTable.Serial, "
            "      ValidFrom, "
            "      ValidUntil, "
            "      CertSize, "
            "      CertBlob, "
            "      case when RevokedReason is null then 0 else 1 end as RevokedReason "
            " from CertTable "
            " left outer join RevokedCertsTable "
            "      on CertTable.ID = RevokedCertsTable.CertID "
            " where ValidUntil < date('now') "
            "limit ?1 offset ?2;";
    }

    return szQuery;
}

DWORD
VmcaDbQueryCertificatesPaged(
    PVMCA_DB_CONTEXT         pDbContext,
    DWORD                    dwStartIndex,
    DWORD                    dwNumPackages,
    VMCA_DB_CERTIFICATE_STATUS       dwStatus,
    PVMCA_DB_CERTIFICATE_ENTRY*  ppCertEntryArray,
    PDWORD                   pdwCount
    )
{
    DWORD dwError = 0;
    PVMCA_DB_CERTIFICATE_ENTRY pCertEntryArray = NULL;
    DWORD iEntry = 0;
    DWORD dwTotalEntries = 0;
    DWORD dwEntriesAvailable = 0;

#ifndef _WIN32
    VMCA_DB_COLUMN_VALUE commonName =
    {
        .data.ppwszValue   = NULL,
        .dataLength    = 0
    };
    VMCA_DB_COLUMN_VALUE altNames =
    {
        .data.ppwszValue   = NULL,
        .dataLength    = 0
    };
    VMCA_DB_COLUMN_VALUE orgName =
    {
        .data.ppwszValue   = NULL,
        .dataLength    = 0
    };
    VMCA_DB_COLUMN_VALUE orgUnitName =
    {
        .data.ppwszValue   = NULL,
        .dataLength    = 0
    };
    VMCA_DB_COLUMN_VALUE country =
    {
        .data.ppwszValue   = NULL,
        .dataLength    = 0
    };
    VMCA_DB_COLUMN_VALUE issuer =
    {
        .data.ppwszValue   = NULL,
        .dataLength    = 0
    };
    VMCA_DB_COLUMN_VALUE serial =
    {
        .data.ppwszValue   = NULL,
        .dataLength    = 0
    };
    VMCA_DB_COLUMN_VALUE certSize =
    {
        .data.ppdwValue   = NULL,
        .dataLength    = 0
    };
    VMCA_DB_COLUMN_VALUE validFrom =
    {
        .data.ppwszValue   = NULL,
        .dataLength    = 0
    };
    VMCA_DB_COLUMN_VALUE validUntil =
    {
        .data.ppwszValue   = NULL,
        .dataLength    = 0
    };
    VMCA_DB_COLUMN_VALUE certBlob =
    {
        .data.ppValue   = NULL,
        .dataLength    = 0
    };

    VMCA_DB_COLUMN_VALUE revoked =
    {
        .data.ppdwValue   = NULL,
        .dataLength    = 0
    };



    VMCA_DB_COLUMN columns[] =
    {
        {
            .pszName = "CommonName",
            .columnType = VMCA_DB_COLUMN_TYPE_STRING,
            .pValue = &commonName
        },
        {
            .pszName = "AltNames",
            .columnType = VMCA_DB_COLUMN_TYPE_STRING,
            .pValue = &altNames
        },
        {
            .pszName = "OrgName",
            .columnType = VMCA_DB_COLUMN_TYPE_STRING,
            .pValue = &orgName
        },
        {
            .pszName = "OrgUnitName",
            .columnType = VMCA_DB_COLUMN_TYPE_STRING,
            .pValue = &orgUnitName
        },
        {
            .pszName = "IssuerName",
            .columnType = VMCA_DB_COLUMN_TYPE_STRING,
            .pValue = &issuer
        },
        {
            .pszName = "Country",
            .columnType = VMCA_DB_COLUMN_TYPE_STRING,
            .pValue = &country
        },
        {
            .pszName = "Serial",
            .columnType = VMCA_DB_COLUMN_TYPE_STRING,
            .pValue = &serial
        },
        {
            .pszName = "CertSize",
            .columnType = VMCA_DB_COLUMN_TYPE_INT32,
            .pValue = &certSize
        },
        {
            .pszName = "ValidFrom",
            .columnType = VMCA_DB_COLUMN_TYPE_STRING,
            .pValue = &validFrom
        },
        {
            .pszName = "ValidUntil",
            .columnType = VMCA_DB_COLUMN_TYPE_STRING,
            .pValue = &validUntil
        },
        {
            .pszName = "CertBlob",
            .columnType = VMCA_DB_COLUMN_TYPE_BLOB,
            .pValue = &certBlob
        },
         {
            .pszName = "RevokedReason",
            .columnType = VMCA_DB_COLUMN_TYPE_INT32,
            .pValue = &revoked
        },


    };

#else

    VMCA_DB_COLUMN_VALUE commonName =
    {
        VMCA_DB_INIT(.data.ppwszValue, NULL),
        VMCA_DB_INIT(.dataLength,0)
    };
    VMCA_DB_COLUMN_VALUE altNames =
    {
        VMCA_DB_INIT(.data.ppwszValue, NULL),
        VMCA_DB_INIT(.dataLength    , 0)
    };
    VMCA_DB_COLUMN_VALUE orgName =
    {
        VMCA_DB_INIT(.data.ppwszValue, NULL),
        VMCA_DB_INIT(.dataLength    , 0)
    };
    VMCA_DB_COLUMN_VALUE orgUnitName =
    {
        VMCA_DB_INIT(.data.ppwszValue   , NULL),
        VMCA_DB_INIT(.dataLength    , 0)
    };
    VMCA_DB_COLUMN_VALUE country =
    {
        VMCA_DB_INIT(.data.ppwszValue   , NULL),
        VMCA_DB_INIT(.dataLength    , 0)
    };
    VMCA_DB_COLUMN_VALUE issuer =
    {
        VMCA_DB_INIT(.data.ppwszValue   , NULL),
        VMCA_DB_INIT(.dataLength    , 0)
    };
    VMCA_DB_COLUMN_VALUE serial =
    {
        VMCA_DB_INIT(.data.ppwszValue   , NULL),
        VMCA_DB_INIT(.dataLength    , 0)
    };
    VMCA_DB_COLUMN_VALUE certSize =
    {
        VMCA_DB_INIT(.data.ppdwValue   , NULL),
        VMCA_DB_INIT(.dataLength    , 0)
    };
    VMCA_DB_COLUMN_VALUE validFrom =
    {
        VMCA_DB_INIT(.data.ppwszValue   , NULL),
        VMCA_DB_INIT(.dataLength    , 0)
    };
    VMCA_DB_COLUMN_VALUE validUntil =
    {
        VMCA_DB_INIT(.data.ppwszValue   , NULL),
        VMCA_DB_INIT(.dataLength    , 0)
    };
    VMCA_DB_COLUMN_VALUE certBlob =
    {
        VMCA_DB_INIT(.data.ppValue   , NULL),
        VMCA_DB_INIT(.dataLength    , 0)
    };

    VMCA_DB_COLUMN_VALUE revoked =
    {
        VMCA_DB_INIT(.data.ppdwValue   , NULL),
        VMCA_DB_INIT(.dataLength    , 0)
    };


    VMCA_DB_COLUMN columns[] =
    {
        {
            VMCA_DB_INIT(.pszName , "CommonName"),
            VMCA_DB_INIT(.columnType , VMCA_DB_COLUMN_TYPE_STRING),
            VMCA_DB_INIT(.pValue , &commonName)
        },
        {
            VMCA_DB_INIT(.pszName , "AltNames"),
            VMCA_DB_INIT(.columnType , VMCA_DB_COLUMN_TYPE_STRING),
            VMCA_DB_INIT(.pValue , &altNames),
        },
        {
            VMCA_DB_INIT(.pszName , "OrgName"),
            VMCA_DB_INIT(.columnType , VMCA_DB_COLUMN_TYPE_STRING),
            VMCA_DB_INIT(.pValue , &orgName)
        },
        {
            VMCA_DB_INIT(.pszName , "OrgUnitName"),
            VMCA_DB_INIT(.columnType , VMCA_DB_COLUMN_TYPE_STRING),
            VMCA_DB_INIT(.pValue , &orgUnitName),
        },
        {
            VMCA_DB_INIT(.pszName , "IssuerName"),
            VMCA_DB_INIT(.columnType , VMCA_DB_COLUMN_TYPE_STRING),
            VMCA_DB_INIT(.pValue , &issuer)
        },
        {
            VMCA_DB_INIT(.pszName , "Country"),
            VMCA_DB_INIT(.columnType , VMCA_DB_COLUMN_TYPE_STRING),
            VMCA_DB_INIT(.pValue , &country)
        },
        {
            VMCA_DB_INIT(.pszName , "Serial"),
            VMCA_DB_INIT(.columnType , VMCA_DB_COLUMN_TYPE_STRING),
            VMCA_DB_INIT(.pValue , &serial)
        },
        {
            VMCA_DB_INIT(.pszName , "CertSize"),
            VMCA_DB_INIT(.columnType , VMCA_DB_COLUMN_TYPE_INT32),
            VMCA_DB_INIT(.pValue , &certSize)
        },
        {
            VMCA_DB_INIT(.pszName , "ValidFrom"),
            VMCA_DB_INIT(.columnType , VMCA_DB_COLUMN_TYPE_STRING),
            VMCA_DB_INIT(.pValue , &validFrom)
        },
        {
            VMCA_DB_INIT(.pszName , "ValidUntil"),
            VMCA_DB_INIT(.columnType , VMCA_DB_COLUMN_TYPE_STRING),
            VMCA_DB_INIT(.pValue , &validUntil)
        },
        {
            VMCA_DB_INIT(.pszName , "CertBlob"),
            VMCA_DB_INIT(.columnType , VMCA_DB_COLUMN_TYPE_BLOB),
            VMCA_DB_INIT(.pValue , &certBlob)
        },
         {
            VMCA_DB_INIT(.pszName , "RevokedReason"),
            VMCA_DB_INIT(.columnType , VMCA_DB_COLUMN_TYPE_INT32),
            VMCA_DB_INIT(.pValue , &revoked)
        },
    };
#endif //_WIN32

    assert(dwNumPackages > 0);

    // in case of ALL, we have no status filter, otherwise appply it
    if (!pDbContext->pQueryCertificatesPaged)
    {
        PCSTR szQuery = VMCAGetDbEnumQuery(dwStatus);
        dwError = sqlite3_prepare_v2(
            pDbContext->pDb,
            szQuery,
            -1,
            &pDbContext->pQueryCertificatesPaged,
            NULL);
        BAIL_ON_VMCA_ERROR(dwError);
   }

    dwError = sqlite3_bind_int(
                    pDbContext->pQueryCertificatesPaged,
                    1,
                    dwNumPackages);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = sqlite3_bind_int(
                    pDbContext->pQueryCertificatesPaged,
                    2,
                    dwStartIndex);
    BAIL_ON_VMCA_ERROR(dwError);

    do
    {
        dwError = VmcaDbStepSql(pDbContext->pQueryCertificatesPaged);

        if (dwError == SQLITE_ROW)
        {
            PVMCA_DB_CERTIFICATE_ENTRY pCertEntry = NULL;
            PDWORD   pdwCertSize     = NULL;
            PDWORD   pdwRevoked      = NULL;

            if (!dwEntriesAvailable)
            {
                PVMCA_DB_CERTIFICATE_ENTRY pNewArray = NULL;
                DWORD dwNumIncr    = 10;
                DWORD dwNumEntries = dwTotalEntries + dwNumIncr;

                dwError = VMCAAllocateMemory(
                                sizeof(*pNewArray) * dwNumEntries,
                                (PVOID*)&pNewArray);
                BAIL_ON_VMCA_ERROR(dwError);

                if (pCertEntryArray)
                {
                    memcpy( (PBYTE)pNewArray,
                            (PBYTE)pCertEntryArray,
                            sizeof(*pCertEntry) * dwTotalEntries);

                    VMCAFreeMemory(pCertEntryArray);
                }

                pCertEntryArray      = pNewArray;
                dwTotalEntries     += dwNumIncr;
                dwEntriesAvailable += dwNumIncr;
            }

            pCertEntry = &pCertEntryArray[iEntry++];
            dwEntriesAvailable--;

            commonName.data.ppwszValue   = &pCertEntry->pwszCommonName;
            commonName.dataLength        = 0;

            altNames.data.ppwszValue     = &pCertEntry->pwszAltNames;
            altNames.dataLength          = 0;

            orgName.data.ppwszValue      = &pCertEntry->pwszOrgName;
            orgName.dataLength           = 0;

            orgUnitName.data.ppwszValue  = &pCertEntry->pwszOrgUnitName;
            orgUnitName.dataLength       = 0;

            issuer.data.ppwszValue       = &pCertEntry->pwszIssuerName;
            issuer.dataLength            = 0;

            serial.data.ppwszValue       = &pCertEntry->pwszSerial;
            serial.dataLength            = 0;

            country.data.ppwszValue      = &pCertEntry->pwszCountryName;
            country.dataLength           = 0;

            pdwCertSize                  = &pCertEntry->dwCertSize;
            certSize.data.ppdwValue      = &pdwCertSize;
            certSize.dataLength          = sizeof(pCertEntry->dwCertSize);

            validFrom.data.ppwszValue    = &pCertEntry->pwszTimeValidFrom;
            validFrom.dataLength         = 0;

            validUntil.data.ppwszValue   = &pCertEntry->pwszTimeValidTo;
            validUntil.dataLength        = 0;

            certBlob.data.ppValue        = &pCertEntry->pCertBlob;
            certBlob.dataLength          = 0;

            pdwRevoked                   = &pCertEntry->dwRevoked;
            revoked.data.ppdwValue       = &pdwRevoked;
            revoked.dataLength           = sizeof(pCertEntry->dwRevoked);

            dwError = VmcaDbFillValues(
                            pDbContext->pQueryCertificatesPaged,
                            &columns[0],
                            sizeof(columns)/sizeof(columns[0]));
            BAIL_ON_VMCA_ERROR(dwError);
            pCertEntry->dwCertSize = (DWORD)certBlob.dataLength;
            pCertEntry->pCertBlob[certBlob.dataLength] = '\0';
        }
        else if (dwError == ERROR_SUCCESS)
        {
            break;
        }
        BAIL_ON_VMCA_ERROR(dwError);

    } while (dwError != SQLITE_DONE);

    *ppCertEntryArray = pCertEntryArray;
    *pdwCount = iEntry;

cleanup:

    if (pDbContext->pQueryCertificatesPaged)
    {
        sqlite3_reset(pDbContext->pQueryCertificatesPaged);
    }

    return dwError;

error:

    *ppCertEntryArray = NULL;
    *pdwCount = 0;

    if (pCertEntryArray)
    {
        VmcaDbFreeCertEntryArray(pCertEntryArray, dwTotalEntries);
    }

    goto cleanup;
}


DWORD
VmcaDbDeleteCertificate(
    PVMCA_DB_CONTEXT pDbContext,
    PCWSTR           pwszSerial
    )
{
    DWORD dwError = 0;
    BOOLEAN bInTx = FALSE;

    if (!pDbContext->pDelCertQuery)
    {
        CHAR szQuery[] = "DELETE FROM CertTable WHERE Serial = ?1;";

        dwError = sqlite3_prepare_v2(
                        pDbContext->pDb,
                        szQuery,
                        -1,
                        &pDbContext->pDelCertQuery,
                        NULL);
        BAIL_ON_VMCA_ERROR(dwError);
    }

    if (!pDbContext->bInTx)
    {
        dwError = VmcaDbBeginTransaction(pDbContext->pDb);
        BAIL_ON_VMCA_ERROR(dwError);

        bInTx = TRUE;
    }

    dwError = sqlite3_bind_text16(
                    pDbContext->pDelCertQuery,
                    1,
                    pwszSerial,
                    -1,
                    SQLITE_TRANSIENT);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VmcaDbStepSql(pDbContext->pDelCertQuery);
    BAIL_ON_VMCA_ERROR(dwError);

    if (!pDbContext->bInTx && bInTx)
    {
        dwError = VmcaDbCommitTransaction(pDbContext->pDb);
        BAIL_ON_VMCA_ERROR(dwError);
    }

cleanup:

    if (pDbContext->pDelCertQuery)
    {
        sqlite3_reset(pDbContext->pDelCertQuery);
    }

    return dwError;

error:

    if (!pDbContext->bInTx && bInTx)
    {
        VmcaDbRollbackTransaction(pDbContext->pDb);
    }

    goto cleanup;
}



DWORD
VmcaDbRevokeCertificate(
    PVMCA_DB_CONTEXT pDbContext,
    PCWSTR           pwszSerial
    )
{
    DWORD dwError = 0;
    BOOLEAN bInTx = FALSE;

    if (!pDbContext->pUpdateCertQuery)
    {
        CHAR szQuery[] = "UPDATE CertTable SET REVOKED = 1 WHERE Serial = ?1;";

        dwError = sqlite3_prepare_v2(
                        pDbContext->pDb,
                        szQuery,
                        -1,
                        &pDbContext->pUpdateCertQuery,
                        NULL);
        BAIL_ON_VMCA_ERROR(dwError);
    }

    if (!pDbContext->bInTx)
    {
        dwError = VmcaDbBeginTransaction(pDbContext->pDb);
        BAIL_ON_VMCA_ERROR(dwError);

        bInTx = TRUE;
    }

    dwError = sqlite3_bind_text16(
                    pDbContext->pUpdateCertQuery,
                    1,
                    pwszSerial,
                    -1,
                    SQLITE_TRANSIENT);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VmcaDbStepSql(pDbContext->pUpdateCertQuery);
    BAIL_ON_VMCA_ERROR(dwError);

    if (!pDbContext->bInTx && bInTx)
    {
        dwError = VmcaDbCommitTransaction(pDbContext->pDb);
        BAIL_ON_VMCA_ERROR(dwError);
    }

cleanup:

    if (pDbContext &&
        pDbContext->pUpdateCertQuery)
    {
        sqlite3_reset(pDbContext->pUpdateCertQuery);
    }

    return dwError;

error:

    if (!pDbContext->bInTx && bInTx)
    {
        VmcaDbRollbackTransaction(pDbContext->pDb);
    }

    goto cleanup;
}



DWORD
VmcaDbVerifyCertificate(
    PVMCA_DB_CONTEXT pDbContext,
    PCWSTR           pwszSerial,
    DWORD            *pdwStatus
    )
{
    DWORD dwError = 0;
    DWORD dwRevoked = VMCA_CERTIFICATE_ACTIVE;

    if (!pDbContext->pVerifyCertQuery)
    {
        CHAR szQuery[] = "select RevokedReason from RevokedCertsTable WHERE Serial = ?1;";

        dwError = sqlite3_prepare_v2(
                        pDbContext->pDb,
                        szQuery,
                        -1,
                        &pDbContext->pVerifyCertQuery,
                        NULL);
        BAIL_ON_VMCA_ERROR(dwError);
    }

    dwError = sqlite3_bind_text16(
                    pDbContext->pVerifyCertQuery,
                    1,
                    pwszSerial,
                    -1,
                    SQLITE_TRANSIENT);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VmcaDbStepSql(pDbContext->pVerifyCertQuery);
    if (dwError == SQLITE_ROW)
    {
        assert(sqlite3_column_count(pDbContext->pVerifyCertQuery) == 1);
        sqlite3_column_int(pDbContext->pVerifyCertQuery, 0);
        dwRevoked = VMCA_CERTIFICATE_REVOKED;
        dwError = ERROR_SUCCESS;
    }
    BAIL_ON_VMCA_ERROR(dwError);
    if (pdwStatus != NULL)
    {
        *pdwStatus = dwRevoked;
    }

cleanup:

    if (pDbContext->pVerifyCertQuery)
    {
        sqlite3_reset(pDbContext->pVerifyCertQuery);
    }

    return dwError;

error:
    goto cleanup;
}

DWORD
VmcaDbRevokeCert (
    PCWSTR pwszSerial,
    PCWSTR pwszIssuerName,
    VMCA_CRL_REASON certRevokeReason
    )
{
    DWORD dwError = 0;
    PVMCA_DB_CONTEXT pDbContext = NULL;
    DWORD dwCertId = 0;

    if (IsNullOrEmptyString(pwszSerial))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR (dwError);
    }

    dwError = VmcaDbCreateContext (&pDbContext);
    BAIL_ON_VMCA_ERROR (dwError);

    dwError = VmcaDbGetCertID (
                              pDbContext,
                              pwszSerial,
                              pwszIssuerName,
                              &dwCertId
                              );
    BAIL_ON_VMCA_ERROR (dwError);

    if (!pDbContext->pRevokeCert)
    {
        CHAR szQuery[] = "INSERT INTO RevokedCertsTable ("
                         " CertID,"
                         " RevokedReason,"
                         " Serial)"
                         " VALUES( ?1,?2,?3);";

        dwError = sqlite3_prepare_v2(
                        pDbContext->pDb,
                        szQuery,
                        -1,
                        &pDbContext->pRevokeCert,
                        NULL);
        BAIL_ON_VMCA_ERROR(dwError);
    }

    if (!pDbContext->bInTx)
    {
        dwError = VmcaDbBeginTransaction(pDbContext->pDb);
        BAIL_ON_VMCA_ERROR(dwError);

        pDbContext->bInTx = TRUE;
    }

    dwError = sqlite3_bind_int(
                    pDbContext->pRevokeCert,
                    1,
                    dwCertId
                    );
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = sqlite3_bind_int(
                    pDbContext->pRevokeCert,
                    2,
                    certRevokeReason
                    );
    BAIL_ON_VMCA_ERROR (dwError);

    dwError = sqlite3_bind_text16(
                    pDbContext->pRevokeCert,
                    3,
                    pwszSerial,
                    -1,
                    SQLITE_TRANSIENT);
     BAIL_ON_VMCA_ERROR(dwError);

    dwError = VmcaDbStepSql(pDbContext->pRevokeCert);
    BAIL_ON_VMCA_ERROR(dwError);

    if (pDbContext->bInTx)
    {
        dwError = VmcaDbCommitTransaction(pDbContext->pDb);
        BAIL_ON_VMCA_ERROR(dwError);

        pDbContext->bInTx = FALSE;
    }

cleanup:
    if (pDbContext && pDbContext->pRevokeCert)
    {
        sqlite3_reset(pDbContext->pRevokeCert);
    }

    if (pDbContext)
    {
        VmcaDbReleaseContext(pDbContext);
    }

    return dwError;

error:
    if (pDbContext->bInTx)
    {
        VmcaDbRollbackTransaction(pDbContext->pDb);
        pDbContext->bInTx = FALSE;
    }

    if (dwError == SQLITE_NOTFOUND)
    {
        dwError = ERROR_OBJECT_NOT_FOUND;
    }

    if (dwError == SQLITE_CONSTRAINT)
    {
        dwError = ERROR_ALREADY_EXISTS;
    }

    goto cleanup;
}

DWORD
VmcaDbGetRevokedCerts(
    PVMCA_DB_REVOKED_CERTS *ppRevokedEntryArray,
    PDWORD pdwCertsReturned
    )
{
    DWORD dwError = 0;
    DWORD dwDbStatus = 0;
    PVMCA_DB_CONTEXT pDbContext = NULL;
    sqlite3_stmt* pDbQuery = NULL;
    DWORD dwCertsReturned = 0;
    DWORD dwCertIndex = 0;
    PVMCA_DB_REVOKED_CERTS pRevokedEntryArray = NULL;

    dwError = VmcaDbCreateContext(&pDbContext);
    BAIL_ON_VMCA_ERROR (dwError);

    dwError = VmcaDbGetRevokedCertsCount(
                        pDbContext,
                        &dwCertsReturned
                        );
    BAIL_ON_VMCA_ERROR (dwError);

    if (dwCertsReturned)
    {

        if(!pDbContext->pQueryRevokedCertificates)
        {
            CHAR szQuery[] = "SELECT "
                        " Serial,"
                        " RevokedDate,"
                        " RevokedReason"
                        " FROM RevokedCertsTable;";

            dwError = sqlite3_prepare_v2(
                        pDbContext->pDb,
                        szQuery,
                        -1,
                        &pDbContext->pQueryRevokedCertificates,
                        NULL);
            BAIL_ON_VMCA_ERROR(dwError);
        }

        pDbQuery = pDbContext->pQueryRevokedCertificates;

        dwError = VMCAAllocateMemory(
                        sizeof(VMCA_DB_REVOKED_CERTS)*dwCertsReturned,
                        (PVOID *)&pRevokedEntryArray
                        );
        BAIL_ON_VMCA_ERROR(dwError);

        for(dwCertIndex = 0; dwCertIndex < dwCertsReturned; ++dwCertIndex)
        {
            dwDbStatus = VmcaDbStepSql(pDbQuery);

            if (dwDbStatus == SQLITE_ROW)
            {

                dwError = VmcaDbGetWStringAtIndex(
                                pDbQuery,
                                0,
                                &pRevokedEntryArray[dwCertIndex].pwszSerial
                                );
                BAIL_ON_VMCA_ERROR (dwError);

                pRevokedEntryArray[dwCertIndex].dwRevokedDate =
                                sqlite3_column_int(
                                        pDbQuery,
                                        1
                                        );

                pRevokedEntryArray[dwCertIndex].dwRevokedReason =
                                sqlite3_column_int(
                                        pDbQuery,
                                        2
                                        );

            }
            else if (dwDbStatus != SQLITE_DONE)
            {
                dwError = dwDbStatus;
                BAIL_ON_VMCA_ERROR (dwError);
            }
        }
    }

    *pdwCertsReturned = dwCertIndex;
    *ppRevokedEntryArray = pRevokedEntryArray;

cleanup:

    if (pDbQuery)
    {
        sqlite3_reset(pDbQuery);
    }
    if (pDbContext)
    {
        VmcaDbReleaseContext(pDbContext);
    }

    return dwError;

error:
    if (ppRevokedEntryArray)
    {
        *ppRevokedEntryArray = NULL;
    }
    if (pdwCertsReturned)
    {
        *pdwCertsReturned = 0;
    }

    if (pRevokedEntryArray)
    {
        VmcaDbFreeRevokedCerts(
            pRevokedEntryArray,
            dwCertsReturned
            );
    }

    goto cleanup;
}

VOID
VmcaDbFreeRevokedCerts(
    PVMCA_DB_REVOKED_CERTS pRevokedEntryArray,
    DWORD dwCertCount
    )
{
    DWORD dwIndex = 0;
    if (pRevokedEntryArray && dwCertCount)
    {
        for (;dwIndex<dwCertCount; dwIndex++)
        {
            VMCA_SAFE_FREE_STRINGW (pRevokedEntryArray[dwIndex].pwszSerial);
        }

        VMCA_SAFE_FREE_MEMORY (pRevokedEntryArray);
    }
}

DWORD
VmcaDbSetCurrentCRLNumber(
    DWORD dwCrlNumber
    )
{
    DWORD dwError = 0;
    PVMCA_DB_CONTEXT pDbContext = NULL;
    sqlite3_stmt* pDbQuery = NULL;
    WCHAR crl_number_prop[] = CRL_NUMBER_PROPERTY_W;

    dwError = VmcaDbCreateContext(&pDbContext);
    BAIL_ON_VMCA_ERROR (dwError);

    if (!pDbContext->bInTx)
    {

        dwError = VmcaDbBeginTransaction(pDbContext->pDb);
        BAIL_ON_VMCA_ERROR (dwError);

        pDbContext->bInTx = TRUE;
    }

    if(!pDbContext->pSetCrlNumber)
    {
        CHAR szQuery[] = "INSERT INTO PropertiesTable "
                         "(Property,"
                         " Value)"
                         "VALUES ( ?2, ?1);";

        dwError = sqlite3_prepare_v2(
                        pDbContext->pDb,
                        szQuery,
                        -1,
                        &pDbContext->pSetCrlNumber,
                        NULL
                        );
        BAIL_ON_VMCA_ERROR(dwError);

    }

    pDbQuery = pDbContext->pSetCrlNumber;

    dwError = sqlite3_bind_blob(
                        pDbContext->pSetCrlNumber,
                        1,
                        (PBYTE)&dwCrlNumber,
                        sizeof (DWORD),
                        SQLITE_TRANSIENT
                        );
    BAIL_ON_VMCA_ERROR (dwError);

    dwError = sqlite3_bind_text16(
                        pDbContext->pSetCrlNumber,
                        2,
                        crl_number_prop,
                        -1,
                        SQLITE_TRANSIENT);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VmcaDbStepSql(pDbQuery);

    BAIL_ON_VMCA_ERROR (dwError);

    if (pDbContext->bInTx)
    {
        dwError = VmcaDbCommitTransaction(pDbContext->pDb);
        BAIL_ON_VMCA_ERROR (dwError);

        pDbContext->bInTx = FALSE;
    }

cleanup:

    if (pDbQuery)
    {
        sqlite3_reset(pDbQuery);
    }
    if (pDbContext)
    {
        VmcaDbReleaseContext(pDbContext);
    }

    return dwError;

error:

    if (pDbContext->bInTx)
    {
        VmcaDbRollbackTransaction(pDbContext->pDb);
        pDbContext->bInTx = FALSE;
    }

    goto cleanup;
}


DWORD
VmcaDbGetCurrentCRLNumber(
    PDWORD pdwCrlNumber
    )
{
    DWORD dwError = 0;
    DWORD dwCrlNumber = 0;
    PVMCA_DB_CONTEXT pDbContext = NULL;
    sqlite3_stmt* pDbQuery = NULL;
    WCHAR crl_number_prop[] = CRL_NUMBER_PROPERTY_W;
    PBYTE pCrlNumberBlob = NULL;
    DWORD pCrlNumberBlobLen = 0;

    if (!pdwCrlNumber)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR (dwError);
    }

    dwError = VmcaDbCreateContext(&pDbContext);
    BAIL_ON_VMCA_ERROR (dwError);

    if(!pDbContext->pGetCrlNumber)
    {
          CHAR szQuery[] = "SELECT "
                                 " Value"
                                 " FROM PropertiesTable"
                                 " WHERE Property = ?1;";

            dwError = sqlite3_prepare_v2(
                pDbContext->pDb,
                szQuery,
                -1,
                &pDbContext->pGetCrlNumber,
                NULL);
            BAIL_ON_VMCA_ERROR(dwError);
      }

      pDbQuery = pDbContext->pGetCrlNumber;

      dwError = sqlite3_bind_text16(
                    pDbContext->pGetCrlNumber,
                    1,
                    crl_number_prop,
                    -1,
                    SQLITE_TRANSIENT);
      BAIL_ON_VMCA_ERROR(dwError);

      dwError = VmcaDbStepSql(pDbQuery);

      if (dwError == SQLITE_ROW)
      {
          dwError = 0;

          dwError = VmcaDbGetColumnBlobAtIndex(
                                  pDbQuery,
                                  0,
                                  &pCrlNumberBlob,
                                  &pCrlNumberBlobLen
                                  );
          BAIL_ON_VMCA_ERROR (dwError);

          if (pCrlNumberBlobLen != sizeof (DWORD))
          {
              dwError = ERROR_INVALID_PARAMETER;
              BAIL_ON_VMCA_ERROR (dwError);
          }

          dwCrlNumber = *((PDWORD)pCrlNumberBlob);
      }
      else if (!dwError)
      {
          dwError = ERROR_OBJECT_NOT_FOUND;
      }

      BAIL_ON_VMCA_ERROR (dwError);

      *pdwCrlNumber = dwCrlNumber;

cleanup:

    if (pDbQuery)
    {
       sqlite3_reset(pDbQuery);
    }
    if (pDbContext)
    {
        VmcaDbReleaseContext(pDbContext);
    }

    return dwError;

error:
    if (pdwCrlNumber)
    {
        *pdwCrlNumber = 0;
    }

    goto cleanup;
}


static
DWORD
VmcaDbGetCertID (
    PVMCA_DB_CONTEXT pDbContext,
    PCWSTR pwszSerial,
    PCWSTR pwszIssuerName,
    PDWORD pdwCertId
    )
{
    DWORD dwError = 0;
    DWORD dwCertId = 0;

    if (!pDbContext ||
        IsNullOrEmptyString (pwszSerial) ||
        !pdwCertId
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR (dwError);
    }

    if (!pDbContext->pGetCertID)
    {
        CHAR szQuery[] = "SELECT ID "
                         "FROM CertTable "
                         "WHERE Serial = ?1 "
                         "AND IssuerName = ?2;";

        dwError = sqlite3_prepare_v2(
                        pDbContext->pDb,
                        szQuery,
                        -1,
                        &pDbContext->pGetCertID,
                        NULL
                        );
        BAIL_ON_VMCA_ERROR(dwError);
    }

    dwError = sqlite3_bind_text16(
                    pDbContext->pGetCertID,
                    1,
                    pwszSerial,
                    -1,
                    SQLITE_TRANSIENT);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = sqlite3_bind_text16(
                    pDbContext->pGetCertID,
                    2,
                    pwszIssuerName,
                    -1,
                    SQLITE_TRANSIENT);
    BAIL_ON_VMCA_ERROR(dwError);


    dwError = VmcaDbStepSql(pDbContext->pGetCertID);
    if (dwError == SQLITE_ROW)
    {
        dwCertId = sqlite3_column_int(
                      pDbContext->pGetCertID,
                      0
                      );

        dwError = ERROR_SUCCESS;
    }
    BAIL_ON_VMCA_ERROR(dwError);

    *pdwCertId = dwCertId;

cleanup:

    if (pDbContext &&
        pDbContext->pGetCertID)
    {
        sqlite3_reset(pDbContext->pGetCertID);
    }

    return dwError;

error:
    if (pdwCertId)
    {
        *pdwCertId = 0;
    }

    if (dwError == SQLITE_NOTFOUND)
    {
        dwError = ERROR_OBJECT_NOT_FOUND;
    }

    goto cleanup;
}

static
DWORD
VmcaDbGetWStringAtIndex(
    sqlite3_stmt* pDbQuery,
    DWORD dwColumnIndex,
    PWSTR *ppszValue
    )
{
    DWORD dwError = 0;
    PWSTR psztmpValue = NULL;
    PWSTR pszValue = NULL;
    DWORD dwLengthOfString = 0;
    int nType = 0;

    nType = sqlite3_column_type(
                                pDbQuery,
                                dwColumnIndex
                               );
    if ( ( nType != SQLITE_TEXT) && (nType != SQLITE_NULL))
    {
        dwError = SQLITE_MISMATCH;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    psztmpValue = (PWSTR)sqlite3_column_text16(
                                                pDbQuery,
                                                dwColumnIndex
                                              );
    if (psztmpValue)
    {

        dwLengthOfString = sqlite3_column_bytes16(
                                                  pDbQuery,
                                                  dwColumnIndex
                                                 );

        dwError =  VMCAAllocateMemory(
                                      dwLengthOfString+sizeof(WCHAR),
                                      (PVOID *)&pszValue
                                     );
        BAIL_ON_VMCA_ERROR (dwError);

        pszValue = memcpy(
                          pszValue,
                          psztmpValue,
                          dwLengthOfString+sizeof (WCHAR)
                         );
    }

    *ppszValue = pszValue;

cleanup:
    return dwError;

error :
    VMCA_SAFE_FREE_STRINGW (pszValue);
    goto cleanup;
}


static
DWORD
VmcaDbGetColumnBlobAtIndex(
    sqlite3_stmt* pSqlStatement,
    DWORD dwColumnIndex,
    PBYTE* ppszValue,
    PDWORD pdwLen)
{
    DWORD dwError = 0;
    DWORD dwLen = 0;
    PSTR psztmpValue = NULL;
    PBYTE pszValue = NULL;
    int nType = 0;

    nType = sqlite3_column_type(
                                pSqlStatement,
                                dwColumnIndex
                                );
    if ( ( nType != SQLITE_BLOB) && (nType != SQLITE_NULL))
    {
        dwError = SQLITE_MISMATCH;
        BAIL_ON_VMCA_ERROR (dwError);
    }

    dwLen = sqlite3_column_bytes(
                                pSqlStatement,
                                dwColumnIndex
                                );
    psztmpValue = (PSTR) sqlite3_column_blob(
                                              pSqlStatement,
                                              dwColumnIndex
                                            );
    if ( psztmpValue != NULL) {

    // Allocate more to make it Zero Padded Memory Location
    dwError = VMCAAllocateMemory(
                                dwLen,
                                (PVOID*)&pszValue
                                );
    BAIL_ON_VMCA_ERROR (dwError);

    memcpy(pszValue, psztmpValue, dwLen);
    }

    *pdwLen = dwLen;
    *ppszValue = pszValue;

cleanup:
    return dwError;

error:
    VMCA_SAFE_FREE_MEMORY (pszValue);

    if (ppszValue)
    {
        *ppszValue = NULL;
    }

    if (pdwLen)
    {
        *pdwLen = 0;
    }

    goto cleanup;
}

static
DWORD
VmcaDbGetRevokedCertsCount(
                           PVMCA_DB_CONTEXT pDbContext,
                           PDWORD pdwCount
                          )
{
    DWORD dwError = 0;
    DWORD dwNumRows = 0;

    if (!pDbContext->pRevokedCertCount)
    {
        CHAR szQuery[] = "SELECT COUNT(*)"
                         " FROM RevokedCertsTable;";

        dwError = sqlite3_prepare_v2(
                        pDbContext->pDb,
                        szQuery,
                        -1,
                        &pDbContext->pRevokedCertCount,
                        NULL);
        BAIL_ON_VMCA_ERROR(dwError);
    }

    dwError = VmcaDbStepSql(
                      pDbContext->pRevokedCertCount
                      );
    if (dwError == SQLITE_ROW)
    {
        dwNumRows = sqlite3_column_int(
                        pDbContext->pRevokedCertCount,
                        0
                        );

        dwError = ERROR_SUCCESS;
    }
    BAIL_ON_VMCA_ERROR(dwError);

    *pdwCount = dwNumRows;
cleanup:

    if (pDbContext &&
        pDbContext->pRevokedCertCount
       )
    {
        sqlite3_reset (pDbContext->pRevokedCertCount);
    }

    return dwError;

error:

    if (pdwCount)
    {
        *pdwCount = 0;
    }

    goto cleanup;
}
