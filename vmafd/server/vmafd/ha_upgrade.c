/*
 * Copyright (C) 2015 VMware, Inc. All rights reserved.
 *
 * Module   : heartbeat.c
 *
 * Abstract :
 *
 */
#include "includes.h"

DWORD
VmAfSrvBeginUpgrade()
{
    DWORD dwError = 0;
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdDbSetUpgradeState(TRUE);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:
    return dwError;
error:
    goto cleanup;
}

DWORD
VmAfSrvEndUpgrade()
{
    DWORD dwError = 0;
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdDbSetUpgradeState(FALSE);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:
    return dwError;
error:
    goto cleanup;
}

DWORD
VmAfSrvGetUpgradeState(
    PBOOL pbIsUpgradeInProgress
    )
{
    DWORD dwError = 0;
    BOOL bIsUpgradeInProgress = FALSE;

    if (!pbIsUpgradeInProgress)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdDbIsUpgradeInProgress(&bIsUpgradeInProgress);
    BAIL_ON_VMAFD_ERROR(dwError);

    *pbIsUpgradeInProgress = bIsUpgradeInProgress;
cleanup:

    return dwError;
error:

    if (pbIsUpgradeInProgress)
    {
        *pbIsUpgradeInProgress = FALSE;
    }
    goto cleanup;
}
