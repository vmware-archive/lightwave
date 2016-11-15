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



#include "includes.h"

/*
 * Initialize a VDIR_OPERTION structure
 * 1. opType :      OPERATION TYPE
 * 2. pSchemaCtx:   SCHEMA CONTEXT
 */
DWORD
VmDirInitStackOperation(
    PVDIR_OPERATION         pOp,
    VDIR_OPERATION_TYPE     opType,
    ber_tag_t               requestCode,
    PVDIR_SCHEMA_CTX        pSchemaCtx
    )
{
    DWORD               dwError = 0;
    PVDIR_SCHEMA_CTX    pLocalSchemaCtx = NULL;

    BAIL_ON_VMDIR_INVALID_POINTER( pOp, dwError );

    memset( pOp, 0, sizeof( *pOp ) );

    pOp->opType  = opType;
    pOp->reqCode = requestCode;

    if ( pOp->reqCode == LDAP_REQ_ADD )
    {
        dwError = VmDirAllocateMemory( sizeof(*(pOp->request.addReq.pEntry)),
                                       (PVOID)&(pOp->request.addReq.pEntry) );
        BAIL_ON_VMDIR_ERROR( dwError );
    }

    if ( pSchemaCtx )
    {   // TODO, dwError = VmDirSchemaCtxClone( pSchemaCtx, &pLocalCtx );
        pLocalSchemaCtx = VmDirSchemaCtxClone( pSchemaCtx );
        if ( !pLocalSchemaCtx )
        {
            dwError = ERROR_NO_SCHEMA;
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }
    else
    {
        dwError = VmDirSchemaCtxAcquire(&pLocalSchemaCtx);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateMemory( sizeof( *pOp->pBECtx ), (PVOID) &(pOp->pBECtx) );
    BAIL_ON_VMDIR_ERROR(dwError);

    if ( pOp->opType == VDIR_OPERATION_TYPE_INTERNAL )
    {   // needs dummy conn->VDIR_ACCESS_INFO for ACL check
        dwError = VmDirAllocateMemory( sizeof( *pOp->conn), (PVOID) &(pOp->conn) );
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    pOp->pSchemaCtx = pLocalSchemaCtx;
    pLocalSchemaCtx = NULL;

cleanup:

   return dwError;

error:

    if ( pLocalSchemaCtx )
    {
        VmDirSchemaCtxRelease( pLocalSchemaCtx );
    }

   goto cleanup;
}

int
VmDirExternalOperationCreate(
    BerElement*       ber,
    ber_int_t         msgId,
    ber_tag_t         reqCode,
    PVDIR_CONNECTION  pConn,
    PVDIR_OPERATION*  ppOperation
    )
{
   int retVal = 0;
   PVDIR_OPERATION pOperation = NULL;

   VmDirLog( LDAP_DEBUG_TRACE, "NewOperation: Begin" );

   retVal = VmDirAllocateMemory( sizeof(*pOperation), (PVOID *)&pOperation );
   BAIL_ON_VMDIR_ERROR( retVal );

   pOperation->protocol = 0;
   pOperation->reqCode = reqCode;
   pOperation->ber = ber;
   pOperation->msgId = msgId;
   pOperation->conn = pConn;
   pOperation->opType = VDIR_OPERATION_TYPE_EXTERNAL;

   retVal = VmDirAllocateMemory( sizeof(*pOperation->pBECtx), (PVOID)&(pOperation->pBECtx));
   BAIL_ON_VMDIR_ERROR( retVal );

   retVal = VmDirSchemaCtxAcquire(&pOperation->pSchemaCtx);
   BAIL_ON_VMDIR_ERROR( retVal );

   pOperation->lowestPendingUncommittedUsn = 0;

   if ( pOperation->reqCode == LDAP_REQ_ADD )
   {
       retVal = VmDirAllocateMemory( sizeof(*(pOperation->request.addReq.pEntry)),
                                     (PVOID)&(pOperation->request.addReq.pEntry) );
       BAIL_ON_VMDIR_ERROR( retVal );
   }

   *ppOperation = pOperation;

cleanup:
   VmDirLog( LDAP_DEBUG_TRACE, "%s: End", __FUNCTION__ );

   return retVal;

error:
   VmDirLog( LDAP_DEBUG_TRACE, "%s: acquire schema context failed",
           __FUNCTION__ );

   VmDirFreeOperation(pOperation);
   *ppOperation = NULL;

   goto cleanup;
}

void
VmDirFreeOperation(
    PVDIR_OPERATION pOperation
    )
{
    if (pOperation)
    {
        VmDirFreeOperationContent(pOperation);
        VMDIR_SAFE_FREE_MEMORY(pOperation);
    }
}

void
VmDirFreeOperationContent(
    PVDIR_OPERATION op
    )
{
    DWORD dwError = ERROR_SUCCESS;

    if (op)
    {
        if (op->pSchemaCtx)
        {
            VmDirSchemaCtxRelease(op->pSchemaCtx);
        }

        if (op->reqControls)
        {
            DeleteControls(&(op->reqControls));
        }

        if (op->syncDoneCtrl)
        {
            PLW_HASHTABLE_NODE      pNode = NULL;
            LW_HASHTABLE_ITER       iter = LW_HASHTABLE_ITER_INIT;
            UptoDateVectorEntry *   pUtdVectorEntry = NULL;
            SyncDoneControlValue *  syncDoneCtrlVal = &op->syncDoneCtrl->value.syncDoneCtrlVal;

            while ((pNode = LwRtlHashTableIterate(syncDoneCtrlVal->htUtdVector, &iter)))
            {
                dwError = LwRtlHashTableRemove(syncDoneCtrlVal->htUtdVector, pNode);
                assert( dwError == 0 );
                pUtdVectorEntry = LW_STRUCT_FROM_FIELD(pNode, UptoDateVectorEntry, Node);
                VmDirFreeBervalContent( &pUtdVectorEntry->invocationId );
                VMDIR_SAFE_FREE_MEMORY( pUtdVectorEntry );
            }
            LwRtlFreeHashTable(&syncDoneCtrlVal->htUtdVector);
            assert( syncDoneCtrlVal->htUtdVector == NULL );

            VMDIR_SAFE_FREE_MEMORY( op->syncDoneCtrl );
        }

        switch (op->reqCode)
        {
            case LDAP_REQ_BIND:
                 VmDirFreeBindRequest(&op->request.bindReq, FALSE);
                 if (op->ldapResult.replyInfo.type == REP_SASL)
                 {
                     VmDirFreeBervalContent( &(op->ldapResult.replyInfo.replyData.bvSaslReply) );
                 }
                 break;

            case LDAP_REQ_ADD:
                 VmDirFreeAddRequest(&op->request.addReq, FALSE);
                 break;

            case LDAP_REQ_SEARCH:
                 VmDirFreeSearchRequest(&op->request.searchReq, FALSE);
                 break;

            case LDAP_REQ_MODIFY:
            case LDAP_REQ_MODDN:
                 VmDirFreeModifyRequest(&op->request.modifyReq, FALSE);
                 break;

            case LDAP_REQ_DELETE:
                 VmDirFreeDeleteRequest(&op->request.deleteReq, FALSE);
                 break;

            default:
                 break;
        }

        VmDirFreeEntryArrayContent(&(op->internalSearchEntryArray));
        VmDirFreeBervalContent( &(op->reqDn) );
        VMDIR_SAFE_FREE_MEMORY(op->ldapResult.pszErrMsg);
        VmDirBackendCtxFree(op->pBECtx);
        VMDIR_SAFE_FREE_MEMORY(op->pszFilters);

        if ( op->opType == VDIR_OPERATION_TYPE_INTERNAL )
        {   // internal op owns dummy conn for ACL check
            VmDirDeleteConnection( &(op->conn)); // passing &conn to be freed seems a bit strange
        }
   }
}

//Clone a operation struct that share the same
//transaction context with pOp
DWORD
VmDirCloneStackOperation(
    PVDIR_OPERATION         pOp,
    PVDIR_OPERATION         pOutOp,
    VDIR_OPERATION_TYPE     opType,
    ber_tag_t               requestCode,
    PVDIR_SCHEMA_CTX        pSchemaCtx
    )
{
    DWORD               dwError = 0;
    PVDIR_SCHEMA_CTX    pLocalSchemaCtx = NULL;

    BAIL_ON_VMDIR_INVALID_POINTER( pOp, dwError );
    BAIL_ON_VMDIR_INVALID_POINTER( pOutOp, dwError );

    pOutOp->opType  = opType;
    pOutOp->reqCode = requestCode;

    if ( pOutOp->reqCode == LDAP_REQ_ADD )
    {
        dwError = VmDirAllocateMemory( sizeof(*(pOutOp->request.addReq.pEntry)),
                                       (PVOID)&(pOutOp->request.addReq.pEntry) );
        BAIL_ON_VMDIR_ERROR( dwError );
    }

    if ( pSchemaCtx )
    {
        pLocalSchemaCtx = VmDirSchemaCtxClone( pSchemaCtx );
        if ( !pLocalSchemaCtx )
        {
            dwError = ERROR_NO_SCHEMA;
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }
    else
    {
        dwError = VmDirSchemaCtxAcquire(&pLocalSchemaCtx);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateAndCopyMemory(pOp->pBECtx, sizeof(*pOutOp->pBECtx ), (PVOID) &(pOutOp->pBECtx));
    BAIL_ON_VMDIR_ERROR(dwError);

    if ( pOutOp->opType == VDIR_OPERATION_TYPE_INTERNAL )
    {   // needs dummy conn->VDIR_ACCESS_INFO for ACL check
        dwError = VmDirAllocateMemory( sizeof( *pOutOp->conn), (PVOID) &(pOutOp->conn) );
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    pOutOp->pSchemaCtx = pLocalSchemaCtx;
    pLocalSchemaCtx = NULL;

cleanup:

   return dwError;

error:

    if ( pLocalSchemaCtx )
    {
        VmDirSchemaCtxRelease( pLocalSchemaCtx );
    }
    VmDirFreeOperationContent(pOutOp);

   goto cleanup;
}
