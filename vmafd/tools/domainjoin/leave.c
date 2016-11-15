/*
 * Copyright (C) 2015 VMware, Inc. All rights reserved.
 *
 * Module   : leave.c
 *
 * Abstract :
 *
 */

#include "includes.h"

DWORD
VmAfdLeaveDomain(
    PCSTR pszUsername,
    PCSTR pszPassword,
    DWORD dwLeaveFlags
    )
{
    DWORD dwError = ERROR_SUCCESS;

    dwError = VmAfdLeaveVmDirA(NULL, pszUsername, pszPassword, dwLeaveFlags);
    if (dwError == ERROR_NOT_JOINED)
    {
        dwError = ERROR_SUCCESS;
    }

    return dwError;
}
