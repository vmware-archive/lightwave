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

VOID
VmAfdSrvSetStatus(
    VMAFD_STATUS state
    )
{
    pthread_mutex_lock(&gVmafdGlobals.mutex);

    gVmafdGlobals.status = state;
    pthread_cond_signal(&gVmafdGlobals.statusCond);

    pthread_mutex_unlock(&gVmafdGlobals.mutex);
}

VMAFD_STATUS
VmAfdSrvGetStatus(
    VOID
    )
{
    VMAFD_STATUS status = VMAFD_STATUS_UNKNOWN;

    pthread_mutex_lock(&gVmafdGlobals.mutex);

    status = gVmafdGlobals.status;

    pthread_mutex_unlock(&gVmafdGlobals.mutex);

    return status;
}

DWORD
VmAfdGetRegArgs(
    PVMAFD_REG_ARG *ppArgs
    )
{
    DWORD dwError = 0;
    PWSTR pwszAccountName = NULL;
    PWSTR pwszPassword = NULL;
    PWSTR pwszAccountDN = NULL;
    PWSTR pwszDomain = NULL;
    PWSTR pwszDCName = NULL;
    PWSTR pwszAccount = NULL;
    PVMAFD_REG_ARG pArgs = NULL;
    VMAFD_DOMAIN_STATE domainState = VMAFD_DOMAIN_STATE_NONE ;

    dwError = VmAfSrvGetDomainState(&domainState);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (domainState == VMAFD_DOMAIN_STATE_NONE)
    {
        dwError = ERROR_NOT_JOINED;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfSrvGetDCName(&pwszDCName);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfSrvGetMachineAccountInfo(
                    &pwszAccount,
                    &pwszPassword,
                    &pwszAccountDN,
                    NULL);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError =  VmAfdAllocateMemory(
                    sizeof(VMAFD_REG_ARG),
                    (PVOID*)&pArgs);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringAFromW(
                    pwszDCName,
                    &pArgs->pszDCName);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringAFromW(
                    pwszAccountDN,
                    &pArgs->pszAccountDN);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringAFromW(
                    pwszPassword,
                    &pArgs->pszPassword);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringAFromW(
                    pwszAccount,
                    &pArgs->pszAccount);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfSrvGetDomainName(&pwszDomain);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringAFromW(
                    pwszDomain,
                    &pArgs->pszDomain);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringPrintf( &(pArgs->pszAccountUPN), "%s@%s",
                                         pArgs->pszAccount, pArgs->pszDomain);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (IsNullOrEmptyString(pArgs->pszDCName))
    {
        dwError = VECS_MISSING_DC_NAME;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if (IsNullOrEmptyString(pArgs->pszAccountDN) ||
        IsNullOrEmptyString(pArgs->pszPassword))
    {
        dwError = VECS_MISSING_CREDS;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    *ppArgs = pArgs;

cleanup:

    VMAFD_SAFE_FREE_MEMORY(pwszAccountName);
    VMAFD_SAFE_FREE_MEMORY(pwszPassword);
    VMAFD_SAFE_FREE_MEMORY(pwszAccountDN);
    VMAFD_SAFE_FREE_MEMORY(pwszDomain);
    VMAFD_SAFE_FREE_MEMORY(pwszDCName);
    VMAFD_SAFE_FREE_MEMORY(pwszAccount);

    return dwError;

error :

    *ppArgs = NULL;

    if (pArgs)
    {
        VmAfdFreeRegArgs(pArgs);
    }

    switch (dwError)
    {
        case VECS_MISSING_CREDS:

            VmAfdLog(VMAFD_DEBUG_ANY, "Account DN / Password missing");

            break;

        case VECS_MISSING_DC_NAME:

            VmAfdLog(VMAFD_DEBUG_ANY, "Invalid domain controller name");

            break;

        default:

            VmAfdLog(
                VMAFD_DEBUG_ANY,
                "Error [%d] fetching registry args",
                dwError);

            break;
    }

    goto cleanup;
}

VOID
VmAfdFreeRegArgs(
    PVMAFD_REG_ARG pArgs
    )
{
	if ( pArgs )
	{
		VMAFD_SAFE_FREE_MEMORY(pArgs->pszAccount);
		VMAFD_SAFE_FREE_MEMORY(pArgs->pszAccountDN);
		VMAFD_SAFE_FREE_MEMORY(pArgs->pszPassword);
		VMAFD_SAFE_FREE_MEMORY(pArgs->pszDCName);
		VMAFD_SAFE_FREE_MEMORY(pArgs->pszDomain);
		VMAFD_SAFE_FREE_MEMORY(pArgs->pszAccountUPN);
		VmAfdFreeMemory(pArgs);
	}
}

BOOLEAN
VmAfdSrvIsValidGUID(
    PCWSTR pwszGUID
    )
{
    DWORD dwError = 0;
    BOOLEAN bResult = FALSE;
    PSTR    pszGUID = NULL;

    if (!IsNullOrEmptyString(pwszGUID))
    {
        unsigned32 status = 0;
        dce_uuid_t uuid;

        dwError = VmAfdAllocateStringAFromW(pwszGUID, &pszGUID);
        BAIL_ON_VMAFD_ERROR(dwError);

        dce_uuid_from_string(pszGUID, &uuid, &status);

        bResult =  (status == uuid_s_ok);
    }

error:

    VMAFD_SAFE_FREE_STRINGA(pszGUID);

    return bResult;
}
