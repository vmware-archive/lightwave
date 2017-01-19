/*
 * Copyright (C) 2014 VMware, Inc. All rights reserved.
 *
 * Module   : authdbutil.c
 *
 * Abstract :
 *
 */
#include "includes.h"

static
DWORD
VecsDbAddAces (
    PVECS_DB_CONTEXT pDbContext,
    DWORD  dwStoreID,
    PVMAFD_ACL pAcl,
    PDWORD pdwAceCount
    );

static
DWORD
VecsDbGetAces (
    PVECS_DB_CONTEXT pDbContext,
    DWORD dwStoreID,
    PVMAFD_ACE_LIST *ppAceList,
    PDWORD pdwAceCount
    );

static
DWORD
VecsDbGetFilteredStoreCount(
      PVECS_DB_CONTEXT pDbContext,
      PBYTE pContextBlob,
      DWORD dwContextSize,
      PDWORD pdwStoreCount
      );

/*static
DWORD
VecsDbGetAceCount(
    PVECS_DB_CONTEXT pDbContext,
    DWORD dwStoreID,
    PDWORD pdwAceCount
    );*/


DWORD
VecsDbSetSecurityDescriptor (
          DWORD dwStoreID,
          PVMAFD_SECURITY_DESCRIPTOR pSecurityDescriptor
          )
{
    DWORD dwError = 0;
    PVECS_DB_CONTEXT pDbContext = NULL;
    sqlite3_stmt* pDbQuery = NULL;
    DWORD dwContextSize = 0;
    PBYTE pSecurityContextBlob = NULL;
    DWORD dwAceCount = 0;
    DWORD dwActualAceCount = 0;
    DWORD dwIsAclPresent = 0;

    if (!pSecurityDescriptor)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VECS_ERROR (dwError);
    }

    dwError = VecsDbCreateContext(&pDbContext, VMAFD_DB_MODE_WRITE);
    BAIL_ON_VECS_ERROR (dwError);

    if (!pDbContext->pQuerySetSecurityDescriptor)
    {
        if (pSecurityDescriptor->changeStatus == VMAFD_UPDATE_STATUS_NEW)
        {
            CHAR szQuery[] = "INSERT INTO SDTable ("
                             " StoreID,"
                             " Revision,"
                             " ContextSize,"
                             " ContextBlob,"
                             " IsAclPresent)"
                             " VALUES( :storeid,"
                                  " :revision,"
                                  " :contextsize,"
                                  " :contextblob,"
                                  " :isaclpresent);" ;
            dwError = sqlite3_prepare_v2 (
                            pDbContext->pDb,
                            szQuery,
                            -1,
                            &pDbContext->pQuerySetSecurityDescriptor,
                            NULL
                            );
            BAIL_ON_VECS_ERROR (dwError);
        }
        else
        {
            CHAR szQuery[] = "UPDATE SDTable SET "
                             "Revision = :revision, "
                             "ContextSize = :contextsize, "
                             "ContextBlob = :contextblob, "
                             "IsAclPresent = :isaclpresent "
                             "WHERE StoreID = :storeid; ";

            dwError = sqlite3_prepare_v2(
                                    pDbContext->pDb,
                                    szQuery,
                                    -1,
                                    &pDbContext->pQuerySetSecurityDescriptor,
                                    NULL
                                    );
            BAIL_ON_VECS_ERROR (dwError);
        }
    }

    pDbQuery = pDbContext->pQuerySetSecurityDescriptor;


    if (!pDbContext->bInTx)
    {
      dwError = VecsDbBeginTransaction (pDbContext->pDb);
      BAIL_ON_VECS_ERROR (dwError);

      pDbContext->bInTx = TRUE;
    }

    if (pSecurityDescriptor->pOwnerSecurityContext)
    {
        DWORD dwSizeUsed = 0;
        dwError = VmAfdGetSecurityContextSize (
                          pSecurityDescriptor->pOwnerSecurityContext,
                          &dwContextSize
                          );
        BAIL_ON_VECS_ERROR (dwError);

        dwError = VmAfdAllocateMemory (
                            dwContextSize,
                            (PVOID *)&pSecurityContextBlob
                            );
        BAIL_ON_VECS_ERROR (dwError);

        dwError = VmAfdEncodeSecurityContext (
                            pSecurityDescriptor->pOwnerSecurityContext,
                            pSecurityContextBlob,
                            dwContextSize,
                            &dwSizeUsed
                            );
        BAIL_ON_VECS_ERROR (dwError);
    }


    dwError = VecsBindDword(
                        pDbQuery,
                        ":storeid",
                        dwStoreID
                        );
    BAIL_ON_VECS_ERROR (dwError);

    dwError = VecsBindDword(
                        pDbQuery,
                        ":revision",
                        pSecurityDescriptor->dwRevision
                        );
    BAIL_ON_VECS_ERROR (dwError);

    dwError = VecsBindDword(
                        pDbQuery,
                        ":contextsize",
                        dwContextSize
                        );
    BAIL_ON_VECS_ERROR (dwError);

    dwError = VecsBindBlob(
                        pDbQuery,
                        ":contextblob",
                        pSecurityContextBlob,
                        dwContextSize
                        );
    BAIL_ON_VECS_ERROR (dwError);

    if (pSecurityDescriptor->pAcl)
    {
        dwAceCount = pSecurityDescriptor->pAcl->dwAceCount;
        dwIsAclPresent = 1;
    }

    dwError = VecsBindDword (
                        pDbQuery,
                        ":isaclpresent",
                        dwIsAclPresent
                        );

    BAIL_ON_VECS_ERROR (dwError);

    dwError = VecsDbStepSql(pDbQuery);
    BAIL_ON_VECS_ERROR (dwError);

    if (dwIsAclPresent && dwAceCount)
    {
        dwError = VecsDbAddAces (
                              pDbContext,
                              dwStoreID,
                              pSecurityDescriptor->pAcl,
                              &dwActualAceCount
                              );
        BAIL_ON_VECS_ERROR (dwError);

        if (dwActualAceCount < dwAceCount)
        {
            dwError = ERROR_INVALID_TRANSACTION;
            BAIL_ON_VECS_ERROR (dwError);
        }
    }

    dwError = VecsDbCommitTransaction (pDbContext->pDb);
    BAIL_ON_VECS_ERROR (dwError);

    pDbContext->bInTx = FALSE;

    pSecurityDescriptor->changeStatus = VMAFD_UPDATE_STATUS_UNCHANGED;

