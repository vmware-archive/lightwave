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
REST_MODULE _ldap_rest_module[] =
{
    {
        "/v1/lwraft/ldap",
        {VmDirRESTLdapSearch, VmDirRESTLdapAdd, NULL, VmDirRESTLdapDelete, VmDirRESTLdapModify}
    },
    {
        "/v1/lwraft/object/*",
        {VmDirRESTLdapSearch, VmDirRESTLdapAdd, NULL, VmDirRESTLdapDelete, VmDirRESTLdapModify}
    },
    {0}
};

DWORD
VmDirRESTGetLdapModule(
    PREST_MODULE*   ppRestModule
    )
{
    *ppRestModule = _ldap_rest_module;
    return 0;
}

/*
 * Performs Add operation. Input JSON data should be there in
 * pHttp->pszInputJson before calling this.
 * Only one entry is allowed to add per call.
 */
DWORD
VmDirRESTLdapAdd(
    void*   pIn,
    void**  ppOut
    )
{
    DWORD   dwError = 0;
    PVDIR_ENTRY pEntry = NULL;
    PVDIR_REST_OPERATION    pRestOp = NULL;
    PVDIR_OPERATION         pAddOp = NULL;

    if (!pIn)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    pRestOp = (PVDIR_REST_OPERATION)pIn;

    dwError = VmDirExternalOperationCreate(
            NULL, -1, LDAP_REQ_ADD, pRestOp->pConn, &pAddOp);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirRESTDecodeEntry(pRestOp, &pEntry);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirResetAddRequestEntry(pAddOp, pEntry);
    BAIL_ON_VMDIR_ERROR(dwError);
    pEntry = NULL;

    dwError = VmDirMLAdd(pAddOp);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VMDIR_SET_REST_RESULT(pRestOp, pAddOp, dwError, NULL);
    VmDirFreeOperation(pAddOp);
    VmDirFreeEntry(pEntry);
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)", __FUNCTION__, dwError);
    goto cleanup;
}

/*
 * Performs Search operation.
 * Only entries for which user has access right will be returned.
 */
DWORD
VmDirRESTLdapSearch(
    void*   pIn,
    void**  ppOut
    )
{
    DWORD   dwError = 0;
    PSTR    pszDN = NULL;
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

    dwError = VmDirRESTGetLdapSearchParams(
            pRestOp,
            &pszDN,
            &pSearchOp->request.searchReq.scope,
            &pSearchOp->request.searchReq.filter,
            &pSearchOp->request.searchReq.attrs,
            &pPagedResultsCtrl);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirStringToBervalContent(pszDN, &pSearchOp->reqDn);
    BAIL_ON_VMDIR_ERROR(dwError);

    pSearchOp->showPagedResultsCtrl = pPagedResultsCtrl;
    pSearchOp->request.searchReq.bStoreRsltInMem = TRUE;

    dwError = VmDirMLSearch(pSearchOp);
    BAIL_ON_VMDIR_ERROR(dwError);

    // set operation result
    dwError = VmDirRESTEncodeEntryArray(
            pRestOp,
            &pSearchOp->internalSearchEntryArray,
            pSearchOp->request.searchReq.attrs,
            &pjResult);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirRESTResultSetObjData(pRestOp->pResult, "result", pjResult);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirRESTResultSetIntData(
            pRestOp->pResult,
            "result_count",
            pSearchOp->internalSearchEntryArray.iSize);
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
    VMDIR_SAFE_FREE_MEMORY(pszDN);
    VMDIR_SAFE_FREE_MEMORY(pPagedResultsCtrl);
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

/*
 * Performs Modify operation
 */
DWORD
VmDirRESTLdapModify(
    void*   pIn,
    void**  ppOut
    )
{
    DWORD   dwError = 0;
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

    switch (pRestOp->pResource->rscType)
    {
        case VDIR_REST_RSC_LDAP:
                dwError = VmDirRESTGetStrParam(pRestOp, "dn", &pszDN, TRUE);
                BAIL_ON_VMDIR_ERROR(dwError)
                break;

        case VDIR_REST_RSC_OBJECT:
                dwError = VmDirRESTEndpointToDN(pRestOp, &pszDN);
                BAIL_ON_VMDIR_ERROR(dwError)
                break;

        default: BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_REQUEST);
    }

    dwError = VmDirStringToBervalContent(pszDN, &pModifyOp->reqDn);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirStringToBervalContent(pszDN, &pModifyOp->request.modifyReq.dn);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirRESTDecodeMods(
            pRestOp->pjInput,
            &pModifyOp->request.modifyReq.mods,
            &pModifyOp->request.modifyReq.numMods);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirMLModify(pModifyOp);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VMDIR_SET_REST_RESULT(pRestOp, pModifyOp, dwError, NULL);
    VMDIR_SAFE_FREE_MEMORY(pszDN);
    VmDirFreeOperation(pModifyOp);
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)", __FUNCTION__, dwError);
    goto cleanup;
}

/*
 * Performs Delete operation
 */
