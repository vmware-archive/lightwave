/*
 * Copyright © 2018 VMware, Inc.  All Rights Reserved.
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

REST_MODULE _api_password_rest_module[] =
{
    {
        "/v1/vmdir/api/password/refresh",
        {NULL, NULL, VmDirRESTRefreshPassword, NULL, NULL}
    },
    {0}
};

DWORD
VmDirRESTApiGetPasswordModule(
    PREST_MODULE*   ppRestModule
    )
{
    *ppRestModule = _api_password_rest_module;
    return 0;
}

/*
 * refresh account password
 * returns:
 * if force is not specified and password is not aged
 *  {"refreshed":"false"}
 * if force was specified or password aged
 *  {"refreshed":"true","password":"..."}
*/
DWORD
VmDirRESTRefreshPassword(
    void*   pIn,
    void**  ppOut
    )
{
    DWORD dwError = 0;
    PVDIR_REST_OPERATION pRestOp = NULL;
    BOOLEAN bRefreshPassword = FALSE;
    PSTR pszNewPassword = NULL;
    BOOLEAN bRefreshed = FALSE;

    if (!pIn)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    pRestOp = (PVDIR_REST_OPERATION)pIn;

    dwError = VmDirRESTGetBoolParam(
                  pRestOp,
                  "force",
                  &bRefreshPassword,
                  TRUE);
    BAIL_ON_VMDIR_ERROR(dwError)

    dwError = VmDirRefreshPassword(
                  pRestOp->pConn,
                  bRefreshPassword,
                  &pszNewPassword);
    BAIL_ON_VMDIR_ERROR(dwError);

    VMDIR_LOG_DEBUG(
            VMDIR_LOG_MASK_ALL,
            "%s: force: %d, %s",
            __FUNCTION__, bRefreshPassword,
            pRestOp->pConn->AccessInfo.pszNormBindedDn);

    if (!IsNullOrEmptyString(pszNewPassword))
    {
        bRefreshed = TRUE;
        dwError = VmDirRESTResultSetStrData(
                      pRestOp->pResult,
                      "password",
                      pszNewPassword);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirRESTResultSetBooleanData(
                  pRestOp->pResult,
                  "refreshed",
                  bRefreshed);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VMDIR_SET_REST_RESULT(pRestOp, NULL, dwError, NULL);
    VMDIR_SAFE_FREE_MEMORY(pszNewPassword);
    return dwError;

error:
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)",
            __FUNCTION__,
            dwError);

    goto cleanup;
}