cleanup:

    if (pDbQuery)
    {
        sqlite3_reset(pDbQuery);
    }

    if (pDbContext)
    {
        VecsDbReleaseContext (pDbContext);
    }

    VMAFD_SAFE_FREE_MEMORY (pSecurityContextBlob);

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
VecsDbGetSecurityDescriptor (
          DWORD dwStoreID,
          PVMAFD_SECURITY_DESCRIPTOR *ppSecurityDescriptor
          )
{
    DWORD dwError = 0;
    PVECS_DB_CONTEXT pDbContext = NULL;
    PVMAFD_SECURITY_DESCRIPTOR pSecurityDescriptor = NULL;
    DWORD dwContextSize = 0;
    PBYTE pSecurityContextBlob = 0;
    DWORD dwIsAclPresent = 0;
    DWORD dwActualAceCount = 0;
    sqlite3_stmt* pDbQuery = NULL;

    if (!ppSecurityDescriptor)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VECS_ERROR (dwError);
    }

    dwError = VecsDbCreateContext(&pDbContext, VMAFD_DB_MODE_READ);
    BAIL_ON_VECS_ERROR (dwError);

    if (!pDbContext->pQueryGetSecurityDescriptor)
    {
        CHAR szQuery[] = "SELECT * FROM SDTable"
                         " WHERE StoreID = :storeid";

        dwError = sqlite3_prepare_v2 (
                              pDbContext->pDb,
                              szQuery,
                              -1,
                              &pDbContext->pQueryGetSecurityDescriptor,
                              NULL
                              );
        BAIL_ON_VECS_ERROR (dwError);
    }

    pDbQuery = pDbContext->pQueryGetSecurityDescriptor;

    dwError = VecsBindDword (
                            pDbQuery,
                            ":storeid",
                            dwStoreID
                            );
    BAIL_ON_VECS_ERROR (dwError);

    dwError = VecsDbStepSql(pDbQuery);

    if (dwError == SQLITE_ROW)
    {
        dwError = VmAfdAllocateMemory(
                        sizeof (VMAFD_SECURITY_DESCRIPTOR),
                        (PVOID *)&pSecurityDescriptor
                        );
        BAIL_ON_VECS_ERROR (dwError);

        dwError = VecsDBGetColumnInt(
                            pDbQuery,
                            "Revision",
                            &(pSecurityDescriptor->dwRevision)
                            );
        BAIL_ON_VECS_ERROR (dwError);

        dwError = VecsDBGetColumnBlob (
                            pDbQuery,
                            "ContextBlob",
                            &pSecurityContextBlob,
                            &dwContextSize
                            );
        BAIL_ON_VECS_ERROR (dwError);

        dwError = VmAfdDecodeSecurityContext (
                            pSecurityContextBlob,
                            dwContextSize,
                            &(pSecurityDescriptor->pOwnerSecurityContext)
                            );
        BAIL_ON_VECS_ERROR (dwError);

        dwError = VecsDBGetColumnInt (
                            pDbQuery,
                            "IsAclPresent",
                            &dwIsAclPresent
                            );
        BAIL_ON_VECS_ERROR (dwError);

        if (dwIsAclPresent)
        {
            dwError = VmAfdAllocateMemory (
                          sizeof (VMAFD_ACL),
                          (PVOID *)&(pSecurityDescriptor->pAcl)
                          );
            BAIL_ON_VECS_ERROR (dwError);

            dwError = VecsDbGetAces(
                                  pDbContext,
                                  dwStoreID,
                                  &pSecurityDescriptor->pAcl->pAceList,
                                  &dwActualAceCount
                                  );
            BAIL_ON_VECS_ERROR (dwError);

            pSecurityDescriptor->pAcl->dwAceCount = dwActualAceCount;
        }
    }

    if (pSecurityDescriptor == NULL)
    {
        dwError = ERROR_OBJECT_NOT_FOUND;
        BAIL_ON_VECS_ERROR (dwError);
    }

    *ppSecurityDescriptor = pSecurityDescriptor;