DWORD
VmDirRESTLdapDelete(
    void*   pIn,
    void**  ppOut
    )
{
    DWORD   dwError = 0;
    PSTR    pszDN = NULL;
    PVDIR_REST_OPERATION    pRestOp = NULL;
    PVDIR_OPERATION         pDeleteOp = NULL;

    if (!pIn)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    pRestOp = (PVDIR_REST_OPERATION)pIn;

    dwError = VmDirExternalOperationCreate(
            NULL, -1, LDAP_REQ_DELETE, pRestOp->pConn, &pDeleteOp);
    BAIL_ON_VMDIR_ERROR(dwError);

    switch (pRestOp->pResource->rscType)
    {
        case VDIR_REST_RSC_LDAP:
                dwError = VmDirRESTGetStrParam(pRestOp, "dn", &pszDN, TRUE);
                BAIL_ON_VMDIR_ERROR(dwError);
                break;

        case VDIR_REST_RSC_OBJECT:
                dwError = VmDirRESTEndpointToDN(pRestOp, &pszDN);
                BAIL_ON_VMDIR_ERROR(dwError);
                break;

        default: BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_REQUEST);
    }

    dwError = VmDirStringToBervalContent(pszDN, &pDeleteOp->reqDn);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirStringToBervalContent(pszDN, &pDeleteOp->request.deleteReq.dn);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirMLDelete(pDeleteOp);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VMDIR_SET_REST_RESULT(pRestOp, pDeleteOp, dwError, NULL);
    VMDIR_SAFE_FREE_MEMORY(pszDN);
    VmDirFreeOperation(pDeleteOp);
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)", __FUNCTION__, dwError);
    goto cleanup;
}

DWORD
VmDirRESTLdapSetResult(
    PVDIR_REST_RESULT   pRestRslt,
    PVDIR_LDAP_RESULT   pLdapRslt,
    DWORD               dwErr,
    PSTR                pszErrMsg
    )
{
    DWORD   dwError = 0;
    int     err = 0;
    PSTR    msg = NULL;

    if (!pRestRslt)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (pLdapRslt)
    {
        err = pLdapRslt->errCode;
        msg = pLdapRslt->pszErrMsg;
    }
    else
    {
        err = VmDirToLDAPError(dwErr);
        msg = pszErrMsg;
    }

    if (IsNullOrEmptyString(msg))
    {
        dwError = VmDirRESTResultSetError(
                pRestRslt, err, ldap_err2string(err));
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    else
    {
        dwError = VmDirRESTResultSetError(
                pRestRslt, err, msg);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)", __FUNCTION__, dwError);
    goto cleanup;
}

DWORD
VmDirRESTLdapGetHttpError(
    PVDIR_REST_RESULT   pRestRslt,
    PSTR*               ppszHttpStatus,
    PSTR*               ppszHttpReason
    )
{
    DWORD   dwError = 0;
    int     httpStatus = 0;
    PVDIR_HTTP_ERROR    pHttpError = NULL;

    if (!pRestRslt || !ppszHttpStatus || !ppszHttpReason)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    switch (pRestRslt->errCode)
    {
    case LDAP_SUCCESS:
        httpStatus = HTTP_OK;
        break;

    case LDAP_UNAVAILABLE:
    case LDAP_SERVER_DOWN:
        httpStatus = HTTP_SERVICE_UNAVAILABLE;
        break;

    case LDAP_UNWILLING_TO_PERFORM:
    case LDAP_INVALID_DN_SYNTAX:
    case LDAP_NO_SUCH_ATTRIBUTE:
    case LDAP_INVALID_SYNTAX:
    case LDAP_UNDEFINED_TYPE:
    case LDAP_TYPE_OR_VALUE_EXISTS:
    case LDAP_OBJECT_CLASS_VIOLATION:
    case LDAP_ALREADY_EXISTS:
    case LDAP_NO_SUCH_OBJECT:
    case LDAP_CONSTRAINT_VIOLATION:
    case LDAP_NOT_ALLOWED_ON_NONLEAF:
    case LDAP_PROTOCOL_ERROR:
        httpStatus = HTTP_BAD_REQUEST;
        break;

    case LDAP_INVALID_CREDENTIALS:
    case LDAP_INSUFFICIENT_ACCESS:
    case LDAP_AUTH_METHOD_NOT_SUPPORTED:
    case LDAP_SASL_BIND_IN_PROGRESS:
        httpStatus = HTTP_UNAUTHORIZED;
        break;

    case LDAP_TIMELIMIT_EXCEEDED:
        httpStatus = HTTP_REQUEST_TIMEOUT;
        break;

    case LDAP_SIZELIMIT_EXCEEDED:
        httpStatus = HTTP_PAYLOAD_TOO_LARGE;
        break;

    default:
        httpStatus = HTTP_INTERNAL_SERVER_ERROR;
        break;
    }

    pHttpError = VmDirRESTGetHttpError(httpStatus);

    *ppszHttpStatus = pHttpError->pszHttpStatus;
    *ppszHttpReason = pHttpError->pszHttpReason;

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)", __FUNCTION__, dwError );
    goto cleanup;
}
