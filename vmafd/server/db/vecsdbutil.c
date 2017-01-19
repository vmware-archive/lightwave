/*
 * Copyright (C) 2014 VMware, Inc. All rights reserved.
 *
 * Module   : vecsdbutil.c
 * Author   : Aishu Raghavan (araghavan@vmware.com)
 *
 * Abstract :
 *
 */
#include "includes.h"
static
VOID
VecsSrvFreeCertContainer(
    PVMAFD_CERT_CONTAINER pContainer,
    DWORD dwArraySize
    );
static
DWORD
VecsDbGetStoreCount(
    PVECS_DB_CONTEXT pDbContext,
    PDWORD pdwStoreCount
    );

static
BOOL
VecsDbIsRestrictedStore (
    PCWSTR pszStoreName
    );


DWORD
VecsDbCreateCertStore(
    PCWSTR pszStoreName,
    PCWSTR pszPassword
    )
{
    DWORD dwError = 0;
    PVECS_DB_CONTEXT pDbContext = NULL;
    sqlite3_stmt* pDbQuery = NULL;
    sqlite3_stmt* pDbQuerySize = NULL;
    DWORD dwSizeOfTable = 0;

    if (IsNullOrEmptyString(pszStoreName))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VECS_ERROR(dwError);
    }

    dwError = VecsDbCreateContext(&pDbContext, VMAFD_DB_MODE_WRITE);
    BAIL_ON_VECS_ERROR(dwError);

    dwError = VecsDbGetStoreCount(pDbContext, &dwSizeOfTable);
    BAIL_ON_VECS_ERROR (dwError);

    if (dwSizeOfTable > STORE_TABLE_LIMIT)
    {
       dwError = ERROR_IMPLEMENTATION_LIMIT;
       BAIL_ON_VECS_ERROR (dwError);
    }

    if(!pDbContext->pCreateStoreTable)
    {
        CHAR szQuery[] =
                        "INSERT INTO StoreTable ("
                        " StoreName,"
                        " Password,"
                        " Salt)"
                        " VALUES( :storeName,"
                                    " :password,"
                                    " :salt);";
        dwError = sqlite3_prepare_v2(
                            pDbContext->pDb,
                            szQuery,
                            -1,
                            &pDbContext->pCreateStoreTable,
                            NULL);
        BAIL_ON_VECS_ERROR(dwError);
    }

    pDbQuery = pDbContext->pCreateStoreTable;

    if (!pDbContext->bInTx)
    {
        dwError = VecsDbBeginTransaction(pDbContext->pDb);
        BAIL_ON_VECS_ERROR(dwError);

        pDbContext->bInTx = TRUE;
    }

    dwError = VecsBindWideString(
                    pDbQuery,
                    ":storeName",
                    (PWSTR)pszStoreName
                    );
    BAIL_ON_VECS_ERROR(dwError);


    dwError = VecsBindWideString(
                    pDbQuery,
                    ":password",
                    (PWSTR)pszPassword
                    );
    BAIL_ON_VECS_ERROR(dwError);


    dwError = VecsBindWideString(
                    pDbQuery,
                    ":salt",
                    (PWSTR)NULL
                    );
    BAIL_ON_VECS_ERROR(dwError);

    dwError = VecsDbStepSql(pDbQuery);
    BAIL_ON_VECS_ERROR(dwError);

    dwError = VecsDbCommitTransaction(pDbContext->pDb);
    BAIL_ON_VECS_ERROR(dwError);

    pDbContext->bInTx = FALSE;

cleanup:

    if (pDbQuery)
    {
        sqlite3_reset(pDbQuery);
    }
    if (pDbQuerySize)
    {
        sqlite3_reset(pDbQuerySize);
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
    if (dwError == SQLITE_CONSTRAINT)
    {
        dwError = ERROR_ALREADY_EXISTS;
    }


    goto cleanup;
}

