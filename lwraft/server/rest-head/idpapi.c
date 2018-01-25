/*
 * Copyright © 218 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the ?~@~\License?~@~]); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an ?~@~\AS IS?~@~] BASIS, without
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
static REST_MODULE _idp_rest_module[] =
{
    {
        VMDIR_V1_RESOURCE_IDP,
        {VmDirRESTGetIDP, NULL, NULL, NULL, NULL}
    }
};

DWORD
VmDirRESTGetIDPModule(
    PREST_MODULE*   ppRestModule
    )
{
    *ppRestModule = _idp_rest_module;
    return 0;
}

/*
 * Retrieves the Identity provider for this node.
 * This is used to determine the STS login endpoint.
 */
DWORD
VmDirRESTGetIDP(
    void*   pRestOperation,
    void**  ppOutput
    )
{
    DWORD   dwError = 0;
    PSTR    pszDCName = NULL;
    PSTR    pszDomainName = NULL;
    char    pszClientId[VMDIR_MAX_OIDC_CLIENTID_LEN] = {0};
    PSTR    pszRedirectUrl = NULL;
    PSTR    pszAuthorizeUrl = NULL;
    char    pszLocalHostName[VMDIR_MAX_HOSTNAME_LEN] = {0};
    PSTR    pszFQDN = NULL;

    PVDIR_REST_OPERATION    pRestOp = NULL;

    if (!pRestOperation)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    pRestOp = (PVDIR_REST_OPERATION)pRestOperation;

    dwError = VmDirRESTGetDCName(&pszDCName);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirRESTGetDomainName(&pszDomainName);
    BAIL_ON_VMDIR_ERROR(dwError);

    //retrieve the client id
    dwError = VmDirReadStringFromFile(VMDIR_REST_CONF_FILE_PATH, pszClientId, sizeof(pszClientId));
    BAIL_ON_VMDIR_ERROR(dwError);

    // Get local host name
    dwError = VmDirGetHostName(pszLocalHostName, sizeof(pszLocalHostName)-1);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringPrintf(&pszFQDN, "%s.%s", pszLocalHostName, pszDomainName);
    BAIL_ON_VMDIR_ERROR(dwError);

    // construct the login endpoint URL
    dwError = VmDirAllocateStringPrintf(&pszRedirectUrl, "https://%s/ui",pszFQDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringPrintf(&pszAuthorizeUrl, "https://%s/openidconnect/oidc/authorize/%s"
                "?response_type=id_token%%20token"
                "&response_mode=fragment&client_id=%s&redirect_uri=%s"
                "&state=_state_lmn_&nonce=_nonce_lmn_"
                "&scope=openid%%20rs_admin_server%%20rs_post",
                pszDCName,
                pszDomainName,
                pszClientId,
                pszRedirectUrl);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirRESTResultSetStrData(pRestOp->pResult, "result", pszAuthorizeUrl);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirRESTResultSetStrData(pRestOp->pResult, "idp_host", pszDCName);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirRESTResultSetIntData(
            pRestOp->pResult,
            "result_count",
            1);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VMDIR_SET_REST_RESULT(pRestOp, NULL, dwError, NULL);
    VMDIR_SAFE_FREE_MEMORY(pszDomainName);
    VMDIR_SAFE_FREE_MEMORY(pszDCName);
    VMDIR_SAFE_FREE_MEMORY(pszFQDN);
    VMDIR_SAFE_FREE_MEMORY(pszRedirectUrl);
    VMDIR_SAFE_FREE_MEMORY(pszAuthorizeUrl);
    return dwError;

error:
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)",
            __FUNCTION__,
            dwError);

    goto cleanup;
}