cleanup:

    if (pDbQuery)
    {
        sqlite3_reset (pDbQuery);
    }
    if (pDbContext)
    {
        VecsDbReleaseContext (pDbContext);
    }

    VMAFD_SAFE_FREE_MEMORY (pSecurityContextBlob);

    return dwError;
error:
    if (ppSecurityDescriptor)
    {
        *ppSecurityDescriptor = NULL;
    }
    if (pSecurityDescriptor)
    {
        VmAfdFreeSecurityDescriptor (pSecurityDescriptor);
    }

    goto cleanup;
}

DWORD
VecsDbEnumFilteredStores (
            PBYTE pSecurityContextBlob,
            DWORD dwSizeOfContext,
            PWSTR **ppwszStoreNames,
            PDWORD pdwCount
            )
{
    DWORD dwError = 0;
    DWORD dwDbStatus = 0;
    PVECS_DB_CONTEXT pDbContext = NULL;
    sqlite3_stmt* pDbQuery = NULL;
    PWSTR *pwszStoreName = NULL;
    DWORD dwStoreCount = 0;
    DWORD dwStoresReturned = 0;
    //PVM_AFD_SECURITY_CONTEXT pEveryoneContext = NULL;
    //PBYTE pEveryoneContextBlob = NULL;

    if (!pSecurityContextBlob ||
        !ppwszStoreNames ||
        !pdwCount
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VECS_ERROR (dwError);
    }

    /*dwError = VmAfdCreateWellKnownContext(
                            VM_AFD_CONTEXT_TYPE_EVERYONE,
                            &pEveryoneContext
                            );
    BAIL_ON_VECS_ERROR (dwError);

    dwError = VmAfdEncodeSecurityContext (
                            pEveryoneContext,
                            &pEveryoneContextBlob
                            );
    BAIL_ON_VECS_ERROR (dwError);*/

    dwError = VecsDbCreateContext(&pDbContext, VMAFD_DB_MODE_READ);
    BAIL_ON_VECS_ERROR (dwError);

    dwError = VecsDbGetFilteredStoreCount (
                                            pDbContext,
                                            pSecurityContextBlob,
                                            dwSizeOfContext,
                                            &dwStoreCount
                                          );
    BAIL_ON_VECS_ERROR (dwError);

    if (dwStoreCount)
    {

        if (!pDbContext->pQueryStoreTable)
        {
            CHAR szQuery[] = "SELECT DISTINCT StoreName"
                             " FROM StoreTable"
                             " WHERE StoreID in"
                             " ("
                             " SELECT StoreID"
                             " FROM SDTable"
                             " WHERE ContextBlob = :contextblob"
                             " )"
                             " OR"
                             " StoreID in"
                             " ("
                             " SELECT StoreID"
                             " FROM AceTable"
                             " WHERE ContextBlob = :contextblob"
                             " )"
                             " LIMIT :count ;";

            dwError = sqlite3_prepare_v2 (
                              pDbContext->pDb,
                              szQuery,
                              -1,
                              &pDbContext->pQueryStoreTable,
                              NULL
                              );
            BAIL_ON_VECS_ERROR (dwError);
          }

          pDbQuery = pDbContext->pQueryStoreTable;

          dwError = VecsBindBlob (
                            pDbQuery,
                            ":contextblob",
                            pSecurityContextBlob,
                            dwSizeOfContext
                            );
          BAIL_ON_VECS_ERROR (dwError);

          dwError = VecsBindDword (
                                    pDbQuery,
                                    ":count",
                                    dwStoreCount
                                  );
          BAIL_ON_VECS_ERROR (dwError);


          dwError = VecsAllocateMemory (
                                        sizeof (PWSTR) * dwStoreCount,
                                        (PVOID *) &pwszStoreName
                                        );
          BAIL_ON_VECS_ERROR (dwError);

          do
          {
              dwDbStatus = VecsDbStepSql(pDbQuery);

              if (dwDbStatus == SQLITE_ROW)
              {
                  dwError = VecsDBGetColumnString(
                                                  pDbQuery,
                                                  "StoreName",
                                                  &pwszStoreName[dwStoresReturned]
                                                 );
                  BAIL_ON_VECS_ERROR (dwError);

                  dwStoresReturned++;
              }
              else if (dwDbStatus != SQLITE_DONE)
              {
                  dwError = dwDbStatus;
              }
          } while (dwDbStatus == SQLITE_ROW);

     }

     *ppwszStoreNames = pwszStoreName;
     *pdwCount = dwStoresReturned;

cleanup:

    if (pDbQuery)
    {
        sqlite3_reset (pDbQuery);
    }
    if (pDbContext)
    {
        VecsDbReleaseContext (pDbContext);
    }

    return dwError;
error:

    if (ppwszStoreNames)
    {
        *ppwszStoreNames = NULL;
    }

    if (pdwCount)
    {
        *pdwCount = 0;
    }

    goto cleanup;
}