DWORD
VecsDbGetCertStore(
        PCWSTR pszStoreName,
        PCWSTR pszPassWord,
        PDWORD pdwStoreId
        )
{
   DWORD dwError = 0;
   PVECS_DB_CONTEXT pDbContext = NULL;
   sqlite3_stmt* pDbQuery = NULL;
   DWORD dwStoreId = 0;

   if (IsNullOrEmptyString(pszStoreName))
   {
       dwError = ERROR_INVALID_PARAMETER;
       BAIL_ON_VECS_ERROR(dwError);
   }

   dwError = VecsDbCreateContext(&pDbContext, VMAFD_DB_MODE_READ);
   BAIL_ON_VECS_ERROR(dwError);

   if(!pDbContext->pQueryStoreTable)
   {
       CHAR szQuery[] =
                       "SELECT  StoreID "
                       "FROM StoreTable "
                       "WHERE StoreName = :storeName";

       dwError = sqlite3_prepare_v2(
                           pDbContext->pDb,
                           szQuery,
                           -1,
                           &pDbContext->pQueryStoreTable,
                           NULL);
       BAIL_ON_VECS_ERROR(dwError);
   }

   pDbQuery = pDbContext->pQueryStoreTable;

   dwError = VecsBindWideString(
                   pDbQuery,
                   ":storeName",
                   (PWSTR)pszStoreName
                   );
   BAIL_ON_VECS_ERROR(dwError);

   dwError = VecsDbStepSql(pDbQuery);
   if (dwError == SQLITE_ROW)
   {
              dwError =  VecsDBGetColumnInt(
                          pDbQuery,
                          "StoreID",
                          &dwStoreId);
              BAIL_ON_VECS_ERROR(dwError);
   }
   else if (dwError == SQLITE_DONE)
   {
       dwError = ERROR_OBJECT_NOT_FOUND;
   }
   BAIL_ON_VECS_ERROR(dwError);

   if (dwStoreId == 0)
   {
       dwError = ERROR_OBJECT_NOT_FOUND;
       BAIL_ON_VECS_ERROR (dwError);
   }

   *pdwStoreId = dwStoreId;

cleanup:

   if (pDbQuery)
   {
       sqlite3_reset(pDbQuery);
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
VecsDbGetCertStoreName(
        DWORD   dwStoreId,
        PWSTR*  ppwszStoreName
        )
{
   DWORD dwError = 0;
   PVECS_DB_CONTEXT pDbContext = NULL;
   sqlite3_stmt* pDbQuery = NULL;
   PWSTR pwszStoreName = NULL;

   if (!ppwszStoreName)
   {
       dwError = ERROR_INVALID_PARAMETER;
       BAIL_ON_VECS_ERROR(dwError);
   }

   dwError = VecsDbCreateContext(&pDbContext, VMAFD_DB_MODE_READ);
   BAIL_ON_VECS_ERROR(dwError);

   if(!pDbContext->pQueryStoreTable)
   {
       CHAR szQuery[] =
                       "SELECT  StoreName "
                       "FROM StoreTable "
                       "WHERE StoreID = :storeID";

       dwError = sqlite3_prepare_v2(
                           pDbContext->pDb,
                           szQuery,
                           -1,
                           &pDbContext->pQueryStoreTable,
                           NULL);
       BAIL_ON_VECS_ERROR(dwError);
   }

   pDbQuery = pDbContext->pQueryStoreTable;

   dwError = VecsBindDword(
                   pDbQuery,
                   ":storeID",
                   (DWORD)dwStoreId
                   );
   BAIL_ON_VECS_ERROR(dwError);

   dwError = VecsDbStepSql(pDbQuery);
   if (dwError == SQLITE_ROW)
   {
              dwError =  VecsDBGetColumnString(
                          pDbQuery,
                          "StoreName",
                          &pwszStoreName);
              BAIL_ON_VECS_ERROR(dwError);
   }
   else if (dwError == SQLITE_DONE)
   {
       dwError = ERROR_OBJECT_NOT_FOUND;
   }
   BAIL_ON_VECS_ERROR(dwError);

   if (dwStoreId == 0)
   {
       dwError = ERROR_OBJECT_NOT_FOUND;
       BAIL_ON_VECS_ERROR (dwError);
   }

   *ppwszStoreName = pwszStoreName;

cleanup:

   if (pDbQuery)
   {
       sqlite3_reset(pDbQuery);
   }
   if (pDbContext)
   {
        VecsDbReleaseContext(pDbContext);
   }

   return dwError;

error:
   VMAFD_SAFE_FREE_MEMORY(pwszStoreName);
   if (ppwszStoreName)
   {
       *ppwszStoreName = NULL;
   }

   goto cleanup;
}


DWORD
VecsDbEnumCertStore(
    PWSTR **ppszStoreNameArray,
    PDWORD pdwCount
    )
{
    DWORD dwError = 0;
    DWORD dwDbStatus = 0;
    PVECS_DB_CONTEXT pDbContext = NULL;
    sqlite3_stmt* pDbQuery = NULL;
    DWORD dwStoresReturned = 0;
    PWSTR *pszStoreNameArray = NULL;
    DWORD dwStoreTableSize = 0;

    dwError = VecsDbCreateContext(&pDbContext, VMAFD_DB_MODE_READ);
    BAIL_ON_VECS_ERROR(dwError);

    dwError = VecsDbGetStoreCount (pDbContext, &dwStoreTableSize);
    BAIL_ON_VECS_ERROR (dwError);

    if (dwStoreTableSize)
    {
      if(!pDbContext->pQueryStoreTable)
      {
          CHAR szQuery[] = "SELECT "
                                 " StoreID,"
                                 " StoreName"
                                 " FROM StoreTable"
                                 " ORDER BY StoreID"
                                 " LIMIT :count";

            dwError = sqlite3_prepare_v2(
                pDbContext->pDb,
                szQuery,
                -1,
                &pDbContext->pQueryStoreTable,
                NULL);
            BAIL_ON_VECS_ERROR(dwError);
      }

      pDbQuery = pDbContext->pQueryStoreTable;

      if (!pDbContext->bInTx)
      {
        dwError = VecsDbBeginTransaction(pDbContext->pDb);
        BAIL_ON_VECS_ERROR(dwError);

        pDbContext->bInTx = TRUE;
      }

      dwError = VecsBindDword(
                   pDbQuery,
                   ":count",
                   dwStoreTableSize
                   );
      BAIL_ON_VECS_ERROR(dwError);

      dwError = VecsAllocateMemory(
                    sizeof(PWSTR)*dwStoreTableSize,
                    (PVOID *)&pszStoreNameArray
                    );
      BAIL_ON_VECS_ERROR(dwError);

      do
      {
          dwDbStatus = VecsDbStepSql(pDbQuery);

          if (dwDbStatus == SQLITE_ROW)
          {
              dwError = VecsDBGetColumnString(
                          pDbQuery,
                          "StoreName",
                          &(pszStoreNameArray[dwStoresReturned]));
              BAIL_ON_VECS_ERROR(dwError);

              dwStoresReturned++;
          }
          else if (dwDbStatus != SQLITE_DONE)
          {
            dwError = dwDbStatus;
            BAIL_ON_VECS_ERROR(dwError);
          }
      } while (dwDbStatus == SQLITE_ROW);
      dwError = VecsDbCommitTransaction(pDbContext->pDb);
      BAIL_ON_VECS_ERROR(dwError);
    }

    pDbContext->bInTx = FALSE;

    *pdwCount = dwStoresReturned;
    *ppszStoreNameArray = pszStoreNameArray;

cleanup:

    if (pDbQuery)
    {
       sqlite3_reset(pDbQuery);
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
    if (ppszStoreNameArray)
    {
        *ppszStoreNameArray = NULL;
    }
    if (pdwCount)
    {
        *pdwCount = 0;
    }
    if (pszStoreNameArray)
    {
        VmAfdFreeStringArrayW (pszStoreNameArray, dwStoresReturned);
    }

    goto cleanup;
}

DWORD
VecsDbDeleteCertStore(
    PCWSTR pwszStoreName
    )
{
    DWORD dwError = 0;
    PVECS_DB_CONTEXT pDbContext = NULL;
    sqlite3_stmt* pDbQuery = NULL;
    DWORD dwStoreId = 0;

    if (IsNullOrEmptyString(pwszStoreName))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VECS_ERROR(dwError);
    }

    if (VecsDbIsRestrictedStore (pwszStoreName))
    {
        dwError = ERROR_ACCESS_DENIED;
        BAIL_ON_VECS_ERROR (dwError);
    }

    dwError = VecsDbGetCertStore(pwszStoreName, NULL , &dwStoreId);
    BAIL_ON_VECS_ERROR(dwError);

    dwError = VecsDbCreateContext(&pDbContext, VMAFD_DB_MODE_WRITE);
    BAIL_ON_VECS_ERROR(dwError);

    if (!pDbContext->pDeleteStore)
    {
        CHAR szQuery[] = "DELETE FROM StoreTable WHERE StoreName = :storeName";

        dwError = sqlite3_prepare_v2(
                            pDbContext->pDb,
                            szQuery,
                            -1,
                            &pDbContext->pDeleteStore,
                            NULL);
        BAIL_ON_VECS_ERROR(dwError);
    }

    pDbQuery = pDbContext->pDeleteStore;

    if (!pDbContext->bInTx)
    {
        dwError = VecsDbBeginTransaction(pDbContext->pDb);
        BAIL_ON_VECS_ERROR(dwError);

        pDbContext->bInTx = TRUE;
    }

    dwError = VecsBindWideString(
                    pDbQuery,
                    ":storeName",
                    (PWSTR)pwszStoreName
                    );
    BAIL_ON_VECS_ERROR(dwError);

    dwError = VecsDbStepSql(pDbQuery);
    BAIL_ON_VECS_ERROR(dwError);

    dwError = VecsDbCommitTransaction(pDbContext->pDb);
    BAIL_ON_VECS_ERROR(dwError);

    pDbContext->bInTx = FALSE;

cleanup:

    if (pDbQuery)
    {
        sqlite3_reset(pDbQuery);
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

DWORD
VecsDbAddCert(
        DWORD dwStoreId,
        CERT_ENTRY_TYPE entryType,
        PWSTR pszAliasName,
        PWSTR pszCertificate,
        PWSTR pszPrivateKey,
        PWSTR pszPassWord,
        BOOLEAN bAutoRefresh
        )
{
   DWORD dwError = 0;
   PVECS_DB_CONTEXT pDbContext = NULL;
   sqlite3_stmt* pDbQuery = NULL;
   size_t nCertSize = 0;
   size_t nPrivateKeySize = 0;
   DWORD dwCertSize = 0;
   DWORD dwPrivateKeySize = 0;

   if (IsNullOrEmptyString(pszAliasName))
   {
       dwError = ERROR_INVALID_PARAMETER;
       BAIL_ON_VECS_ERROR(dwError);
   }

   if (pszCertificate)
   {
      dwError = VmAfdGetStringLengthW(
                    (PCWSTR)pszCertificate,
                    &nCertSize
                    );
      BAIL_ON_VECS_ERROR (dwError);
      dwCertSize = (nCertSize +1)* sizeof(WCHAR);
   }

   if (pszPrivateKey)
   {
      dwError = VmAfdGetStringLengthW(
                    (PCWSTR)pszPrivateKey,
                    &nPrivateKeySize
                    );
      BAIL_ON_VECS_ERROR (dwError);
      dwPrivateKeySize = (nPrivateKeySize+1)* sizeof(WCHAR);
   }

   dwError = VecsDbCreateContext(&pDbContext, VMAFD_DB_MODE_WRITE);
   BAIL_ON_VECS_ERROR(dwError);

   if(!pDbContext->pAddCertQuery)
   {
       CHAR szQuery[] =
                      "INSERT INTO CertTable ("
                      " Alias,"
                      " CertSize,"
                      " CertBlob,"
                      " Password,"
                      " EntryType,"
                      " KeySize,"
                      " PrivateKey,"
                      " StoreID,"
                      " AutoRefresh)"
                      " VALUES( :alias,"
                            " :certsize,"
                            " :certblob,"
                            " :password,"
                            " :entrytype,"
                            " :keysize,"
                            " :privatekey,"
                            " :storeid,"
                            " :autorefresh);";

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

       pDbContext->bInTx = TRUE;
   }

   dwError = VecsBindWideString(
                   pDbQuery,
                   ":alias",
                   pszAliasName
                   );
   BAIL_ON_VECS_ERROR(dwError);

   dwError = VecsBindBlob(
                  pDbQuery,
                  ":certblob",
                  (PBYTE)pszCertificate,
                  dwCertSize
                  );
   BAIL_ON_VECS_ERROR (dwError);

   dwError = VecsBindDword(
                  pDbQuery,
                  ":certsize",
                  dwCertSize
                  );
   BAIL_ON_VECS_ERROR (dwError);

   dwError = VecsBindWideString(
                  pDbQuery,
                  ":password",
                  pszPassWord
                  );
   BAIL_ON_VECS_ERROR (dwError);

   dwError = VecsBindDword(
                  pDbQuery,
                  ":entrytype",
                  entryType
                  );
   BAIL_ON_VECS_ERROR (dwError);

   dwError = VecsBindBlob(
                  pDbQuery,
                  ":privatekey",
                  (PBYTE)pszPrivateKey,
                  dwPrivateKeySize
                  );
   BAIL_ON_VECS_ERROR (dwError);

   dwError = VecsBindDword(
                  pDbQuery,
                  ":keysize",
                  dwPrivateKeySize
                  );
   BAIL_ON_VECS_ERROR (dwError);

   dwError = VecsBindDword(
                  pDbQuery,
                  ":storeid",
                  dwStoreId
                  );
   BAIL_ON_VECS_ERROR (dwError);

   dwError = VecsBindDword(
                  pDbQuery,
                  ":autorefresh",
                  bAutoRefresh
                  );
   BAIL_ON_VECS_ERROR (dwError);



   dwError = VecsDbStepSql(pDbQuery);
   BAIL_ON_VECS_ERROR (dwError);

   dwError = VecsDbCommitTransaction(pDbContext->pDb);
   BAIL_ON_VECS_ERROR(dwError);

   pDbContext->bInTx = FALSE;

cleanup:

   if (pDbQuery)
   {
       sqlite3_reset(pDbQuery);
   }
   if (pDbContext)
   {
      VecsDbReleaseContext (pDbContext);
   }

   return dwError;

error:

   if (pDbContext && pDbContext->bInTx)
   {
       VecsDbRollbackTransaction(pDbContext->pDb);
       pDbContext->bInTx = FALSE;
   }

   if (dwError == SQLITE_CONSTRAINT)
   {
      dwError = ERROR_ALREADY_EXISTS;
   }

   goto cleanup;
}

DWORD
VecsDbGetEntryByAliasInfoLevel1(
    DWORD dwStoreId,
    PWSTR pszAliasName,
    PVMAFD_CERT_ARRAY *ppCertContainer
    )
{
    DWORD dwError = 0;
    PVECS_DB_CONTEXT pDbContext = NULL;
    sqlite3_stmt* pDbQuery = NULL;
    PVMAFD_CERT_CONTAINER pEntry = NULL;
    PVMAFD_CERT_ARRAY pCertContainer = NULL;
    DWORD dwCount = 0;

    if (IsNullOrEmptyString(pszAliasName) ||
        !ppCertContainer)
    {
       dwError = ERROR_INVALID_PARAMETER;
       BAIL_ON_VECS_ERROR(dwError);
    }

    dwError = VecsDbCreateContext(&pDbContext, VMAFD_DB_MODE_READ);
    BAIL_ON_VECS_ERROR(dwError);

    if(!pDbContext->pGetCertFromAliasQuery)
    {
       CHAR szQuery[] =
                       "SELECT  *"
                       " FROM CertTable"
                       " WHERE StoreID = :storeid"
                       " AND Alias = :alias";

       dwError = sqlite3_prepare_v2(
                           pDbContext->pDb,
                           szQuery,
                           -1,
                           &pDbContext->pGetCertFromAliasQuery,
                           NULL);
       BAIL_ON_VECS_ERROR(dwError);
    }

    pDbQuery = pDbContext->pGetCertFromAliasQuery;

    dwError = VecsBindDword(
                   pDbQuery,
                   ":storeid",
                   dwStoreId
                   );
    BAIL_ON_VECS_ERROR(dwError);

    dwError = VecsBindWideString(
                   pDbQuery,
                   ":alias",
                   pszAliasName
                   );
    BAIL_ON_VECS_ERROR(dwError);


    dwError = VecsDbStepSql(pDbQuery);
    if (dwError == SQLITE_ROW)
    {
        dwError = VmAfdAllocateMemory(
                        sizeof (VMAFD_CERT_CONTAINER),
                        (PVOID *) &pEntry
                        );
        BAIL_ON_VECS_ERROR (dwError);

        dwError = VecsDBGetColumnInt(
                        pDbQuery,
                        "EntryType",
                        &(pEntry[dwCount].dwStoreType)
                        );
        BAIL_ON_VECS_ERROR (dwError);

        dwError = VecsDBGetColumnString(
                        pDbQuery,
                        "Alias",
                        &(pEntry[dwCount].pAlias)
                        );
        BAIL_ON_VECS_ERROR (dwError);

        dwError = VecsDBGetColumnInt (
                        pDbQuery,
                        "Date",
                        &(pEntry[dwCount].dwDate));
        BAIL_ON_VECS_ERROR (dwError);

        dwCount++;
    }
    else if (dwError == SQLITE_DONE)
    {
       dwError = ERROR_OBJECT_NOT_FOUND;
    }

    if (!dwCount)
    {
        dwError = ERROR_OBJECT_NOT_FOUND;
    }
    BAIL_ON_VECS_ERROR(dwError);

    dwError = VmAfdAllocateMemory (
                    sizeof (VMAFD_CERT_ARRAY),
                    (PVOID *) &pCertContainer
                    );
    BAIL_ON_VECS_ERROR (dwError);

    pCertContainer->dwCount = dwCount;
    pCertContainer->certificates = pEntry;

    *ppCertContainer = pCertContainer;
cleanup:

    if (pDbQuery)
    {
       sqlite3_reset(pDbQuery);
    }
    if (pDbContext)
    {
        VecsDbReleaseContext (pDbContext);
    }

    return dwError;

error:
    if (ppCertContainer)
    {
      *ppCertContainer = NULL;
    }
    if (pCertContainer)
    {
        VMAFD_SAFE_FREE_MEMORY(pCertContainer);
    }
    if (pEntry)
    {
        VecsSrvFreeCertContainer(pEntry, dwCount);
    }

    goto cleanup;
}

DWORD
VecsDbGetEntryByAliasInfoLevel2(
    DWORD dwStoreId,
    PWSTR pszAliasName,
    PVMAFD_CERT_ARRAY *ppCertContainer
    )
{
    DWORD dwError = 0;
    PVECS_DB_CONTEXT pDbContext = NULL;
    sqlite3_stmt* pDbQuery = NULL;
    PVMAFD_CERT_CONTAINER pEntry = NULL;
    PVMAFD_CERT_ARRAY pCertContainer = NULL;
    DWORD dwCount = 0;
    PBYTE pCertBlob = NULL;
    DWORD dwCertSize = 0;


    if (IsNullOrEmptyString(pszAliasName) ||
        !ppCertContainer)
    {
       dwError = ERROR_INVALID_PARAMETER;
       BAIL_ON_VECS_ERROR(dwError);
    }

    dwError = VecsDbCreateContext(&pDbContext, VMAFD_DB_MODE_READ);
    BAIL_ON_VECS_ERROR(dwError);

    if(!pDbContext->pGetCertFromAliasQuery)
    {
       CHAR szQuery[] =
                       "SELECT  *"
                       " FROM CertTable"
                       " WHERE StoreID = :storeid"
                       " AND Alias = :alias";

       dwError = sqlite3_prepare_v2(
                           pDbContext->pDb,
                           szQuery,
                           -1,
                           &pDbContext->pGetCertFromAliasQuery,
                           NULL);
       BAIL_ON_VECS_ERROR(dwError);
    }

    pDbQuery = pDbContext->pGetCertFromAliasQuery;

    dwError = VecsBindDword(
                   pDbQuery,
                   ":storeid",
                   dwStoreId
                   );
    BAIL_ON_VECS_ERROR(dwError);

    dwError = VecsBindWideString(
                   pDbQuery,
                   ":alias",
                   pszAliasName
                   );
    BAIL_ON_VECS_ERROR(dwError);


    dwError = VecsDbStepSql(pDbQuery);
    if (dwError == SQLITE_ROW)
    {
        dwError = VmAfdAllocateMemory(
                        sizeof (VMAFD_CERT_CONTAINER),
                        (PVOID *) &pEntry
                        );
        BAIL_ON_VECS_ERROR (dwError);

        dwError =  VecsDBGetColumnBlob(
                          pDbQuery,
                          "CertBlob",
                          &pCertBlob,
                          &dwCertSize);
        BAIL_ON_VECS_ERROR (dwError);

        if (pCertBlob)
        {

            dwError = VmAfdAllocateStringW(
                              (PWSTR)pCertBlob,
                              &(pEntry[dwCount].pCert)
                              );
            BAIL_ON_VECS_ERROR (dwError);
        }

        dwError = VecsDBGetColumnInt(
                        pDbQuery,
                        "EntryType",
                        &(pEntry[dwCount].dwStoreType)
                        );
        BAIL_ON_VECS_ERROR (dwError);

        dwError = VecsDBGetColumnString(
                        pDbQuery,
                        "Alias",
                        &(pEntry[dwCount].pAlias)
                        );
        BAIL_ON_VECS_ERROR (dwError);

        dwError = VecsDBGetColumnInt (
                    pDbQuery,
                    "Date",
                    &(pEntry[dwCount].dwDate));
        dwCount++;
    }
    else if (dwError == SQLITE_DONE)
    {
       dwError = ERROR_OBJECT_NOT_FOUND;
    }

    if (!dwCount)
    {
        dwError = ERROR_OBJECT_NOT_FOUND;
    }
    BAIL_ON_VECS_ERROR(dwError);

    dwError = VmAfdAllocateMemory (
                    sizeof (VMAFD_CERT_ARRAY),
                    (PVOID *) &pCertContainer
                    );
    BAIL_ON_VECS_ERROR (dwError);

    pCertContainer->dwCount = dwCount;
    pCertContainer->certificates = pEntry;

    *ppCertContainer = pCertContainer;
cleanup:

    if (pDbQuery)
    {
       sqlite3_reset(pDbQuery);
    }
    if (pDbContext)
    {
        VecsDbReleaseContext (pDbContext);
    }

    VMAFD_SAFE_FREE_MEMORY (pCertBlob);

    return dwError;
error:
    if (ppCertContainer)
    {
      *ppCertContainer = NULL;
    }
    if (pCertContainer)
    {
        VMAFD_SAFE_FREE_MEMORY(pCertContainer);
    }
    if (pEntry)
    {
        VecsSrvFreeCertContainer(pEntry, dwCount);
    }

    goto cleanup;
}

DWORD
VecsDbGetEntryTypeByAlias(
    DWORD dwStoreId,
    PWSTR pszAliasName,
    CERT_ENTRY_TYPE *pEntryType
    )
{
    DWORD dwError = 0;
    PVECS_DB_CONTEXT pDbContext = NULL;
    sqlite3_stmt* pDbQuery = NULL;
    CERT_ENTRY_TYPE entryType = CERT_ENTRY_TYPE_UNKNOWN;

    if (IsNullOrEmptyString(pszAliasName) ||
        !pEntryType
       )
    {
       dwError = ERROR_INVALID_PARAMETER;
       BAIL_ON_VECS_ERROR(dwError);
    }

    dwError = VecsDbCreateContext(&pDbContext, VMAFD_DB_MODE_READ);
    BAIL_ON_VECS_ERROR(dwError);

    if(!pDbContext->pQueryTypeByAlias)
    {
       CHAR szQuery[] =
                       "SELECT  EntryType"
                       " FROM CertTable"
                       " WHERE StoreID = :storeid"
                       " AND Alias = :alias";

       dwError = sqlite3_prepare_v2(
                           pDbContext->pDb,
                           szQuery,
                           -1,
                           &pDbContext->pQueryTypeByAlias,
                           NULL);
       BAIL_ON_VECS_ERROR(dwError);
    }

    pDbQuery = pDbContext->pQueryTypeByAlias;

    dwError = VecsBindDword(
                   pDbQuery,
                   ":storeid",
                   dwStoreId
                   );
    BAIL_ON_VECS_ERROR(dwError);

    dwError = VecsBindWideString(
                   pDbQuery,
                   ":alias",
                   pszAliasName
                   );
    BAIL_ON_VECS_ERROR(dwError);


    dwError = VecsDbStepSql(pDbQuery);
    if (dwError == SQLITE_ROW)
    {
              dwError =  VecsDBGetColumnInt(
                          pDbQuery,
                          "EntryType",
                          &entryType
                          );
              BAIL_ON_VECS_ERROR(dwError);

    }
    else if (dwError == SQLITE_DONE)
    {
       dwError = ERROR_OBJECT_NOT_FOUND;
    }
    BAIL_ON_VECS_ERROR(dwError);

    if (entryType == CERT_ENTRY_TYPE_UNKNOWN)
    {
       dwError = ERROR_OBJECT_NOT_FOUND;
       BAIL_ON_VECS_ERROR (dwError);
    }

    *pEntryType = entryType;

cleanup:
    if (pDbQuery)
    {
       sqlite3_reset(pDbQuery);
    }
    if (pDbContext)
    {
        VecsDbReleaseContext (pDbContext);
    }

    return dwError;

error:
    if (pEntryType)
    {
        *pEntryType = CERT_ENTRY_TYPE_UNKNOWN;
    }

    goto cleanup;
}

DWORD
VecsDbGetEntryDateByAlias(
    DWORD dwStoreId,
    PWSTR pszAliasName,
    PDWORD pdwDate
    )
{
    DWORD dwError = 0;
    PVECS_DB_CONTEXT pDbContext = NULL;
    sqlite3_stmt* pDbQuery = NULL;
    DWORD dwDate = 0;

    if (IsNullOrEmptyString(pszAliasName) ||
        !pdwDate
       )
    {
       dwError = ERROR_INVALID_PARAMETER;
       BAIL_ON_VECS_ERROR(dwError);
    }

    dwError = VecsDbCreateContext(&pDbContext, VMAFD_DB_MODE_READ);
    BAIL_ON_VECS_ERROR(dwError);

    if(!pDbContext->pQueryDateByAlias)
    {
       CHAR szQuery[] =
                       "SELECT  Date"
                       " FROM CertTable"
                       " WHERE StoreID = :storeid"
                       " AND Alias = :alias";

       dwError = sqlite3_prepare_v2(
                           pDbContext->pDb,
                           szQuery,
                           -1,
                           &pDbContext->pQueryDateByAlias,
                           NULL);
       BAIL_ON_VECS_ERROR(dwError);
    }

    pDbQuery = pDbContext->pQueryDateByAlias;

    dwError = VecsBindDword(
                   pDbQuery,
                   ":storeid",
                   dwStoreId
                   );
    BAIL_ON_VECS_ERROR(dwError);

    dwError = VecsBindWideString(
                   pDbQuery,
                   ":alias",
                   pszAliasName
                   );
    BAIL_ON_VECS_ERROR(dwError);


    dwError = VecsDbStepSql(pDbQuery);
    if (dwError == SQLITE_ROW)
    {
              dwError = VecsDBGetColumnInt(
                              pDbQuery,
                              "Date",
                              &dwDate
                              );
              BAIL_ON_VECS_ERROR (dwError);

    }
    else if (dwError == SQLITE_DONE)
    {
       dwError = ERROR_OBJECT_NOT_FOUND;
    }
    BAIL_ON_VECS_ERROR(dwError);

    if (dwDate == 0)
    {
       dwError = ERROR_OBJECT_NOT_FOUND;
       BAIL_ON_VECS_ERROR (dwError);
    }

    *pdwDate = dwDate;

cleanup:
    if (pDbQuery)
    {
       sqlite3_reset(pDbQuery);
    }
    if (pDbContext)
    {
        VecsDbReleaseContext (pDbContext);
    }

    return dwError;

error:
    if (pdwDate)
    {
        *pdwDate = 0;
    }

    goto cleanup;
}




DWORD
VecsDbGetCertificateByAlias(
    DWORD dwStoreId,
    PWSTR pszAliasName,
    PWSTR *ppszCertificate
    )
{
    DWORD dwError = 0;
    PVECS_DB_CONTEXT pDbContext = NULL;
    sqlite3_stmt* pDbQuery = NULL;
    PBYTE pCertBlob = NULL;
    DWORD dwCertSize = 0;

    if (IsNullOrEmptyString(pszAliasName) ||
        !ppszCertificate)
    {
       dwError = ERROR_INVALID_PARAMETER;
       BAIL_ON_VECS_ERROR(dwError);
    }

    dwError = VecsDbCreateContext(&pDbContext, VMAFD_DB_MODE_READ);
    BAIL_ON_VECS_ERROR(dwError);

    if(!pDbContext->pGetCertFromAliasQuery)
    {
       CHAR szQuery[] =
                       "SELECT  CertBlob"
                       " FROM CertTable"
                       " WHERE StoreID = :storeid"
                       " AND Alias = :alias";

       dwError = sqlite3_prepare_v2(
                           pDbContext->pDb,
                           szQuery,
                           -1,
                           &pDbContext->pGetCertFromAliasQuery,
                           NULL);
       BAIL_ON_VECS_ERROR(dwError);
    }

    pDbQuery = pDbContext->pGetCertFromAliasQuery;

    dwError = VecsBindDword(
                   pDbQuery,
                   ":storeid",
                   dwStoreId
                   );
    BAIL_ON_VECS_ERROR(dwError);

    dwError = VecsBindWideString(
                   pDbQuery,
                   ":alias",
                   pszAliasName
                   );
    BAIL_ON_VECS_ERROR(dwError);


    dwError = VecsDbStepSql(pDbQuery);
    if (dwError == SQLITE_ROW)
    {
              dwError =  VecsDBGetColumnBlob(
                          pDbQuery,
                          "CertBlob",
                          &pCertBlob,
                          &dwCertSize);
              BAIL_ON_VECS_ERROR(dwError);

              if (dwCertSize == 0)
              {
                dwError = ERROR_OBJECT_NOT_FOUND;
                BAIL_ON_VECS_ERROR (dwError);
              }

    }
    else if (dwError == SQLITE_DONE)
    {
       dwError = ERROR_OBJECT_NOT_FOUND;
    }
    BAIL_ON_VECS_ERROR(dwError);

    if (pCertBlob == NULL)
    {
       dwError = ERROR_OBJECT_NOT_FOUND;
       BAIL_ON_VECS_ERROR (dwError);
    }

    dwError = VmAfdAllocateStringW((PWSTR)pCertBlob, ppszCertificate);
    BAIL_ON_VECS_ERROR (dwError);

cleanup:

    if (pDbQuery)
    {
       sqlite3_reset(pDbQuery);
    }
    VMAFD_SAFE_FREE_MEMORY (pCertBlob);
    if (pDbContext)
    {
        VecsDbReleaseContext (pDbContext);
    }

    return dwError;

error:
    if (ppszCertificate)
    {
      *ppszCertificate = NULL;
    }

    goto cleanup;
}

DWORD
VecsDbGetPrivateKeyByAlias(
    DWORD dwStoreId,
    PWSTR pszAliasName,
    PWSTR pszPassword,
    PWSTR *ppszPrivateKey
    )
{
    DWORD dwError = 0;
    PVECS_DB_CONTEXT pDbContext = NULL;
    sqlite3_stmt* pDbQuery = NULL;
    PBYTE pPrivateKeyBlob = NULL;
    DWORD dwPrivateKeySize = 0;

    if (IsNullOrEmptyString(pszAliasName) ||
        !ppszPrivateKey)
    {
       dwError = ERROR_INVALID_PARAMETER;
       BAIL_ON_VECS_ERROR(dwError);
    }

    dwError = VecsDbCreateContext(&pDbContext, VMAFD_DB_MODE_READ);
    BAIL_ON_VECS_ERROR(dwError);

    if(!pDbContext->pGetPKeyFromAlias)
    {
       CHAR szQuery[] =
                       "SELECT  PrivateKey"
                       " FROM CertTable"
                       " WHERE StoreID = :storeid"
                       " AND Alias = :alias";

       dwError = sqlite3_prepare_v2(
                           pDbContext->pDb,
                           szQuery,
                           -1,
                           &pDbContext->pGetPKeyFromAlias,
                           NULL);
       BAIL_ON_VECS_ERROR(dwError);
    }

    pDbQuery = pDbContext->pGetPKeyFromAlias;

    dwError = VecsBindDword(
                   pDbQuery,
                   ":storeid",
                   dwStoreId
                   );
    BAIL_ON_VECS_ERROR(dwError);

    dwError = VecsBindWideString(
                   pDbQuery,
                   ":alias",
                   pszAliasName
                   );
    BAIL_ON_VECS_ERROR(dwError);


    dwError = VecsDbStepSql(pDbQuery);
    if (dwError == SQLITE_ROW)
    {
              dwError =  VecsDBGetColumnBlob(
                          pDbQuery,
                          "PrivateKey",
                          &pPrivateKeyBlob,
                          &dwPrivateKeySize
                          );
              BAIL_ON_VECS_ERROR(dwError);

              if (dwPrivateKeySize == 0)
              {
                  dwError = ERROR_OBJECT_NOT_FOUND;
                  BAIL_ON_VECS_ERROR (dwError);
              }

    }
    else if (dwError == SQLITE_DONE)
    {
       dwError = ERROR_OBJECT_NOT_FOUND;
    }
    BAIL_ON_VECS_ERROR(dwError);

    if (pPrivateKeyBlob == NULL)
    {
      dwError = ERROR_OBJECT_NOT_FOUND;
      BAIL_ON_VECS_ERROR (dwError);
    }


    dwError = VmAfdAllocateStringW((PWSTR)pPrivateKeyBlob, ppszPrivateKey);
    BAIL_ON_VECS_ERROR (dwError);

cleanup:

    if (pDbQuery)
    {
       sqlite3_reset(pDbQuery);
    }
    VMAFD_SAFE_FREE_MEMORY (pPrivateKeyBlob);
    if (pDbContext)
    {
        VecsDbReleaseContext(pDbContext);
    }

    return dwError;

error:
    if (ppszPrivateKey)
    {
      *ppszPrivateKey = NULL;
    }

    goto cleanup;
}

DWORD
VecsDbGetEntriesCount(
    DWORD dwStoreId,
    PDWORD pdwSize
    )
{
    DWORD dwError = 0;
    PVECS_DB_CONTEXT pDbContext = NULL;
    sqlite3_stmt* pDbQuery = NULL;
    DWORD dwCertsReturned = 0;

    dwError = VecsDbCreateContext(&pDbContext, VMAFD_DB_MODE_READ);
    BAIL_ON_VECS_ERROR(dwError);

    if(!pDbContext->pQueryCertificatesPaged)
    {
         CHAR szQuery[] = "SELECT COUNT(*)"
                                 " FROM CertTable"
                                 " WHERE StoreID = :storeid";

          dwError = sqlite3_prepare_v2(
                pDbContext->pDb,
                szQuery,
                -1,
                &pDbContext->pQueryCertificatesPaged,
                NULL);
          BAIL_ON_VECS_ERROR(dwError);
    }

    pDbQuery = pDbContext->pQueryCertificatesPaged;

    dwError = VecsBindDword(
                   pDbQuery,
                   ":storeid",
                   dwStoreId
                   );
    BAIL_ON_VECS_ERROR(dwError);

    dwError = VecsDbStepSql(pDbQuery);

    if (dwError == SQLITE_ROW)
    {
        dwCertsReturned = sqlite3_column_int(pDbQuery, 0);
        dwError = 0;
    }
    BAIL_ON_VECS_ERROR (dwError);

    *pdwSize = dwCertsReturned;
cleanup:

    if (pDbQuery)
    {
       sqlite3_reset(pDbQuery);
    }
    if (pDbContext)
    {
        VecsDbReleaseContext(pDbContext);
    }

    return dwError;

error:
    if (pdwSize)
    {
        *pdwSize = 0;
    }

    goto cleanup;
}

DWORD
VecsDbEnumInfoLevel1(
    DWORD dwStoreId,
    DWORD dwIndex,
    DWORD dwLimit,
    PVMAFD_CERT_ARRAY *ppCertContainer
    )
{
    DWORD dwError = 0;
    DWORD dwDbStatus = 0;
    PVECS_DB_CONTEXT pDbContext = NULL;
    sqlite3_stmt* pDbQuery = NULL;
    DWORD dwCertsReturned = 0;
    PVMAFD_CERT_ARRAY pCertContainer = NULL;
    PVMAFD_CERT_CONTAINER pContainer = NULL;

    dwError = VecsDbCreateContext(&pDbContext, VMAFD_DB_MODE_READ);
    BAIL_ON_VECS_ERROR(dwError);

    if(!pDbContext->pQueryInfoLevel1)
    {
         CHAR szQuery[] = "SELECT "
                                 " ID,"
                                 " Alias,"
                                 " EntryType,"
                                 " Date"
                                 " FROM CertTable"
                                 " WHERE StoreID = :storeid"
                                 " ORDER BY ID"
                                 " LIMIT :count OFFSET :skip";

          dwError = sqlite3_prepare_v2(
                pDbContext->pDb,
                szQuery,
                -1,
                &pDbContext->pQueryInfoLevel1,
                NULL);
          BAIL_ON_VECS_ERROR(dwError);
     }

    pDbQuery = pDbContext->pQueryInfoLevel1;

    if (!pDbContext->bInTx)
    {
       dwError = VecsDbBeginTransaction(pDbContext->pDb);
       BAIL_ON_VECS_ERROR(dwError);

       pDbContext->bInTx = TRUE;
    }

    dwError = VecsBindDword(
                   pDbQuery,
                   ":storeid",
                   dwStoreId
                   );
    BAIL_ON_VECS_ERROR(dwError);

    dwError = VecsBindDword(
                   pDbQuery,
                   ":count",
                   dwLimit
                   );
    BAIL_ON_VECS_ERROR(dwError);

    dwError = VecsBindDword(
                   pDbQuery,
                   ":skip",
                   dwIndex
                   );
    BAIL_ON_VECS_ERROR(dwError);

    dwError = VecsAllocateMemory(
                    sizeof(VMAFD_CERT_CONTAINER)*dwLimit,
                    (PVOID *)&pContainer
                    );
    BAIL_ON_VECS_ERROR(dwError);

    do
    {
        dwDbStatus = VecsDbStepSql(pDbQuery);

        if (dwDbStatus == SQLITE_ROW)
        {
              dwError = VecsDBGetColumnString(
                          pDbQuery,
                          "Alias",
                          &(pContainer[dwCertsReturned].pAlias));
              BAIL_ON_VECS_ERROR(dwError);

              dwError = VecsDBGetColumnInt(
                            pDbQuery,
                            "EntryType",
                            &(pContainer[dwCertsReturned].dwStoreType));
              BAIL_ON_VECS_ERROR (dwError);

              dwError = VecsDBGetColumnInt (
                            pDbQuery,
                            "Date",
                            &(pContainer[dwCertsReturned].dwDate));
              BAIL_ON_VECS_ERROR (dwError);

              dwCertsReturned++;
        }
        else if (dwDbStatus != SQLITE_DONE)
        {
            dwError = dwDbStatus;
            BAIL_ON_VECS_ERROR(dwError);
        }
    } while (dwDbStatus == SQLITE_ROW);

    dwError = VecsDbCommitTransaction(pDbContext->pDb);
    BAIL_ON_VECS_ERROR(dwError);

    pDbContext->bInTx = FALSE;

    dwError = VecsAllocateMemory(
                    sizeof(VMAFD_CERT_ARRAY),
                    (PVOID *) &pCertContainer);
    BAIL_ON_VECS_ERROR (dwError);

    pCertContainer->dwCount = dwCertsReturned;
    pCertContainer->certificates = pContainer;

    *ppCertContainer = pCertContainer;

cleanup:

    if (pDbQuery)
    {
       sqlite3_reset(pDbQuery);
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
    if (ppCertContainer)
    {
        *ppCertContainer = NULL;
    }
    if (pCertContainer)
    {
        VMAFD_SAFE_FREE_MEMORY(pCertContainer);
    }
    if (pContainer)
    {
        VecsSrvFreeCertContainer(pContainer,dwCertsReturned);
    }

    goto cleanup;
}


DWORD
VecsDbEnumInfoLevel2(
    DWORD dwStoreId,
    DWORD dwIndex,
    DWORD dwLimit,
    PVMAFD_CERT_ARRAY *ppCertContainer
    )
{
    DWORD dwError = 0;
    DWORD dwDbStatus = 0;
    PVECS_DB_CONTEXT pDbContext = NULL;
    sqlite3_stmt* pDbQuery = NULL;
    DWORD dwCertsReturned = 0;
    PVMAFD_CERT_ARRAY pCertContainer = NULL;
    PVMAFD_CERT_CONTAINER pContainer = NULL;
    PBYTE pCertBlob = NULL;
    DWORD dwCertSize = 0;

    dwError = VecsDbCreateContext(&pDbContext, VMAFD_DB_MODE_READ);
    BAIL_ON_VECS_ERROR(dwError);

    if(!pDbContext->pQueryInfoLevel2)
    {
         CHAR szQuery[] = "SELECT "
                                 " ID,"
                                 " Alias,"
                                 " CertBlob,"
                                 " EntryType,"
                                 " AutoRefresh,"
                                 " Date"
                                 " FROM CertTable"
                                 " WHERE StoreID = :storeid"
                                 " ORDER BY ID"
                                 " LIMIT :count OFFSET :skip";

          dwError = sqlite3_prepare_v2(
                pDbContext->pDb,
                szQuery,
                -1,
                &pDbContext->pQueryInfoLevel2,
                NULL);
          BAIL_ON_VECS_ERROR(dwError);
      }

    pDbQuery = pDbContext->pQueryInfoLevel2;

    if (!pDbContext->bInTx)
    {
       dwError = VecsDbBeginTransaction(pDbContext->pDb);
       BAIL_ON_VECS_ERROR(dwError);

       pDbContext->bInTx = TRUE;
    }

    dwError = VecsBindDword(
                   pDbQuery,
                   ":storeid",
                   dwStoreId
                   );
    BAIL_ON_VECS_ERROR(dwError);

    dwError = VecsBindDword(
                   pDbQuery,
                   ":count",
                   dwLimit
                   );
    BAIL_ON_VECS_ERROR(dwError);

    dwError = VecsBindDword(
                   pDbQuery,
                   ":skip",
                   dwIndex
                   );
    BAIL_ON_VECS_ERROR(dwError);

    dwError = VecsAllocateMemory(
                    sizeof(VMAFD_CERT_CONTAINER)*dwLimit,
                    (PVOID *)&pContainer
                    );
    BAIL_ON_VECS_ERROR(dwError);

    do
    {
        dwDbStatus = VecsDbStepSql(pDbQuery);

        if (dwDbStatus == SQLITE_ROW)
        {
              dwError =  VecsDBGetColumnBlob(
                          pDbQuery,
                          "CertBlob",
                          &pCertBlob,
                          &dwCertSize);
              BAIL_ON_VECS_ERROR (dwError);

              if (pCertBlob)
              {

                  dwError = VmAfdAllocateStringW(
                              (PWSTR)pCertBlob,
                              &(pContainer[dwCertsReturned].pCert)
                              );
                  BAIL_ON_VECS_ERROR (dwError);
              }

              VMAFD_SAFE_FREE_MEMORY (pCertBlob);

              dwError = VecsDBGetColumnString(
                          pDbQuery,
                          "Alias",
                          &(pContainer[dwCertsReturned].pAlias));
              BAIL_ON_VECS_ERROR(dwError);

              dwError = VecsDBGetColumnInt(
                            pDbQuery,
                            "EntryType",
                            &(pContainer[dwCertsReturned].dwStoreType));
              BAIL_ON_VECS_ERROR (dwError);

              dwError = VecsDBGetColumnInt(
                            pDbQuery,
                            "AutoRefresh",
                            &(pContainer[dwCertsReturned].dwAutoRefresh));
              BAIL_ON_VECS_ERROR (dwError);

              dwError = VecsDBGetColumnInt (
                            pDbQuery,
                            "Date",
                            &(pContainer[dwCertsReturned].dwDate));
              BAIL_ON_VECS_ERROR (dwError);

              dwCertsReturned++;
        }
        else if (dwDbStatus != SQLITE_DONE)
        {
            dwError = dwDbStatus;
            BAIL_ON_VECS_ERROR(dwError);
        }
    } while (dwDbStatus == SQLITE_ROW);

    dwError = VecsDbCommitTransaction(pDbContext->pDb);
    BAIL_ON_VECS_ERROR(dwError);

    pDbContext->bInTx = FALSE;

    dwError = VecsAllocateMemory(
                    sizeof(VMAFD_CERT_ARRAY),
                    (PVOID *) &pCertContainer);
    BAIL_ON_VECS_ERROR (dwError);

    pCertContainer->dwCount = dwCertsReturned;
    pCertContainer->certificates = pContainer;

    *ppCertContainer = pCertContainer;

cleanup:

    if (pDbQuery)
    {
       sqlite3_reset(pDbQuery);
    }
    if (pDbContext)
    {
        VecsDbReleaseContext (pDbContext);
    }

    return dwError;

error:

    if (pDbContext && pDbContext->bInTx)
    {
       VecsDbRollbackTransaction(pDbContext->pDb);
       pDbContext->bInTx = FALSE;
    }
    if (ppCertContainer)
    {
        *ppCertContainer = NULL;
    }
    if (pCertContainer)
    {
        VMAFD_SAFE_FREE_MEMORY(pCertContainer);
    }
    if (pContainer)
    {
        VecsSrvFreeCertContainer(pContainer,dwCertsReturned);
    }
    VMAFD_SAFE_FREE_MEMORY (pCertBlob);

    goto cleanup;
}

DWORD
VecsDbDeleteCert(
    DWORD dwStoreId,
    PWSTR pszAliasName
    )
{
    DWORD dwError = 0;
    PVECS_DB_CONTEXT pDbContext = NULL;
    sqlite3_stmt* pDbQuery = NULL;

    if (IsNullOrEmptyString(pszAliasName))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VECS_ERROR (dwError);
    }

    dwError = VecsDbCreateContext(&pDbContext, VMAFD_DB_MODE_WRITE);
    BAIL_ON_VECS_ERROR(dwError);

    if(!pDbContext->pDelCertQuery)
    {
         CHAR szQuery[] = "DELETE"
                                 " FROM"
                                 " CertTable"
                                 " WHERE StoreID = :storeid"
                                 " AND Alias = :alias;";

          dwError = sqlite3_prepare_v2(
                pDbContext->pDb,
                szQuery,
                -1,
                &pDbContext->pDelCertQuery,
                NULL);
          BAIL_ON_VECS_ERROR(dwError);
      }

    pDbQuery = pDbContext->pDelCertQuery;

    if (!pDbContext->bInTx)
    {
       dwError = VecsDbBeginTransaction(pDbContext->pDb);
       BAIL_ON_VECS_ERROR(dwError);

       pDbContext->bInTx = TRUE;
    }

    dwError = VecsBindDword(
                   pDbQuery,
                   ":storeid",
                   dwStoreId
                   );
    BAIL_ON_VECS_ERROR(dwError);

    dwError = VecsBindWideString(
                   pDbQuery,
                   ":alias",
                   pszAliasName
                   );
    BAIL_ON_VECS_ERROR(dwError);

    dwError = VecsDbStepSql(pDbQuery);
    BAIL_ON_VECS_ERROR (dwError);

    if (!sqlite3_changes(pDbContext->pDb))
    {
        dwError = ERROR_OBJECT_NOT_FOUND;
        BAIL_ON_VECS_ERROR(dwError);
    }

    dwError = VecsDbCommitTransaction(pDbContext->pDb);
    BAIL_ON_VECS_ERROR(dwError);

    pDbContext->bInTx = FALSE;

cleanup:

    if (pDbQuery)
    {
       sqlite3_reset(pDbQuery);
    }
    if (pDbContext)
    {
        VecsDbReleaseContext (pDbContext);
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

DWORD
VecsDbSetEntryDwordAttributeByAlias(
    DWORD dwStoreId,
    PWSTR pszAliasName,
    PSTR  pszAttributeName,
    DWORD dwValue
    )
{
    DWORD dwError = 0;
    PVECS_DB_CONTEXT pDbContext = NULL;
    sqlite3_stmt* pDbQuery = NULL;
    PSTR pszQuery = NULL;

    if (IsNullOrEmptyString(pszAliasName)
        || IsNullOrEmptyString(pszAttributeName))
    {
       dwError = ERROR_INVALID_PARAMETER;
       BAIL_ON_VECS_ERROR(dwError);
    }

    dwError = VecsDbCreateContext(&pDbContext, VMAFD_DB_MODE_WRITE);
    BAIL_ON_VECS_ERROR(dwError);

    if(!pDbContext->pUpdateCertAttrQuery)
    {
       CHAR szQuery[] =
                       "UPDATE CertTable"
                       " SET %s = :value"
                       " WHERE StoreID = :storeid"
                       " AND Alias = :alias";
       dwError = VmAfdAllocateStringPrintf(
           &pszQuery, szQuery, pszAttributeName);
       BAIL_ON_VECS_ERROR(dwError);

       dwError = sqlite3_prepare_v2(
                           pDbContext->pDb,
                           pszQuery,
                           -1,
                           &pDbContext->pUpdateCertAttrQuery,
                           NULL);
       BAIL_ON_VECS_ERROR(dwError);
    }

    pDbQuery = pDbContext->pUpdateCertAttrQuery;

    dwError = VecsBindDword(
                   pDbQuery,
                   ":storeid",
                   dwStoreId
                   );
    BAIL_ON_VECS_ERROR(dwError);

    dwError = VecsBindWideString(
                   pDbQuery,
                   ":alias",
                   pszAliasName
                   );
    BAIL_ON_VECS_ERROR(dwError);

    dwError = VecsBindDword(
                   pDbQuery,
                   ":value",
                   dwValue
                   );
    BAIL_ON_VECS_ERROR(dwError);

    dwError = VecsDbStepSql(pDbQuery);
    BAIL_ON_VECS_ERROR(dwError);

cleanup:
    VMAFD_SAFE_FREE_STRINGA(pszQuery);
    if (pDbQuery)
    {
       sqlite3_reset(pDbQuery);
    }
    if (pDbContext)
    {
        VecsDbReleaseContext (pDbContext);
    }

    return dwError;

error:
    goto cleanup;
}

static
VOID
VecsSrvFreeCertContainer(
    PVMAFD_CERT_CONTAINER pContainer,
    DWORD dwArraySize
    )
{
    if (pContainer)
    {
        DWORD dwIndex = 0;
        for (;dwIndex<dwArraySize; dwIndex++)
        {
            PVMAFD_CERT_CONTAINER pTemp = &pContainer[dwIndex];
            VMAFD_SAFE_FREE_MEMORY(pTemp->pCert);
            VMAFD_SAFE_FREE_MEMORY(pTemp->pAlias);
            VMAFD_SAFE_FREE_MEMORY(pTemp->pPassword);
            VMAFD_SAFE_FREE_MEMORY(pTemp->pPrivateKey);
        }
        VmAfdFreeMemory (pContainer);
    }
}

static
DWORD
VecsDbGetStoreCount(
    PVECS_DB_CONTEXT pDbContext,
    PDWORD pdwStoreCount
    )
{
    DWORD dwError = 0;
    sqlite3_stmt* pDbQuery = NULL;
    DWORD dwStoreCount = 0;

    char szQuery[] = "SELECT COUNT(*)"
                     " FROM StoreTable";

    if (!pDbContext || !pdwStoreCount)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VECS_ERROR (dwError);
    }

    dwError = sqlite3_prepare_v2(
                pDbContext->pDb,
                szQuery,
                -1,
                &pDbQuery,
                NULL);
    BAIL_ON_VECS_ERROR(dwError);

    dwError = VecsDbStepSql(pDbQuery);
    if (dwError == SQLITE_ROW)
    {
        dwError = 0;
        dwStoreCount = sqlite3_column_int(pDbQuery, 0);
    }
    BAIL_ON_VECS_ERROR (dwError);

    *pdwStoreCount = dwStoreCount;
cleanup:
    if (pDbQuery)
    {
      sqlite3_finalize (pDbQuery);
    }

    return dwError;
error:
    if (pdwStoreCount)
    {
        *pdwStoreCount = 0;
    }

    goto cleanup;
}

static
BOOL
VecsDbIsRestrictedStore (
    PCWSTR pszStoreName
    )
{
    BOOL bIsRestricted = FALSE;
    WCHAR wszSystemStoreName[] = SYSTEM_CERT_STORE_NAME_W;
    WCHAR wszTrustedRootsStoreName[] = TRUSTED_ROOTS_STORE_NAME_W;
    WCHAR wszCRLStoreName[] = CRL_STORE_NAME_W;


    if (VmAfdStringIsEqualW(
                  pszStoreName,
                  &wszSystemStoreName[0],
                  0
                  )
       )
    {
        bIsRestricted = TRUE;
    }

    else if (VmAfdStringIsEqualW (
                            pszStoreName,
                            &wszTrustedRootsStoreName[0],
                            0
                            )
              )
    {
        bIsRestricted = TRUE;
    }

    else if (VmAfdStringIsEqualW(
                            pszStoreName,
                            &wszCRLStoreName[0],
                            0
                            )
            )
    {
        bIsRestricted = TRUE;
    }


    return bIsRestricted;
}

DWORD
VecsDbSetDbVersion(
    DWORD dwVersion
    )
{
    DWORD dwError = 0;
    PVECS_DB_CONTEXT pDbContext = NULL;
    sqlite3_stmt* pDbQuery = NULL;

    char szQuery[] = "INSERT INTO AfdProperties ("
                     " Property,"
                     " Value)"
                     " VALUES(\"dbVersion\", :version);";

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
                        ":version",
                        dwVersion
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
VecsDbGetDbVersion(
    PDWORD pdwVersion
    )
{
    DWORD dwError = 0;
    DWORD dwVersion = 0;
    PVECS_DB_CONTEXT pDbContext = NULL;
    sqlite3_stmt* pDbQuery = NULL;
    DWORD bTableExists = 0;
    DWORD dwCount = 0;

    char szQuery[] = "SELECT Value from AfdProperties"
                     " WHERE Property = \"dbVersion\"";

    if (!pdwVersion)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VecsDbCreateContext(&pDbContext, VMAFD_DB_MODE_READ);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VecsCheckifTableExists(pDbContext->pDb, "AfdProperties", &bTableExists);
    BAIL_ON_VECS_ERROR(dwError);

    if (bTableExists)
    {
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
                            &dwVersion
                            );
            BAIL_ON_VMAFD_ERROR(dwError);
            ++dwCount;
        }
        else if (dwError == SQLITE_DONE || !dwCount)
        {
            // pre 6.5 version
            dwVersion = 0;
            dwError = 0;
        }
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    *pdwVersion = dwVersion;

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
    if (pdwVersion)
    {
        *pdwVersion = 0;
    }

    goto cleanup;
}
