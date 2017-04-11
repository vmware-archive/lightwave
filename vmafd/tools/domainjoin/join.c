/*
 * Copyright (C) 2015 VMware, Inc. All rights reserved.
 *
 * Module   : join.c
 *
 * Abstract :
 *
 */

#include "includes.h"

DWORD
VmAfdJoinDomain(
    PCSTR pszDomain,
    PCSTR pszUsername,
    PCSTR pszPassword,
    PCSTR pszOrgUnit
    )
{
    DWORD dwError = 0;
    VMAFD_JOIN_FLAGS dwFlags = 0;

#if defined(PLATFORM_VMWARE_ESX)

    dwFlags = (VMAFD_JOIN_FLAGS_ENABLE_NSSWITCH |
               VMAFD_JOIN_FLAGS_ENABLE_PAM |
               VMAFD_JOIN_FLAGS_ENABLE_SSH);

#endif

    if (!pszDomain)
    {
        fprintf(stderr, "Error: An invalid domain name was specified.\n");
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }
    if (!pszPassword)
    {
        fprintf(stderr, "Error: An invalid password was specified.\n");
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }
    if (!pszUsername)
    {
        pszUsername = "Administrator";
    }

#if !defined(PLATFORM_VMWARE_ESX)
    dwError = VMwDeployStartServiceClient();
    BAIL_ON_VMAFD_ERROR(dwError);
#endif

    dwError = VmAfdJoinVmDir2A(
                    pszDomain,
                    pszUsername,
                    pszPassword,
                    NULL, /* machine account */
                    pszOrgUnit,
                    dwFlags);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    return dwError;

error:

    goto cleanup;
}