static
DWORD
VecsDbAddAces (
        PVECS_DB_CONTEXT pDbContext,
        DWORD dwStoreID,
        PVMAFD_ACL pAcl,
        PDWORD pdwAceCount
        )
{
    DWORD dwError = 0;
    PVMAFD_ACE_LIST pAceList = NULL;
    DWORD dwContextSize = 0;
    PBYTE pSecurityContextBlob = NULL;
    sqlite3_stmt* pDbQuery = NULL;
    sqlite3_stmt** pDbQueryArray = NULL;
    DWORD dwAceCount = 0;
    DWORD dwIndx = 0;

    if (!dwStoreID || !pAcl || !pDbContext)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VECS_ERROR (dwError);
    }

    if (pDbContext->dbOpenMode != VMAFD_DB_MODE_WRITE)
    {
        dwError = ERROR_INVALID_ACCESS;
        BAIL_ON_VECS_ERROR(dwError);
    }

    pAceList = pAcl->pAceList;

    while (pAceList)
    {
        dwAceCount ++;
        pAceList = pAceList->pNext;
    }

    if (!pDbContext->pQueryAddAcesArray)
    {
        dwError = VmAfdAllocateMemory (
                                dwAceCount * sizeof (sqlite3_stmt*),
                                (PVOID *) &pDbQueryArray
                                );
        BAIL_ON_VECS_ERROR (dwError);

        dwError = VmAfdAllocateMemory (
                                sizeof (VECS_DB_STMT_ARRAY),
                                (PVOID *) &pDbContext->pQueryAddAcesArray
                                );
        BAIL_ON_VECS_ERROR (dwError);

        pDbContext->pQueryAddAcesArray->pStmtToExecute = pDbQueryArray;
        pDbContext->pQueryAddAcesArray->dwCount = dwAceCount;
    }

    pAceList = pAcl->pAceList;

    while (pAceList)
    {
        sqlite3_stmt* pAddAceCursor = NULL;

        if (pAceList->Ace.changeStatus == VMAFD_UPDATE_STATUS_NEW)
        {
              CHAR szQuery[] = "INSERT INTO AceTable ("
                             " StoreID,"
                             " ContextSize,"
                             " ContextBlob,"
                             " AccessMask,"
                             " AccessType)"
                             " VALUES( :storeid,"
                                        " :contextsize,"
                                        " :contextblob,"
                                        " :accessmask,"
                                        " :accesstype);";
              dwError = sqlite3_prepare_v2 (
                                pDbContext->pDb,
                                szQuery,
                                -1,
                                &pAddAceCursor,
                                NULL
                                );
              BAIL_ON_VECS_ERROR (dwError);
        }
        else
        {
              CHAR szQuery[] = "UPDATE AceTable "
                               "SET AccessMask = :accessmask,"
                               "AccessType = :accesstype, "
                               "ContextSize = :contextsize "
                               "WHERE StoreID = :storeid "
                               "AND ContextBlob = :contextblob;";
              dwError = sqlite3_prepare_v2 (
                                  pDbContext->pDb,
                                  szQuery,
                                  -1,
                                  &pAddAceCursor,
                                  NULL
                                  );

              BAIL_ON_VECS_ERROR (dwError);
        }

        pDbContext->pQueryAddAcesArray->pStmtToExecute[dwIndx] = pAddAceCursor;

        pDbQuery = pAddAceCursor;

        if (pAceList->Ace.pSecurityContext)
        {
            DWORD dwContextSizeRead = 0;
            dwError = VmAfdGetSecurityContextSize(
                                pAceList->Ace.pSecurityContext,
                                &dwContextSize
                                );
            BAIL_ON_VECS_ERROR (dwError);

            dwError = VmAfdAllocateMemory (
                                dwContextSize,
                                (PVOID *)&pSecurityContextBlob
                                );
            BAIL_ON_VECS_ERROR (dwError);

            dwError = VmAfdEncodeSecurityContext (
                                pAceList->Ace.pSecurityContext,
                                pSecurityContextBlob,
                                dwContextSize,
                                &dwContextSizeRead
                                );
            BAIL_ON_VECS_ERROR (dwError);
        }

        dwError = VecsBindDword(
                              pDbQuery,
                              ":storeid",
                              dwStoreID
                              );
        BAIL_ON_VECS_ERROR (dwError);

        dwError = VecsBindDword (
                              pDbQuery,
                              ":contextsize",
                              dwContextSize
                              );
        BAIL_ON_VECS_ERROR (dwError);

        dwError = VecsBindBlob (
                              pDbQuery,
                              ":contextblob",
                              pSecurityContextBlob,
                              dwContextSize
                              );
        BAIL_ON_VECS_ERROR (dwError);

        dwError = VecsBindDword (
                              pDbQuery,
                              ":accessmask",
                              pAceList->Ace.accessMask
                              );
        BAIL_ON_VECS_ERROR (dwError);

        dwError = VecsBindDword (
                              pDbQuery,
                              ":accesstype",
                              pAceList->Ace.type
                              );
        BAIL_ON_VECS_ERROR (dwError);

        dwError = VecsDbStepSql (pDbQuery);
        BAIL_ON_VECS_ERROR (dwError);

        VMAFD_SAFE_FREE_MEMORY (pSecurityContextBlob);
        dwContextSize = 0;
        pAceList->Ace.changeStatus = VMAFD_UPDATE_STATUS_UNCHANGED;

        pAceList = pAceList->pNext;
        dwIndx ++;
    }
    if (pdwAceCount)
    {
        *pdwAceCount = dwAceCount;
    }

