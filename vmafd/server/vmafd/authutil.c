/*
 * Copyright (C) 2014 VMware, Inc. All rights reserved.
 *
 * Module   : authutil.c
 *
 * Abstract :
 *
 */
#include "includes.h"

static
DWORD
VmAfdCheckAcl (
      PVMAFD_SECURITY_DESCRIPTOR pSecurityDescriptor,
      PVM_AFD_SECURITY_CONTEXT pSecurityContext,
      DWORD dwDesiredAccess
      );

static
DWORD
VmAfdCopyAcl(
             PVMAFD_ACL pAclSrc,
             PVMAFD_ACL *ppAclDest
            );

static
DWORD
VmAfdCopyAceList(
                  PVMAFD_ACE_LIST pAceListSrc,
                  PVMAFD_ACE_LIST *ppAceListDest
                );


DWORD
VmAfdInitializeSecurityDescriptor (
    PVM_AFD_SECURITY_CONTEXT pSecurityContext,
    DWORD dwRevision,
    PVMAFD_SECURITY_DESCRIPTOR *ppSecurityDescriptor
    )
{
    DWORD dwError = 0;
    PVMAFD_SECURITY_DESCRIPTOR pSecurityDescriptor = NULL;
    PVMAFD_ACL pAcl = NULL;

    if (!pSecurityContext ||
        !ppSecurityDescriptor
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    dwError = VmAfdAllocateMemory (
                          sizeof (VMAFD_SECURITY_DESCRIPTOR),
                          (PVOID *)&pSecurityDescriptor
                          );
    BAIL_ON_VMAFD_ERROR (dwError);

    dwError = VmAfdAllocateMemory (
                      sizeof (VMAFD_ACL),
                      (PVOID *)&pAcl
                      );
    BAIL_ON_VMAFD_ERROR (dwError);


    dwError = VmAfdCopySecurityContext (
                                        pSecurityContext,
                                        &pSecurityDescriptor->pOwnerSecurityContext
                                       );
    BAIL_ON_VMAFD_ERROR (dwError);

    pSecurityDescriptor->dwRevision = dwRevision;
    pSecurityDescriptor->changeStatus = VMAFD_UPDATE_STATUS_NEW;

    pAcl->dwAceCount = 0;

    pSecurityDescriptor->pAcl = pAcl;

    *ppSecurityDescriptor = pSecurityDescriptor;

cleanup:
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

    if (pAcl)
    {
        VmAfdFreeAcl (pAcl);
    }

    goto cleanup;
}

VOID
VmAfdFreeAce (
    PVMAFD_ACE pAce
    )
{
    if (pAce)
    {
        if (pAce->pSecurityContext)
        {
            VmAfdFreeSecurityContext(
                             pAce->pSecurityContext
                             );
        }
        VMAFD_SAFE_FREE_MEMORY (pAce);
    }
}


VOID
VmAfdFreeAceList (
    PVMAFD_ACE_LIST pAceList
    )
{
    PVMAFD_ACE_LIST pAceListCurr = pAceList;
    PVMAFD_ACE_LIST pAceListNext = NULL;

    while (pAceListCurr)
    {
        pAceListNext = pAceListCurr->pNext;

        if (pAceListCurr->Ace.pSecurityContext)
        {
            VmAfdFreeSecurityContext (pAceListCurr->Ace.pSecurityContext);
        }

        VMAFD_SAFE_FREE_MEMORY (pAceListCurr);

        pAceListCurr = pAceListNext;
    }
}

VOID
VmAfdFreeAcl (
     PVMAFD_ACL pAcl
     )
{
    if (pAcl)
    {
        if (pAcl->pAceList)
        {
            VmAfdFreeAceList (pAcl->pAceList);
        }

        VMAFD_SAFE_FREE_MEMORY (pAcl);
    }
}

VOID
VmAfdFreeSecurityDescriptor (
    PVMAFD_SECURITY_DESCRIPTOR pSecurityDescriptor
    )
{
    if (pSecurityDescriptor)
    {
        if (pSecurityDescriptor->pOwnerSecurityContext)
        {
            VmAfdFreeSecurityContext (
                        pSecurityDescriptor->pOwnerSecurityContext
                        );
        }
        if (pSecurityDescriptor->pAcl)
        {
            VmAfdFreeAcl (pSecurityDescriptor->pAcl);
        }

        VMAFD_SAFE_FREE_MEMORY (pSecurityDescriptor);
    }
}

DWORD
VmAfdSetSecurityDescriptor (
      PVECS_SERV_STORE pStore,
      PVMAFD_SECURITY_DESCRIPTOR pSecurityDescriptor
      )
{
    DWORD dwError = 0;

    if (!pSecurityDescriptor ||
        !pStore
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    dwError = VecsDbSetSecurityDescriptor (
                                pStore->dwStoreId,
                                pSecurityDescriptor
                                );
    BAIL_ON_VMAFD_ERROR (dwError);

cleanup:
    return dwError;
error:
    goto cleanup;
}

DWORD
VmAfdGetSecurityDescriptor (
          PVECS_SERV_STORE pStore,
          PVMAFD_SECURITY_DESCRIPTOR *ppSecurityDescriptor
          )
{
    DWORD dwError = 0;

    PVMAFD_SECURITY_DESCRIPTOR pSecurityDescriptor = NULL;

    if (!pStore ||
        !ppSecurityDescriptor
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    dwError = VecsDbGetSecurityDescriptor (
                            pStore->dwStoreId,
                            &pSecurityDescriptor
                            );
    BAIL_ON_VMAFD_ERROR (dwError);

    *ppSecurityDescriptor = pSecurityDescriptor;

cleanup:
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
VmAfdAccessCheckWithHandle (
      PVECS_SRV_STORE_HANDLE pStore,
      PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
      DWORD dwDesiredAccess
      )
{
    DWORD dwError = 0;
    DWORD dwLogError = 0;
    PVECS_SERV_STORE pStoreInfo = NULL;


    PVMAFD_SECURITY_DESCRIPTOR pSecurityDescriptor = NULL;
    PWSTR pszAccountName = NULL;

    if (!pStore ||
        !pConnectionContext ||
        !pConnectionContext->pSecurityContext
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    if ((dwDesiredAccess | VECS_MAXIMUM_ALLOWED_MASK) !=
              VECS_MAXIMUM_ALLOWED_MASK
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    /*
     * We don't care about dwLogError errors because they are
     * used solely for logging purpose. Even if some call fails,
     * the function should not fail
     */

    dwLogError = VmAfdAllocateNameFromContext (
                                                pConnectionContext->pSecurityContext,
                                                &pszAccountName
                                              );


    dwLogError = VmAfdGetStoreFromHandle (
                                          pStore,
                                          pConnectionContext->pSecurityContext,
                                          &pStoreInfo
                                         );

    if (
        !IsNullOrEmptyString(pszAccountName) &&
        pStoreInfo
       )
    {
        PSTR paszAccountName = NULL;
        dwLogError = VmAfdAllocateStringAFromW(
                                                pszAccountName,
                                                &paszAccountName
                                              );
        if (paszAccountName)
        {
          switch (dwDesiredAccess)
          {
            case READ_STORE:
              VmAfdLog (VMAFD_DEBUG_DEBUG,
                  "User %s requested READ operation on Store with ID: %d",
                  paszAccountName,
                  pStoreInfo->dwStoreId
                  );
             break;
            case WRITE_STORE:
              VmAfdLog (VMAFD_DEBUG_DEBUG,
                  "User %s requested WRITE operation on  Store with ID:%d",
                  paszAccountName,
                  pStoreInfo->dwStoreId
                  );
              break;

            default:
              break;
          }
        }
        VMAFD_SAFE_FREE_MEMORY (paszAccountName);
    }

    dwError = VmAfdGetSecurityDescriptorFromHandle (
                          pStore,
                          &pSecurityDescriptor
                          );
    BAIL_ON_VMAFD_ERROR (dwError);

    if (!(VmAfdIsRootSecurityContext (pConnectionContext)))
    {
       if (!(VmAfdEqualsSecurityContext(
                     pConnectionContext->pSecurityContext,
                     pSecurityDescriptor->pOwnerSecurityContext
                      )
            ))
       {
          dwError = VmAfdCheckAcl (
                            pSecurityDescriptor,
                            pConnectionContext->pSecurityContext,
                            dwDesiredAccess
                            );

         BAIL_ON_VMAFD_ERROR (dwError);
       }
    }

cleanup:
    VMAFD_SAFE_FREE_MEMORY (pszAccountName);
    VMAFD_SAFE_FREE_MEMORY (pStoreInfo);
    if (pSecurityDescriptor)
    {
        VmAfdFreeSecurityDescriptor (pSecurityDescriptor);
    }

    return dwError;

error:
    goto cleanup;

}


DWORD
VmAfdAccessCheck (
      PVECS_SERV_STORE pStore,
      PVM_AFD_CONNECTION_CONTEXT pSecurityContext,
      DWORD dwDesiredAccess
      )
{
    DWORD dwError = 0;

    /*
    PVMAFD_SECURITY_DESCRIPTOR pSecurityDescriptor = NULL;

    if (!pStore ||
        !pSecurityContext
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    if ((dwDesiredAccess | VECS_MAXIMUM_ALLOWED_MASK) !=
              VECS_MAXIMUM_ALLOWED_MASK
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    dwError = VmAfdGetSecurityDescriptor (
                          pStore,
                          &pSecurityDescriptor
                          );
    BAIL_ON_VMAFD_ERROR (dwError);

    if (!(VmAfdIsRootSecurityContext (pSecurityContext)))
    {
       if (!(VmAfdEqualsSecurityContext(
                     pSecurityContext,
                     pSecurityDescriptor->pOwnerSecurityContext
                      )
            ))
       {
          dwError = VmAfdCheckAcl (
                            pSecurityDescriptor,
                            pSecurityContext,
                            dwDesiredAccess
                            );
         BAIL_ON_VMAFD_ERROR (dwError);
       }
    }

cleanup:
    if (pSecurityDescriptor)
    {
        VmAfdFreeSecurityDescriptor (pSecurityDescriptor);
    }

    return dwError;

error:
    goto cleanup;
    */

    return dwError;
}

DWORD
VmAfdCheckOwnerShip (
      PVECS_SERV_STORE pStore,
      PVM_AFD_SECURITY_CONTEXT pSecurityContext
      )
{
    DWORD dwError = 0;
    PVMAFD_SECURITY_DESCRIPTOR pSecurityDescriptor = NULL;
    if (!pStore ||
        !pSecurityContext
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    dwError = VmAfdGetSecurityDescriptor (
                          pStore,
                          &pSecurityDescriptor
                          );
    BAIL_ON_VMAFD_ERROR (dwError);

    if (!(VmAfdEqualsSecurityContext(
                 pSecurityContext,
                 pSecurityDescriptor->pOwnerSecurityContext
                  )
        ))
    {
      dwError = ERROR_ACCESS_DENIED;
      BAIL_ON_VMAFD_ERROR (dwError);
    }

cleanup:
    if (pSecurityDescriptor)
    {
        VmAfdFreeSecurityDescriptor (pSecurityDescriptor);
    }

    return dwError;

error:
    goto cleanup;
}


DWORD
VmAfdCheckOwnerShipWithHandle (
      PVECS_SRV_STORE_HANDLE pStore,
      PVM_AFD_CONNECTION_CONTEXT pConnectionContext
      )
{
    DWORD dwError = 0;
    PVMAFD_SECURITY_DESCRIPTOR pSecurityDescriptor = NULL;
    if (!pStore ||
        !pConnectionContext ||
        !pConnectionContext->pSecurityContext
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    dwError = VmAfdGetSecurityDescriptorFromHandle (
                          pStore,
                          &pSecurityDescriptor
                          );
    BAIL_ON_VMAFD_ERROR (dwError);

    if (!(VmAfdIsRootSecurityContext (pConnectionContext)))
    {
       if (!(VmAfdEqualsSecurityContext(
                     pConnectionContext->pSecurityContext,
                     pSecurityDescriptor->pOwnerSecurityContext
                      )
            ))
       {
          dwError = ERROR_ACCESS_DENIED;
          BAIL_ON_VMAFD_ERROR (dwError);
       }
    }

cleanup:
    if (pSecurityDescriptor)
    {
        VmAfdFreeSecurityDescriptor (pSecurityDescriptor);
    }

    return dwError;

error:
    goto cleanup;
}


DWORD
VmAfdModifyPermissions (
      PVECS_SERV_STORE pStore,
      PCWSTR pszServiceName,
      DWORD accessMask,
      VMAFD_ACE_TYPE aceType,
      PVMAFD_SECURITY_DESCRIPTOR pSecurityDescriptor,
      VMW_IPC_MODIFY_PERMISSIONS modifyType
      )
{
    DWORD dwError = 0;
    PVM_AFD_SECURITY_CONTEXT pSecurityContext = NULL;
    PVMAFD_ACL pAcl = NULL;
    PVMAFD_ACE_LIST pAceList = NULL;
    PVMAFD_ACE_LIST pAceListCursor = NULL;
    PVMAFD_ACE_LIST pAceListNew = NULL;
    WCHAR wszEveryone[] = GROUP_EVERYONE_W;

    if (IsNullOrEmptyString (pszServiceName) ||
        !pSecurityDescriptor ||
        !modifyType ||
        !pStore ||
        !accessMask
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    if (VmAfdStringIsEqualW(
                           pszServiceName,
                           wszEveryone,
                           FALSE
                           )
       )
    {
         dwError = VmAfdCreateWellKnownContext (
                                VM_AFD_CONTEXT_TYPE_EVERYONE,
                                &pSecurityContext
                                );
    }

    else
    {

            dwError = VmAfdAllocateContextFromName (
                                    pszServiceName,
                                    &pSecurityContext
                                    );
    }
    BAIL_ON_VMAFD_ERROR (dwError);

    dwError = VmAfdCheckOwnerShip (
                                  pStore,
                                  pSecurityContext
                                  );

    if (dwError == 0)
    {
        goto cleanup;
    }
    dwError = 0;

    if (
        !pSecurityDescriptor->pAcl &&
        modifyType != VMW_IPC_MODIFY_PERMISSIONS_REVOKE
       )
    {
          dwError = VmAfdAllocateMemory(
                                sizeof (VMAFD_ACL),
                                (PVOID *) &pAcl
                                );
          BAIL_ON_VMAFD_ERROR (dwError);

          dwError = VmAfdAllocateMemory (
                                sizeof (VMAFD_ACE_LIST),
                                (PVOID *) pAceList
                                );
          BAIL_ON_VMAFD_ERROR (dwError);

          dwError = VmAfdCopySecurityContext(
                                    pSecurityContext,
                                    &(pAceList->Ace.pSecurityContext
                                     )
                                    );
          BAIL_ON_VMAFD_ERROR (dwError);

          pAceList->Ace.accessMask = accessMask;
          pAceList->Ace.changeStatus = VMAFD_UPDATE_STATUS_NEW;
          pAceList->Ace.type = aceType;

          pAcl->pAceList = pAceList;
          pAcl->dwAceCount ++;
          pSecurityDescriptor->pAcl = pAcl;
    }
    else if (pSecurityDescriptor->pAcl)
    {
        pAceListCursor = pSecurityDescriptor->pAcl->pAceList;

        while (pAceListCursor)
        {
            if (
                    VmAfdEqualsSecurityContext(
                          pSecurityContext,
                          pAceListCursor->Ace.pSecurityContext
                          )
                    &&
                    pAceListCursor->Ace.type == aceType
              )
            {
              break;
            }
            pAceListCursor = pAceListCursor->pNext;
        }

        if (
            !pAceListCursor &&
            modifyType != VMW_IPC_MODIFY_PERMISSIONS_REVOKE
           )
        {

          dwError = VmAfdAllocateMemory (
                            sizeof (VMAFD_ACE_LIST),
                            (PVOID *) &pAceListNew
                            );
          BAIL_ON_VMAFD_ERROR (dwError);

          dwError = VmAfdCopySecurityContext (
                                          pSecurityContext,
                                          &(pAceListNew->Ace.pSecurityContext)
                                          );
          BAIL_ON_VMAFD_ERROR (dwError);

          pAceListNew->pNext = pSecurityDescriptor->pAcl->pAceList;
          pAceListNew->Ace.accessMask = accessMask;
          pAceListNew->Ace.changeStatus = VMAFD_UPDATE_STATUS_NEW;
          pAceListNew->Ace.type = aceType;
          pSecurityDescriptor->pAcl->pAceList = pAceListNew;
          pSecurityDescriptor->pAcl->dwAceCount ++;
        }
        else if (pAceListCursor)
        {
            switch (modifyType)
            {
                case VMW_IPC_MODIFY_PERMISSIONS_SET:
                  pAceListCursor->Ace.accessMask = accessMask;
                  break;
                case VMW_IPC_MODIFY_PERMISSIONS_ADD:
                  pAceListCursor->Ace.accessMask = pAceListCursor->Ace.accessMask |
                                                   accessMask;
                  break;
                case VMW_IPC_MODIFY_PERMISSIONS_REVOKE:
                  pAceListCursor->Ace.accessMask = pAceListCursor->Ace.accessMask &
                                                   ~accessMask;
                  break;
                default:
                  dwError = ERROR_INVALID_PARAMETER;
                  BAIL_ON_VMAFD_ERROR (dwError);
            }
            pAceListCursor->Ace.changeStatus = VMAFD_UPDATE_STATUS_MODIFIED;
        }
    }
    if (pSecurityDescriptor->changeStatus == VMAFD_UPDATE_STATUS_UNCHANGED)
    {
        pSecurityDescriptor->changeStatus = VMAFD_UPDATE_STATUS_MODIFIED;
    }
cleanup:
    if (pSecurityContext)
    {
        VmAfdFreeSecurityContext (pSecurityContext);
    }

    return dwError;

error:
    if (pAcl)
    {
        VmAfdFreeAcl (pAcl);
    }
    if (pAceList)
    {
        VmAfdFreeAceList (pAceList);
    }
    if (pAceListNew)
    {
        VmAfdFreeAceList (pAceListNew);
    }
    goto cleanup;
}


DWORD
VmAfdModifyOwner(
          PVECS_SERV_STORE pStore,
          PCWSTR pszUserName,
          PVMAFD_SECURITY_DESCRIPTOR pSecurityDescriptor
          )
{
    DWORD dwError = 0;
    PVM_AFD_SECURITY_CONTEXT pSecurityContext = NULL;
    PVM_AFD_SECURITY_CONTEXT pSecurityContextToClean = NULL;

    if (IsNullOrEmptyString (pszUserName) ||
        !pSecurityDescriptor ||
        !pStore
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    dwError = VmAfdAllocateContextFromName (
                                    pszUserName,
                                    &pSecurityContext
                                    );
    BAIL_ON_VMAFD_ERROR (dwError);

    pSecurityContextToClean = pSecurityContext;

    dwError = VmAfdCheckOwnerShip (
                                  pStore,
                                  pSecurityContext
                                  );

    if (dwError == 0)
    {
        goto cleanup;
    }
    dwError = 0;

    pSecurityContextToClean = pSecurityDescriptor->pOwnerSecurityContext;

    pSecurityDescriptor->pOwnerSecurityContext = pSecurityContext;

cleanup:
    if (pSecurityContextToClean)
    {
        VmAfdFreeSecurityContext (pSecurityContextToClean);
    }

    return dwError;

error:
    goto cleanup;
}

DWORD
VmAfdCopySecurityDescriptor (
                              PVMAFD_SECURITY_DESCRIPTOR pSecurityDescriptorSrc,
                              PVMAFD_SECURITY_DESCRIPTOR *ppSecurityDescriptorDest
                            )
{
    DWORD dwError = 0;
    PVMAFD_SECURITY_DESCRIPTOR pSecurityDescriptor = NULL;

    if (!pSecurityDescriptorSrc ||
        !ppSecurityDescriptorDest
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    dwError = VmAfdAllocateMemory (
                                    sizeof (VMAFD_SECURITY_DESCRIPTOR),
                                    (PVOID *)&pSecurityDescriptor
                                  );
    BAIL_ON_VMAFD_ERROR (dwError);

    dwError = VmAfdCopySecurityContext(
                        pSecurityDescriptorSrc->pOwnerSecurityContext,
                        &pSecurityDescriptor->pOwnerSecurityContext
                        );
    BAIL_ON_VMAFD_ERROR (dwError);

    dwError = VmAfdCopyAcl(
                       pSecurityDescriptorSrc->pAcl,
                       &pSecurityDescriptor->pAcl
                       );
    BAIL_ON_VMAFD_ERROR (dwError);

    pSecurityDescriptor->dwRevision = pSecurityDescriptorSrc->dwRevision;
    pSecurityDescriptor->changeStatus = pSecurityDescriptorSrc->changeStatus;

    *ppSecurityDescriptorDest = pSecurityDescriptor;

cleanup:
    return dwError;

error:
    if (ppSecurityDescriptorDest)
    {
        *ppSecurityDescriptorDest = NULL;
    }

    if (pSecurityDescriptor)
    {
        VmAfdFreeSecurityDescriptor(pSecurityDescriptor);
    }

    goto cleanup;
}

static
DWORD
VmAfdCopyAcl(
             PVMAFD_ACL pAclSrc,
             PVMAFD_ACL *ppAclDest
            )
{
    DWORD dwError = 0;
    PVMAFD_ACL pAcl = NULL;

    if (!pAclSrc ||
        !ppAclDest
        )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    dwError = VmAfdAllocateMemory (
                                    sizeof (VMAFD_ACL),
                                    (PVOID *) &pAcl
                                  );
    BAIL_ON_VMAFD_ERROR (dwError);

    dwError = VmAfdCopyAceList (
                                pAclSrc->pAceList,
                                &pAcl->pAceList
                               );
    BAIL_ON_VMAFD_ERROR (dwError);

    pAcl->dwAceCount = pAclSrc->dwAceCount;

    *ppAclDest = pAcl;

cleanup:
    return dwError;

error:
    if (ppAclDest)
    {
        *ppAclDest = NULL;
    }

    if (pAcl)
    {
        VmAfdFreeAcl(pAcl);
    }

    goto cleanup;
}

static
DWORD
VmAfdCopyAceList(
                  PVMAFD_ACE_LIST pAceListSrc,
                  PVMAFD_ACE_LIST *ppAceListDest
                )
{
    DWORD dwError = 0;
    PVMAFD_ACE_LIST pAceListCursor = NULL;
    PVMAFD_ACE_LIST pAceListStart = NULL;
    PVMAFD_ACE_LIST pAceListDstCursor = NULL;
    PVMAFD_ACE_LIST pAceListDstPrev = NULL;



    if (
        !ppAceListDest
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    pAceListCursor = pAceListSrc;

    while (pAceListCursor)
    {
        dwError = VmAfdAllocateMemory(
                                  sizeof (VMAFD_ACE_LIST),
                                  (PVOID *) &pAceListDstCursor
                                  );
        BAIL_ON_VMAFD_ERROR (dwError);

        if (!pAceListStart)
        {
            pAceListStart = pAceListDstCursor;
        }

        if (pAceListDstPrev)
        {
            pAceListDstPrev->pNext = pAceListDstCursor;
        }

        pAceListDstCursor->Ace.type = pAceListCursor->Ace.type;
        pAceListDstCursor->Ace.changeStatus =
                                  pAceListCursor->Ace.changeStatus;
        pAceListDstCursor->Ace.accessMask =
                                  pAceListCursor->Ace.accessMask;

        dwError = VmAfdCopySecurityContext(
                                    pAceListCursor->Ace.pSecurityContext,
                                    &(pAceListDstCursor->Ace.pSecurityContext
                                     )
                                    );
        BAIL_ON_VMAFD_ERROR (dwError);

        pAceListDstPrev = pAceListDstCursor;

        pAceListCursor = pAceListCursor->pNext;
    }

    *ppAceListDest = pAceListStart;

cleanup:
    return dwError;

error:
    if (ppAceListDest)
    {
        *ppAceListDest = NULL;
    }

    if (pAceListStart)
    {
        VmAfdFreeAceList(pAceListStart);
    }

    goto cleanup;
}

static
DWORD
VmAfdCheckAcl (
          PVMAFD_SECURITY_DESCRIPTOR pSecurityDescriptor,
          PVM_AFD_SECURITY_CONTEXT pSecurityContext,
          DWORD dwDesiredAccess
          )
{
    DWORD dwError = 0;
    PVMAFD_ACL pAcl = NULL;
    DWORD dwGrantedAccess = 0;
    PVMAFD_ACE_LIST pAceListCursor = NULL;


    if (!pSecurityContext||
        !pSecurityDescriptor
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    pAcl = pSecurityDescriptor->pAcl;

    if (pAcl)
    {
        if (pAcl->dwAceCount == 0)
        {
            dwError = ERROR_ACCESS_DENIED;
            BAIL_ON_VMAFD_ERROR (dwError);
        }

        pAceListCursor = pAcl->pAceList;

        while (pAceListCursor)
        {

            if (VmAfdContextBelongsToGroup(
                          pSecurityContext,
                          pAceListCursor->Ace.pSecurityContext
                          )
                ||
                VmAfdEqualsSecurityContext(
                          pSecurityContext,
                          pAceListCursor->Ace.pSecurityContext
                          )
               )
            {
                    switch (pAceListCursor->Ace.type)
                    {
                        case VMAFD_ACE_TYPE_ALLOWED:
                                dwGrantedAccess = dwGrantedAccess |
                                                  pAceListCursor->Ace.accessMask;
                                 break;
                        case VMAFD_ACE_TYPE_DENIED:
                                if (dwDesiredAccess & pAceListCursor->Ace.accessMask)
                                {
                                    dwError = ERROR_ACCESS_DENIED;
                                }
                                break;
                        default:
                                dwError = ERROR_INVALID_PARAMETER;
                     }
                     BAIL_ON_VMAFD_ERROR (dwError);
            }

            /*if (VmAfdEqualsSecurityContext(
                          pSecurityContext,
                          pAceListCursor->Ace.pSecurityContext
                          )
               )
            {
                *dwGrantedAccess = dwGrantedAccess | (
                                  dwDesiredAccess &
                                  pAceListCursor->Ace.accessMask
                                  );*

                dwGrantedAccess = dwGrantedAccess | pAceListCursor->Ace.accessMask;
            }*/
            pAceListCursor = pAceListCursor->pNext;
        }

        if (dwGrantedAccess < dwDesiredAccess)
        {
            dwError = ERROR_ACCESS_DENIED;
            BAIL_ON_VMAFD_ERROR (dwError);
        }

        /*if (dwGrantedAccess != dwDesiredAccess)
        {
            dwError = ERROR_ACCESS_DENIED;
            BAIL_ON_VMAFD_ERROR (dwError);
        }*/
    }

cleanup:
    return dwError;

error:
    goto cleanup;
}
