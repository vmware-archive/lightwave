/*
 * Copyright (C) 2015 VMware, Inc. All rights reserved.
 *
 * Module   : info.c
 *
 * Abstract :
 *
 */

#include "includes.h"

DWORD
VmAfdGetJoinStatus(
    PSTR*    ppszDomain,
    PBOOLEAN pbIsDC
    )
{
    DWORD dwError = ERROR_SUCCESS;
    VMAFD_STATUS status = {0};
    VMAFD_DOMAIN_STATE domainState = VMAFD_DOMAIN_STATE_NONE;
    BOOLEAN bIsDC = FALSE;
    PSTR pszDomain = NULL;

    dwError = VmAfdGetStatusA(NULL, &status);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdGetDomainStateA(NULL, &domainState);
    BAIL_ON_VMAFD_ERROR(dwError);

    switch (domainState)
    {
        case VMAFD_DOMAIN_STATE_CLIENT:

            break;

        case VMAFD_DOMAIN_STATE_CONTROLLER:

            bIsDC = TRUE;

            break;

        case VMAFD_DOMAIN_STATE_NONE:
        default:

            dwError = ERROR_NOT_JOINED;
            BAIL_ON_VMAFD_ERROR(dwError);

            break;
    }

    dwError = VmAfdGetDomainNameA(NULL, &pszDomain);
    BAIL_ON_VMAFD_ERROR(dwError);

    *ppszDomain = pszDomain;
    *pbIsDC = bIsDC;

cleanup:

    return dwError;

error:

    VMAFD_SAFE_FREE_MEMORY(pszDomain);

    goto cleanup;
}