cleanup:
    VMAFD_SAFE_FREE_MEMORY (pSecurityContextBlob);

    return dwError;
error:
    if (pdwAceCount)
    {
        *pdwAceCount = 0;
    }
    goto cleanup;
}

static
DWORD
VecsDbGetAces (
    PVECS_DB_CONTEXT pDbContext,
    DWORD dwStoreID,
    PVMAFD_ACE_LIST *ppAceList,
    PDWORD pdwAceCount
    )
{
    DWORD dwError = 0;
    DWORD dwDbStatus = 0;
    PVMAFD_ACE_LIST pAceListCurr = NULL;
    PVMAFD_ACE_LIST pAceListPrev = NULL;
    PBYTE pSecurityContextBlob = NULL;
    DWORD dwAceCount = 0;

    sqlite3_stmt* pDbQuery = NULL;

    if (!pDbContext || !ppAceList)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VECS_ERROR (dwError);
    }

    if (!pDbContext->pQueryGetAces)
    {
        CHAR szQuery[] = "SELECT * FROM AceTable"
                         " WHERE StoreID = :storeid;";

        dwError = sqlite3_prepare_v2 (
                                pDbContext->pDb,
                                szQuery,
                                -1,
                                &pDbContext->pQueryGetAces,
                                NULL
                                );
        BAIL_ON_VECS_ERROR (dwError);
    }

    pDbQuery = pDbContext->pQueryGetAces;

    dwError = VecsBindDword(
                    pDbQuery,
                    ":storeid",
                    dwStoreID
                    );
    BAIL_ON_VECS_ERROR (dwError);

    do
    {
        dwDbStatus = VecsDbStepSql (pDbQuery);

        if (dwDbStatus == SQLITE_ROW)
        {
            DWORD dwContextSize = 0;

            dwError = VmAfdAllocateMemory (
                                sizeof (VMAFD_ACE_LIST),
                                (PVOID *)&pAceListCurr
                                );
             BAIL_ON_VECS_ERROR (dwError);

            pAceListCurr->pNext = pAceListPrev;

            dwError = VecsDBGetColumnBlob (
                                pDbQuery,
                                "ContextBlob",
                                &pSecurityContextBlob,
                                &dwContextSize
                                );
            BAIL_ON_VECS_ERROR (dwError);

            dwError = VmAfdDecodeSecurityContext (
                                  pSecurityContextBlob,
                                  dwContextSize,
                                  &(pAceListCurr->Ace.pSecurityContext)
                                  );
            BAIL_ON_VECS_ERROR (dwError);

            VMAFD_SAFE_FREE_MEMORY (pSecurityContextBlob);

            dwError = VecsDBGetColumnInt (
                                    pDbQuery,
                                    "AccessMask",
                                    &pAceListCurr->Ace.accessMask
                                    );
            BAIL_ON_VECS_ERROR (dwError);

            dwError = VecsDBGetColumnInt (
                                    pDbQuery,
                                    "AccessType",
                                    &pAceListCurr->Ace.type
                                    );
            BAIL_ON_VECS_ERROR (dwError);

            pAceListPrev = pAceListCurr;
            pAceListCurr = NULL;
            dwAceCount++;

        }
        else if (dwDbStatus != SQLITE_DONE)
        {
            dwError = dwDbStatus;
            BAIL_ON_VECS_ERROR (dwError);
        }
    }while (dwDbStatus == SQLITE_ROW);

    *ppAceList = pAceListPrev;
    if (pdwAceCount)
    {
        *pdwAceCount = dwAceCount;
    }

