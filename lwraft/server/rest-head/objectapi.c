/*
 * Copyright © 2017 VMware, Inc.  All Rights Reserved.
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
 * REST_MODULE (from copenapitypes.h)
 * callback indices must correspond to:
 *      GET, PUT, POST, DELETE, PATCH
 */
REST_MODULE _object_rest_module[] =
{
    {
        "/v1/lwraft/object/*",
        {VmDirRESTObjectGet, VmDirRESTObjectPut, NULL, VmDirRESTObjectDelete, VmDirRESTObjectPatch}
    }
};

DWORD
VmDirRESTGetObjectModule(
    PREST_MODULE*   ppRestModule
    )
{
    *ppRestModule = _object_rest_module;
    return 0;
}

DWORD
VmDirRESTObjectPut(
    void*   pIn,
    void**  ppOut
    )
{
    DWORD   dwError = 0;
    PSTR    pszTenant = NULL;
    PVDIR_ENTRY pObj = NULL;
    PVDIR_REST_OPERATION    pRestOp = NULL;
    PVDIR_OPERATION         pAddOp = NULL;

    if (!pIn)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    pRestOp = (PVDIR_REST_OPERATION)pIn;

    // put request must have subpath (=objectpath)
    if (IsNullOrEmptyString(pRestOp->pszSubPath))
    {
        dwError = VMDIR_ERROR_INVALID_REQUEST;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirExternalOperationCreate(
            NULL, -1, LDAP_REQ_ADD, pRestOp->pConn, &pAddOp);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirRESTGetObjectTenantParam(pRestOp, &pszTenant);
    BAIL_ON_VMDIR_ERROR(dwError)

    dwError = VmDirRESTDecodeObject(
            pRestOp->pjInput, pRestOp->pszSubPath, pszTenant, &pObj);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirResetAddRequestEntry(pAddOp, pObj);
    BAIL_ON_VMDIR_ERROR(dwError);
    pObj = NULL;

    dwError = VmDirMLAdd(pAddOp);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VMDIR_SET_REST_RESULT(pRestOp, pAddOp, dwError, NULL);
    VMDIR_SAFE_FREE_MEMORY(pszTenant);
    VmDirFreeOperation(pAddOp);
    VmDirFreeEntry(pObj);
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)", __FUNCTION__, dwError);
    goto cleanup;
}

