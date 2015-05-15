/*
 * Copyright © 2012-2015 VMware, Inc.  All Rights Reserved.
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

#ifdef _WIN32
#pragma warning(disable : 4996 4995)
#endif

DWORD
VecsDbAddCertificate(
    PVECS_DB_CONTEXT           pDbContext,
    PVECS_DB_CERTIFICATE_ENTRY pCertEntry
    )
{
    DWORD dwError = 0;
    BOOLEAN bInTx = FALSE;
    sqlite3_stmt* pDbQuery = NULL;

    if (!pDbContext || !pCertEntry)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VECS_ERROR(dwError);
    }

    // TODO : Validate Cert Entry

    if (!pDbContext->pAddCertQuery)
    {
        CHAR szQuery[] =
                "INSERT INTO CertTable ("
                " Alias,"
                " Serial,"
                " CertSize,"
                " CertBlob,"
                " Password,"
                " StoreType,"
                " KeySize,"
                " PrivateKey)"
                " VALUES( :alias,"
                            " :serial,"
                            " :certsize,"
                            " :certblob,"
                            " :password,"
                            " :storetype,"
                            " :keysize,"
                            " :privatekey);";

        dwError = sqlite3_prepare_v2(
                        pDbContext->pDb,
                        szQuery,
                        -1,
                        &pDbContext->pAddCertQuery,
                        NULL);
        BAIL_ON_VECS_ERROR(dwError);
    }

    pDbQuery = pDbContext->pAddCertQuery;

    if (!pDbContext->bInTx)
    {
        dwError = VecsDbBeginTransaction(pDbContext->pDb);
        BAIL_ON_VECS_ERROR(dwError);

        bInTx = TRUE;
    }

    dwError = VecsBindWideString(pDbQuery,
                                ":alias",
                                pCertEntry->pwszAlias);
    BAIL_ON_VECS_ERROR(dwError);


    dwError = VecsBindWideString(pDbQuery,
                                 ":serial",
                                 pCertEntry->pwszSerial);
    BAIL_ON_VECS_ERROR(dwError);


    dwError = VecsBindDword(pDbQuery,
                            ":certsize",
                            pCertEntry->dwCertSize);
    BAIL_ON_VECS_ERROR(dwError);

    dwError = VecsBindBlob(pDbQuery,
                ":certblob",
                pCertEntry->pCertBlob,
                pCertEntry->dwCertSize);
    BAIL_ON_VECS_ERROR(dwError);


    dwError = VecsBindWideString(
                            pDbQuery,
                            ":password",
                            pCertEntry->pwszPassword);
    BAIL_ON_VECS_ERROR(dwError);

    dwError = VecsBindDword(pDbQuery,
                            ":storetype",
                            pCertEntry->dwStoreType);
    BAIL_ON_VECS_ERROR(dwError);

    dwError = VecsBindDword(pDbQuery,
                            ":keysize",
                            pCertEntry->dwKeySize);
    BAIL_ON_VECS_ERROR(dwError);

    dwError = VecsBindBlob(pDbQuery,
                ":privatekey",
                pCertEntry->pPrivateKey,
                pCertEntry->dwKeySize);
    BAIL_ON_VECS_ERROR(dwError);


    dwError = VecsDbStepSql(pDbQuery);
    BAIL_ON_VECS_ERROR(dwError);

    if (!pDbContext->bInTx && bInTx)
    {
        dwError = VecsDbCommitTransaction(pDbContext->pDb);
        BAIL_ON_VECS_ERROR(dwError);
    }

cleanup:

    if (pDbQuery)
    {
        sqlite3_reset(pDbQuery);
    }

    return dwError;

error:

    if (pDbContext && !pDbContext->bInTx && bInTx)
    {
        VecsDbRollbackTransaction(pDbContext->pDb);
    }

    goto cleanup;
}


DWORD
VecsDbCopyRow(
     sqlite3_stmt*     pSqlStatement,
    PVECS_DB_CERTIFICATE_ENTRY pEntry
    )
{
        DWORD dwError = 0;
        DWORD dwSize = 0;

        dwError =  VecsDBGetColumnInt(
                        pSqlStatement,
                        "id",
                        &pEntry->dwID);
        BAIL_ON_VECS_ERROR(dwError);

        dwError = VecsDBGetColumnString(
                        pSqlStatement,
                        "alias",
                        &pEntry->pwszAlias);
        BAIL_ON_VECS_ERROR(dwError);

        dwError = VecsDBGetColumnString(
                        pSqlStatement,
                        "serial",
                        &pEntry->pwszSerial);
        BAIL_ON_VECS_ERROR(dwError);

        dwError =  VecsDBGetColumnInt(
                        pSqlStatement,
                        "size",
                        &pEntry->dwCertSize);
        BAIL_ON_VECS_ERROR(dwError);

        dwError = VecsDBGetColumnBlob(
                    pSqlStatement,
                    "cert",
                    &pEntry->pCertBlob,
                    &dwSize);
        BAIL_ON_VECS_ERROR(dwError);

        dwError = VecsDBGetColumnString(
                        pSqlStatement,
                        "password",
                        &pEntry->pwszPassword);
        BAIL_ON_VECS_ERROR(dwError);

        dwError =  VecsDBGetColumnInt(
                        pSqlStatement,
                        "store",
                        &pEntry->dwStoreType);
        BAIL_ON_VECS_ERROR(dwError);

        dwError =  VecsDBGetColumnInt(
                        pSqlStatement,
                        "keysize",
                        &pEntry->dwKeySize);
        BAIL_ON_VECS_ERROR(dwError);

        dwError = VecsDBGetColumnBlob(
                    pSqlStatement,
                    "privatekey",
                    &pEntry->pPrivateKey,
                    &dwSize);
        BAIL_ON_VECS_ERROR(dwError);

error :
    return dwError;
}


DWORD
VecsDbQueryAllCertificates(
    PVECS_DB_CONTEXT            pDbContext,
    PVECS_DB_CERTIFICATE_ENTRY* ppCertEntryArray,
    PDWORD                      pdwCount
    )
{
    DWORD dwError = 0;
    DWORD dwDbStatus = 0;
    PVECS_DB_CERTIFICATE_ENTRY pCertEntryArray = NULL;
    DWORD iEntry = 0;
    DWORD dwEntriesAvailable = 0;
    PVECS_DB_CERTIFICATE_ENTRY pTemp = NULL;

    if (!pDbContext->pQueryAllCertificates)
    {
        CHAR szQuery[] = "SELECT "
                                 " ID as id,"
                                 " Alias as alias,"
                                 " Serial as serial,"
                                 " CertSize as size,"
                                 " CertBlob as cert,"
                                 " Password as password,"
                                 " StoreType  as store, "
                                 " KeySize as keysize,"
                                 " PrivateKey as privatekey"
                         " FROM CertTable;";

        dwError = sqlite3_prepare_v2(
                        pDbContext->pDb,
                        szQuery,
                        -1,
                        &pDbContext->pQueryAllCertificates,
                        NULL);
        BAIL_ON_VECS_ERROR(dwError);
    }

    dwError = VecsDbGetCertificateCount(pDbContext, 
                            NULL, &dwEntriesAvailable);    
    

    do
    {
        pTemp = pCertEntryArray + iEntry;
        dwDbStatus = VecsDbStepSql(pDbContext->pQueryAllCertificates);

        if (dwDbStatus == SQLITE_ROW)
        {
            dwError =  VecsDbCopyRow(pDbContext->pQueryAllCertificates,pTemp);
            BAIL_ON_VECS_ERROR(dwError);
            iEntry++;
        }


    } while (dwDbStatus != SQLITE_DONE);

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
        VecsDbFreeCertEntryArray(pCertEntryArray, iEntry);
    }

    goto cleanup;
}


DWORD
VecsDbQueryCertificatesPaged(
    PVECS_DB_CONTEXT         pDbContext,
    DWORD                    dwStartIndex,
    DWORD                    dwNumPackages,
    CERTIFICATE_STORE_TYPE   dwStoreType,
    PVECS_DB_CERTIFICATE_ENTRY*  ppCertEntryArray,
    PDWORD                   pdwCount
    )
{
    DWORD dwError = 0;
    PVECS_DB_CERTIFICATE_ENTRY pCertEntryArray = NULL;
    PVECS_DB_CERTIFICATE_ENTRY pTemp = NULL;
    DWORD iEntry = 0;
    DWORD dwDbStatus = 0;


    assert(dwNumPackages > 0);

    // in case of ALL, we have no status filter, otherwise appply it
    if (!pDbContext->pQueryCertificatesPaged)
    {
        if (dwStoreType == CERTIFICATE_STORE_TYPE_ALL)
        {
           CHAR szQuery[] = "SELECT "
                                 " ID as id,"
                                 " Alias as alias,"
                                 " Serial as serial,"
                                 " CertSize as size,"
                                 " CertBlob as cert,"
                                 " Password as password,"
                                 " StoreType  as store,"
                                 " KeySize as keysize,"
                                 " PrivateKey as privatekey"
                                 " FROM CertTable"
                                 " LIMIT :count OFFSET :skip;";

            dwError = sqlite3_prepare_v2(
                pDbContext->pDb,
                szQuery,
                -1,
                &pDbContext->pQueryCertificatesPaged,
                NULL);
            BAIL_ON_VECS_ERROR(dwError);

        }
        else
        {
            CHAR szQuery[] = "SELECT "
                                 " ID as id,"
                                 " Alias as alias,"
                                 " Serial as serial,"
                                 " CertSize as size,"
                                 " CertBlob as cert,"
                                 " Password as password,"
                                 " StoreType  as store,"
                                 " KeySize as keysize,"
                                 " PrivateKey as privatekey"
                                 " FROM CertTable"
                                 " WHERE StoreType=:store "
                                 " LIMIT :count OFFSET :skip;";
            dwError = sqlite3_prepare_v2(
                pDbContext->pDb,
                szQuery,
                -1,
                &pDbContext->pQueryCertificatesPaged,
                NULL);
            BAIL_ON_VECS_ERROR(dwError);
        }
    }

    dwError = VecsBindDword(pDbContext->pQueryCertificatesPaged ,
                            ":count",
                            dwNumPackages);
    BAIL_ON_VECS_ERROR(dwError);

    dwError = VecsBindDword(pDbContext->pQueryCertificatesPaged ,
                            ":skip",
                            dwStartIndex);
    BAIL_ON_VECS_ERROR(dwError);

    if (dwStoreType != CERTIFICATE_STORE_TYPE_ALL)
    {
        dwError = VecsBindDword(pDbContext->pQueryCertificatesPaged ,
                            ":store",
                            dwStoreType);
        BAIL_ON_VECS_ERROR(dwError);
    }

    dwError = VecsAllocateMemory(
                                sizeof(*pCertEntryArray) *
                                         (dwNumPackages + 1),
                                (PVOID*)&pCertEntryArray);
    BAIL_ON_VECS_ERROR(dwError);


     do
    {
        pTemp = &pCertEntryArray[iEntry];
        dwDbStatus = VecsDbStepSql(pDbContext->pQueryCertificatesPaged);

        if (dwDbStatus == SQLITE_ROW)
        {
            dwError =  VecsDbCopyRow(pDbContext->pQueryCertificatesPaged,pTemp);
            BAIL_ON_VECS_ERROR(dwError);
            iEntry++;
        }
    } while (dwDbStatus == SQLITE_ROW);

    *ppCertEntryArray = pCertEntryArray;
    *pdwCount = iEntry;

cleanup:

    if (pDbContext->pQueryCertificatesPaged)
    {
        sqlite3_finalize(pDbContext->pQueryCertificatesPaged);
        // set pDbContext->pQueryCertificatesPaged to NULL to make sure new query is constructed
        pDbContext->pQueryCertificatesPaged = NULL;
    }
    return dwError;

error:
    *ppCertEntryArray = NULL;
    *pdwCount = 0;

    if (pCertEntryArray)
    {
        VecsDbFreeCertEntryArray(pCertEntryArray, iEntry);
    }

    goto cleanup;
}


DWORD
VecsDbDeleteCertificate(
    PVECS_DB_CONTEXT pDbContext,
    PCWSTR           pwszAlias
    )
{
    DWORD dwError = 0;
    BOOLEAN bInTx = FALSE;

    if (!pDbContext->pDelCertQuery)
    {
        CHAR szQuery[] = "DELETE FROM CertTable WHERE Alias = :alias;";

        dwError = sqlite3_prepare_v2(
                        pDbContext->pDb,
                        szQuery,
                        -1,
                        &pDbContext->pDelCertQuery,
                        NULL);
        BAIL_ON_VECS_ERROR(dwError);
    }

    if (!pDbContext->bInTx)
    {
        dwError = VecsDbBeginTransaction(pDbContext->pDb);
        BAIL_ON_VECS_ERROR(dwError);

        bInTx = TRUE;
    }

    dwError = sqlite3_bind_text16(
                    pDbContext->pDelCertQuery,
                    1,
                    pwszAlias,
                    -1,
                    SQLITE_TRANSIENT);
    BAIL_ON_VECS_ERROR(dwError);

    dwError = VecsDbStepSql(pDbContext->pDelCertQuery);
    BAIL_ON_VECS_ERROR(dwError);

    if (!pDbContext->bInTx && bInTx)
    {
        dwError = VecsDbCommitTransaction(pDbContext->pDb);
        BAIL_ON_VECS_ERROR(dwError);
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
        VecsDbRollbackTransaction(pDbContext->pDb);
    }

    goto cleanup;
}


DWORD
VecsDbQueryCertificateByAlias(
    PVECS_DB_CONTEXT            pDbContext,
    PVECS_DB_CERTIFICATE_ENTRY* ppCertEntryArray,
    PDWORD                      pdwCount,
    PCWSTR                      pwszAlias
    )
{
    DWORD dwError = 0;
    DWORD dwDbStatus = 0;
    PVECS_DB_CERTIFICATE_ENTRY pCertEntryArray = NULL;
    DWORD iEntry = 0;
    DWORD dwEntriesAvailable = 0;
    PVECS_DB_CERTIFICATE_ENTRY pTemp = NULL;

    if (!pDbContext->pGetCertFromAliasQuery)
    {
        CHAR szQuery[] = "SELECT "
                                 " ID as id,"
                                 " Alias as alias,"
                                 " Serial as serial,"
                                 " CertSize as size,"
                                 " CertBlob as cert,"
                                 " Password as password,"
                                 " StoreType  as store, "
                                 " KeySize as keysize,"
                                 " PrivateKey as privatekey"
                         " FROM CertTable"
                         " WHERE Alias = :alias ;";

        dwError = sqlite3_prepare_v2(
                        pDbContext->pDb,
                        szQuery,
                        -1,
                        &pDbContext->pGetCertFromAliasQuery,
                        NULL);
        BAIL_ON_VECS_ERROR(dwError);
    }

    dwError = VecsDbGetCertificateCount(pDbContext, 
                        pwszAlias, &dwEntriesAvailable);

    BAIL_ON_VECS_ERROR(dwError);

    if( dwEntriesAvailable == 0) {
        *pdwCount = dwEntriesAvailable;
        goto cleanup;
    }

    dwError = VecsAllocateMemory(
                                sizeof(*pCertEntryArray) *
                                         (dwEntriesAvailable + 1),
                                (PVOID*)&pCertEntryArray);
    BAIL_ON_VECS_ERROR(dwError);
    dwError = sqlite3_bind_text16(
                    pDbContext->pGetCertFromAliasQuery,
                    1,
                    pwszAlias,
                    -1,
                    SQLITE_TRANSIENT);
    BAIL_ON_VECS_ERROR(dwError);

     do
    {
        pTemp = &pCertEntryArray[iEntry];
        dwDbStatus = VecsDbStepSql(pDbContext->pGetCertFromAliasQuery);

        if (dwDbStatus == SQLITE_ROW)
        {
            dwError =  VecsDbCopyRow(pDbContext->pGetCertFromAliasQuery,pTemp);
            BAIL_ON_VECS_ERROR(dwError);
            iEntry++;
        }
    } while (dwDbStatus == SQLITE_ROW);


    *ppCertEntryArray = pCertEntryArray;
    *pdwCount = iEntry;

cleanup:

    if (pDbContext->pGetCertFromAliasQuery)
    {
        sqlite3_reset(pDbContext->pGetCertFromAliasQuery);
    }

    return dwError;

error:

    *ppCertEntryArray = NULL;
    *pdwCount = 0;

    if (pCertEntryArray)
    {
        VecsDbFreeCertEntryArray(pCertEntryArray, iEntry);
    }

    goto cleanup;
}


DWORD
VecsDbGetCertificateCount(  PVECS_DB_CONTEXT pDbContext,
                            PCWSTR pwszAlias,
                            DWORD *pdwCount )
{
    #define MAX_QUERY 512
    PSTR pszQuery = "SELECT COUNT(*) AS TotalCertCount FROM CertTable";
    PSTR pszWhereCondition = " WHERE Alias = :alias ;";
    char pszFinalQuery[MAX_QUERY] = {0,};
    DWORD dwError = 0;
    DWORD dwEntriesAvailable = 0;
    DWORD dwDbStatus = 0;
    
    strncat(pszFinalQuery, pszQuery, MAX_QUERY);
    if (pwszAlias != NULL){
        strncat(pszFinalQuery, pszWhereCondition, MAX_QUERY);
    }
    if (!pDbContext->pGetCountQuery)
    {
         dwError = sqlite3_prepare_v2(
                        pDbContext->pDb,
                        pszFinalQuery,
                        -1,
                        &pDbContext->pGetCountQuery,
                        NULL);
        BAIL_ON_VECS_ERROR(dwError);
    }

     dwError = sqlite3_bind_text16(
                    pDbContext->pGetCountQuery,
                    1,
                    pwszAlias,
                    -1,
                    SQLITE_TRANSIENT);
    BAIL_ON_VECS_ERROR(dwError);

    dwDbStatus = VecsDbStepSql(pDbContext->pGetCountQuery);
    if (( dwDbStatus != SQLITE_DONE) &&
        ( dwDbStatus != SQLITE_ROW))
    {
        dwError = dwDbStatus;
        BAIL_ON_VECS_ERROR(dwError);
    }
    
    dwError  = VecsDBGetColumnInt(
                        pDbContext->pGetCountQuery,
                        "TotalCertCount",
                        &dwEntriesAvailable);
    BAIL_ON_VECS_ERROR(dwError);
    *pdwCount = dwEntriesAvailable;

cleanup:

    if (pDbContext->pGetCountQuery)
    {
        sqlite3_reset(pDbContext->pGetCountQuery);
    }

    return dwError;
error :
    goto cleanup;
}