cleanup:
    VMAFD_SAFE_FREE_MEMORY (pSecurityContextBlob);
    if (pDbQuery)
    {
        sqlite3_reset (pDbQuery);
    }
    return dwError;

error:
    if (ppAceList)
    {
        *ppAceList = NULL;
    }
    if (pAceListPrev)
    {
        VmAfdFreeAceList (pAceListPrev);
    }
    if (pdwAceCount)
    {
        *pdwAceCount = 0;
    }

    goto cleanup;
}

static
DWORD
VecsDbGetFilteredStoreCount(
            PVECS_DB_CONTEXT pDbContext,
            PBYTE pContextBlob,
            DWORD dwContextSize,
            PDWORD pdwCount
            )
{
    DWORD dwError = 0;
    sqlite3_stmt* pDbQuery = NULL;
    DWORD dwStoreCount = 0;

    char szQuery[] = "SELECT COUNT(*)"
                     " FROM StoreTable"
                     " WHERE StoreID in"
                     " ("
                     " SELECT StoreID"
                     " FROM SDTable"
                     " WHERE ContextBlob = :contextblob"
                     " )"
                     " OR"
                     " StoreID in"
                     " ("
                     " SELECT StoreID"
                     " FROM AceTable"
                     " WHERE ContextBlob = :contextblob"
                     " );";

    if (!pDbContext || !pdwCount)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VECS_ERROR (dwError);
    }

    dwError = sqlite3_prepare_v2 (
                                  pDbContext->pDb,
                                  szQuery,
                                  -1,
                                  &pDbQuery,
                                  NULL
                                  );
    BAIL_ON_VECS_ERROR (dwError);

    dwError = VecsBindBlob (
                            pDbQuery,
                            ":contextblob",
                            pContextBlob,
                            dwContextSize
                            );
    BAIL_ON_VECS_ERROR (dwError);

    dwError = VecsDbStepSql (pDbQuery);
    if (dwError == SQLITE_ROW)
    {
        dwError = 0;
        dwStoreCount = sqlite3_column_int(pDbQuery, 0);
    }
    BAIL_ON_VECS_ERROR (dwError);

    *pdwCount = dwStoreCount;

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