DWORD
VmDirRESTObjectGet(
    void*   pIn,
    void**  ppOut
    )
{
    DWORD   dwError = 0;
    PSTR    pszTenant = NULL;
    PSTR    pszDN = NULL;
    size_t  skipped = 0;
    PVDIR_LDAP_CONTROL  pPagedResultsCtrl = NULL;
    json_t*             pjResult = NULL;
    PVDIR_REST_OPERATION    pRestOp = NULL;
    PVDIR_OPERATION         pSearchOp = NULL;

    if (!pIn)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    pRestOp = (PVDIR_REST_OPERATION)pIn;

    dwError = VmDirExternalOperationCreate(
            NULL, -1, LDAP_REQ_SEARCH, pRestOp->pConn, &pSearchOp);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirRESTGetObjectGetParams(
            pRestOp,
            &pszTenant,
            &pSearchOp->request.searchReq.scope,
            &pSearchOp->request.searchReq.filter,
            &pSearchOp->request.searchReq.attrs,
            &pPagedResultsCtrl);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirRESTDecodeObjectPathToDN(
            pRestOp->pszSubPath, pszTenant, &pszDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirStringToBervalContent(pszDN, &pSearchOp->reqDn);
    BAIL_ON_VMDIR_ERROR(dwError);

    pSearchOp->showPagedResultsCtrl = pPagedResultsCtrl;
    pSearchOp->request.searchReq.bStoreRsltInMem = TRUE;

    dwError = VmDirMLSearch(pSearchOp);
    BAIL_ON_VMDIR_ERROR(dwError);

    // set operation result
    dwError = VmDirRESTEncodeObjectArray(
            &pSearchOp->internalSearchEntryArray,
            pSearchOp->request.searchReq.attrs,
            pszTenant,
            &pjResult,
            &skipped);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirRESTResultSetObjData(pRestOp->pResult, "result", pjResult);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirRESTResultSetIntData(
            pRestOp->pResult,
            "result_count",
            pSearchOp->internalSearchEntryArray.iSize - skipped);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (pPagedResultsCtrl)
    {
        VDIR_PAGED_RESULT_CONTROL_VALUE* pCtrl =
                &pPagedResultsCtrl->value.pagedResultCtrlVal;

        if (!IsNullOrEmptyString(pCtrl->cookie))
        {
            dwError = VmDirRESTResultSetStrData(
                    pRestOp->pResult, "paged_results_cookie", pCtrl->cookie);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }

cleanup:
    VMDIR_SET_REST_RESULT(pRestOp, pSearchOp, dwError, NULL);
    VMDIR_SAFE_FREE_MEMORY(pPagedResultsCtrl);
    VMDIR_SAFE_FREE_MEMORY(pszTenant);
    VMDIR_SAFE_FREE_MEMORY(pszDN);
    VmDirFreeOperation(pSearchOp);
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)", __FUNCTION__, dwError);

    if (pjResult)
    {
        json_decref(pjResult);
    }
    goto cleanup;
}

DWORD
VmDirRESTObjectPatch(
    void*   pIn,
    void**  ppOut
    )
{
    DWORD   dwError = 0;
    PSTR    pszTenant = NULL;
    PSTR    pszDN = NULL;
    PVDIR_REST_OPERATION    pRestOp = NULL;
    PVDIR_OPERATION         pModifyOp = NULL;

    if (!pIn)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    pRestOp = (PVDIR_REST_OPERATION)pIn;

    dwError = VmDirExternalOperationCreate(
            NULL, -1, LDAP_REQ_MODIFY, pRestOp->pConn, &pModifyOp);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirRESTGetObjectTenantParam(pRestOp, &pszTenant);
    BAIL_ON_VMDIR_ERROR(dwError)

    dwError = VmDirRESTDecodeObjectPathToDN(
            pRestOp->pszSubPath, pszTenant, &pszDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirStringToBervalContent(pszDN, &pModifyOp->reqDn);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirStringToBervalContent(pszDN, &pModifyOp->request.modifyReq.dn);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirRESTDecodeObjectMods(
            pRestOp->pjInput,
            pszTenant,
            &pModifyOp->request.modifyReq.mods,
            &pModifyOp->request.modifyReq.numMods);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirMLModify(pModifyOp);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VMDIR_SET_REST_RESULT(pRestOp, pModifyOp, dwError, NULL);
    VMDIR_SAFE_FREE_MEMORY(pszTenant);
    VMDIR_SAFE_FREE_MEMORY(pszDN);
    VmDirFreeOperation(pModifyOp);
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)", __FUNCTION__, dwError);
    goto cleanup;
}

DWORD
VmDirRESTObjectDelete(
    void*   pIn,
    void**  ppOut
    )
{
    DWORD   dwError = 0;
    PSTR    pszTenant = NULL;
    PSTR    pszDN = NULL;
    BOOLEAN bRecursive = FALSE;
    PVDIR_REST_OPERATION    pRestOp = NULL;
    PVDIR_OPERATION         pDeleteOp = NULL;

    if (!pIn)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    pRestOp = (PVDIR_REST_OPERATION)pIn;

    dwError = VmDirRESTGetBoolParam(pRestOp, "recursive", &bRecursive, FALSE);
    BAIL_ON_VMDIR_ERROR(dwError);

    // TODO implement recursive option
    dwError = bRecursive ? VMDIR_ERROR_UNWILLING_TO_PERFORM : 0;
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirExternalOperationCreate(
            NULL, -1, LDAP_REQ_DELETE, pRestOp->pConn, &pDeleteOp);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirRESTGetObjectTenantParam(pRestOp, &pszTenant);
    BAIL_ON_VMDIR_ERROR(dwError)

    dwError = VmDirRESTDecodeObjectPathToDN(
            pRestOp->pszSubPath, pszTenant, &pszDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirStringToBervalContent(pszDN, &pDeleteOp->reqDn);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirStringToBervalContent(pszDN, &pDeleteOp->request.deleteReq.dn);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirMLDelete(pDeleteOp);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VMDIR_SET_REST_RESULT(pRestOp, pDeleteOp, dwError, NULL);
    VMDIR_SAFE_FREE_MEMORY(pszTenant);
    VMDIR_SAFE_FREE_MEMORY(pszDN);
    VmDirFreeOperation(pDeleteOp);
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)", __FUNCTION__, dwError);
    goto cleanup;
}
