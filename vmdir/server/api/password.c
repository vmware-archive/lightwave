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

static
DWORD
_VmDirResetActPassword(
    PVDIR_CONNECTION pConn,
    PSTR*            ppszNewPassword
    )
{
    DWORD dwError = 0;
    DWORD iCnt = 0;
    PSTR pszPassword = NULL;
    VDIR_BERVALUE bvPassword = VDIR_BERVALUE_INIT;
    PSTR pszDomainName = NULL;

    dwError = VmDirDomainDNToName(
                  pConn->AccessInfo.pszNormBindedDn,
                  &pszDomainName);
    BAIL_ON_VMDIR_ERROR(dwError);

    for (iCnt = 0; iCnt < VMDIR_MAX_PASSWORD_RETRIES; iCnt++)
    {
        VMDIR_SAFE_FREE_MEMORY(pszPassword);

        dwError = VmDirGenerateRandomInternalPassword(
                      pszDomainName,
                      &pszPassword);
        BAIL_ON_VMDIR_ERROR(dwError);

        bvPassword.lberbv_val = (PSTR)pszPassword;
        bvPassword.lberbv_len = VmDirStringLenA(pszPassword);

        dwError = VmDirExternalEntryAttributeReplace(
                      pConn,
                      pConn->AccessInfo.pszNormBindedDn,
                      ATTR_USER_PASSWORD,
                      &bvPassword);
        if (dwError == LDAP_CONSTRAINT_VIOLATION)
        {
            continue;
        }
        BAIL_ON_VMDIR_ERROR(dwError);

        break;
    }

    if (iCnt == VMDIR_MAX_PASSWORD_RETRIES)
    {
        dwError = LDAP_CONSTRAINT_VIOLATION;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    *ppszNewPassword = pszPassword;

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszDomainName);
    return dwError;

error:
    VMDIR_SAFE_FREE_MEMORY(pszPassword);
    goto cleanup;
}

/*
 * refresh password if required or if bRefreshPassword is set
 * if refreshed, new password is set to ppszNewPassword
*/
DWORD
VmDirRefreshPassword(
    PVDIR_CONNECTION pConn,
    BOOL             bRefreshPassword,
    PSTR             *ppszNewPassword
    )
{
    DWORD dwError = 0;
    PVDIR_OPERATION pSearchOp = NULL;
    PSTR  pszExpInDays = NULL;
    PSTR  pszLastChange = NULL;
    PSTR  pszPolicyDN = NULL;
    PSTR  pszNewPassword = NULL;
    PSTR  pszActDN = NULL;
    int   iExpInDays=0;

    if (!pConn ||
        IsNullOrEmptyString(pConn->AccessInfo.pszNormBindedDn) ||
        !ppszNewPassword)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    pszActDN = pConn->AccessInfo.pszNormBindedDn;

    dwError = VmDirExternalOperationCreate(
            NULL, -1, LDAP_REQ_SEARCH, pConn, &pSearchOp);
    BAIL_ON_VMDIR_ERROR(dwError);

    pSearchOp->protocol = VDIR_OPERATION_PROTOCOL_REST;

    if (!bRefreshPassword)
    {
        dwError = VmDirDNToPasswordPolicyDN(pszActDN, &pszPolicyDN);
        BAIL_ON_VMDIR_ERROR(dwError);


        // get policy expireInDays attribute
        dwError = VmDirDNCopySingleAttributeString(
                      pszPolicyDN,
                      ATTR_PASS_EXP_IN_DAY,
                      &pszExpInDays);
        if (!dwError)
        {
            iExpInDays = atoi(pszExpInDays);
        }
        else if (dwError == LDAP_NO_SUCH_ATTRIBUTE ||
                 dwError == VMDIR_ERROR_NO_SUCH_ATTRIBUTE)
        {
            dwError = 0;
        }
        BAIL_ON_VMDIR_ERROR(dwError);

        // iExpInDays == 0 or no such value means never expire
        if (iExpInDays > 0)
        {
            // get account last password change time
            dwError = VmDirDNCopySingleAttributeString(
                          pszActDN,
                          ATTR_PWD_LAST_SET,
                          &pszLastChange);
            BAIL_ON_VMDIR_ERROR(dwError);

            time_t  tNow = time(NULL);
            time_t  tPwdLastSet = 0;
            INT64   iPwdLastSet = 0;

            dwError = VmDirStringToINT64(pszLastChange, NULL, &iPwdLastSet);
            BAIL_ON_VMDIR_ERROR(dwError);

            /* Attempt reset when halfway to expiration. */
            time_t  tDiff = (iExpInDays * 24 * 60 * 60) / 2;
            tPwdLastSet = (time_t)iPwdLastSet;

            if ((tNow - tPwdLastSet) >= tDiff)
            {
                bRefreshPassword = TRUE;
            }
        }
    }

    if (bRefreshPassword)
    {
        dwError = _VmDirResetActPassword(
                       pConn,
                       &pszNewPassword);
        BAIL_ON_VMDIR_ERROR(dwError);

        VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, "Act (%s) password refreshed", pszActDN);
    }

    *ppszNewPassword = pszNewPassword;

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszLastChange);
    VMDIR_SAFE_FREE_MEMORY(pszExpInDays);
    VMDIR_SAFE_FREE_MEMORY(pszPolicyDN);
    VmDirFreeOperation(pSearchOp);

    return dwError;

error:

    VMDIR_SAFE_FREE_MEMORY(pszNewPassword);
    goto cleanup;
}