#ifndef _WIN32
DWORD
VecsDbCleanupPermissions(VOID)
{
    DWORD dwError = 0;
    PVECS_DB_CONTEXT pDbContext = NULL;
    sqlite3_stmt* pDbCleanSD = NULL;
    sqlite3_stmt* pDbCleanACE = NULL;
    DWORD dwContextSize = 0;
    DWORD dwSizeUsed = 0;
    PVM_AFD_SECURITY_CONTEXT pRootContext = NULL;
    PVOID pRootContextBlob = NULL;

    CHAR szCleanSD[] = "UPDATE SDTable "
                       "SET ContextSize = :contextsize,"
                       "ContextBlob = :rootBlob "
                       "WHERE ContextSize != :contextsize;";
    CHAR szCleanACE[] = "DELETE FROM AceTable WHERE ContextSize != :contextsize;";

    dwError = VecsDbCreateContext(&pDbContext, VMAFD_DB_MODE_WRITE);
    BAIL_ON_VECS_ERROR (dwError);

    dwError = sqlite3_prepare_v2 (
                    pDbContext->pDb,
                    szCleanSD,
                    -1,
                    &pDbCleanSD,
                    NULL
                    );
    BAIL_ON_VECS_ERROR (dwError);

    dwError = sqlite3_prepare_v2 (
                    pDbContext->pDb,
                    szCleanACE,
                    -1,
                    &pDbCleanACE,
                    NULL
                    );
    BAIL_ON_VECS_ERROR (dwError);

    if (!pDbContext->bInTx)
    {
        dwError = VecsDbBeginTransaction (pDbContext->pDb);
        BAIL_ON_VECS_ERROR (dwError);

        pDbContext->bInTx = TRUE;
    }

    dwError = VmAfdCreateWellKnownContext(
                              VM_AFD_CONTEXT_TYPE_ROOT,
                              &pRootContext
                              );
    BAIL_ON_VECS_ERROR(dwError);

    dwError = VmAfdGetSecurityContextSize (
                                pRootContext,
                                &dwContextSize
                                );
    BAIL_ON_VECS_ERROR (dwError);

    dwError = VecsBindDword (
                    pDbCleanSD,
                    ":contextsize",
                    dwContextSize
                    );
    BAIL_ON_VECS_ERROR (dwError);

    dwError = VmAfdAllocateMemory (
                        dwContextSize,
                        (PVOID *)&pRootContextBlob
                        );
    BAIL_ON_VECS_ERROR (dwError);

    dwError = VmAfdEncodeSecurityContext (
                        pRootContext,
                        pRootContextBlob,
                        dwContextSize,
                        &dwSizeUsed
                        );
    BAIL_ON_VECS_ERROR (dwError);

    dwError = VecsBindBlob(
                        pDbCleanSD,
                        ":rootBlob",
                        pRootContextBlob,
                        dwContextSize
                        );
    BAIL_ON_VECS_ERROR (dwError);

    dwError = VecsBindDword (
                    pDbCleanACE,
                    ":contextsize",
                    dwContextSize
                    );
    BAIL_ON_VECS_ERROR (dwError);

    dwError = VecsDbStepSql(pDbCleanSD);
    BAIL_ON_VECS_ERROR (dwError);

    dwError = VecsDbStepSql(pDbCleanACE);
    BAIL_ON_VECS_ERROR (dwError);

    dwError = VecsDbCommitTransaction (pDbContext->pDb);
    BAIL_ON_VECS_ERROR (dwError);

    pDbContext->bInTx = FALSE;

cleanup:
    VMAFD_SAFE_FREE_MEMORY(pRootContextBlob);
    if (pRootContext)
    {
        VmAfdFreeSecurityContext(pRootContext);
    }
    if (pDbCleanSD)
    {
        sqlite3_reset(pDbCleanSD);
        sqlite3_finalize(pDbCleanSD);
    }
    if (pDbCleanACE)
    {
        sqlite3_reset(pDbCleanACE);
        sqlite3_finalize(pDbCleanACE);
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
#endif

/*static
DWORD
VecsDbGetAceCount(
    PVECS_DB_CONTEXT pDbContext,
    DWORD dwStoreID,
    PDWORD pdwAceCount
    )
{
    DWORD dwError = 0;
    sqlite3_stmt* pDbQuery = NULL;
    DWORD dwAceCount = 0;

    char szQuery[] = "SELECT COUNT(*)"
                     " FROM AceTable"
                     " WHERE StoreID = :storeid;";

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

    dwError = VecsBindDword (
                        pDbQuery,
                        ":storeid",
                        dwStoreID
                        );
    BAIL_ON_VECS_ERROR (dwError);

    dwError = VecsDbStepSql(pDbQuery);
    if (dwError == SQLITE_ROW)
    {
        dwError = 0;
        dwAceCount = sqlite3_column_int(pDbQuery, 0);
    }
    BAIL_ON_VECS_ERROR (dwError);

    *pdwAceCount = dwAceCount;
cleanup:
    if (pDbQuery)
    {
      sqlite3_finalize (pDbQuery);
    }

    return dwError;
error:
    if (pdwAceCount)
    {
        *pdwAceCount = 0;
    }

    goto cleanup;
}*/
