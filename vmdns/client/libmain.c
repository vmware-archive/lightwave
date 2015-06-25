/*
 * Copyright (C) 2011 VMware, Inc. All rights reserved.
 *
 * Module   : libmain.c
 *
 * Abstract :
 *
 *            VMware dns Service
 *
 *            Client API
 *
 *            Library Entry Points
 *
 * Authors  : Sriram Nambakam (snambakam@vmware.com)
 *
 */

#include "includes.h"

#ifndef _WIN32

ULONG
VmDnsInitialize(
    VOID
    )
{
    return 0;
}

VOID
VmDnsShutdown(
    VOID
    )
{
}

#endif // #ifndef _WIN32
