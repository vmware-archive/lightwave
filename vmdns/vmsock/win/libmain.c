/*
 * Copyright (c) VMware Inc.  All rights Reserved.
 */

#include "includes.h"

DWORD
VmWinSockInitialize(
    PVM_SOCK_PACKAGE* ppPackage
    )
{
    DWORD dwError = 0;
    WSADATA wsaData = { 0 };

    dwError = WSAStartup(MAKEWORD(2, 2), &wsaData);
    BAIL_ON_VMDNS_ERROR(dwError);

    *ppPackage = gpVmWinSockPackage;

cleanup:

    return dwError;

error:

    *ppPackage = NULL;

    goto cleanup;
}

VOID
VmWinSockShutdown(
    PVM_SOCK_PACKAGE pPackage
    )
{
    WSACleanup();
}
