/*
 * Copyright (c) VMware Inc.  All rights Reserved.
 */

#include "includes.h"

DWORD
VmSockPosixInitialize(
    PVM_SOCK_PACKAGE* ppPackage
    )
{
    *ppPackage = gpVmSockPosixPackage;

    return 0;
}

VOID
VmSockPosixShutdown(
    PVM_SOCK_PACKAGE pPackage
    )
{
}
