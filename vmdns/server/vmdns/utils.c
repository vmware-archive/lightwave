/*
 * Copyright (C) 2011 VMware, Inc. All rights reserved.
 *
 * Module   : utils.c
 *
 * Abstract :
 *
 *            VMware dns Service
 *
 *            Service
 *
 *            utility functions
 *
 */

#include "includes.h"

VOID
VmDnsdStateSet(
    VMDNS_SERVER_STATE   state)
{
    VmDnsLockMutex(gVmdnsGlobals.pMutex);
    gVmdnsGlobals.vmdnsdState = state;
    VmDnsUnlockMutex(gVmdnsGlobals.pMutex);
}

VMDNS_SERVER_STATE
VmDnsdState(
    VOID
    )
{
    VMDNS_SERVER_STATE rtnState;

    VmDnsLockMutex(gVmdnsGlobals.pMutex);
    rtnState = gVmdnsGlobals.vmdnsdState;
    VmDnsUnlockMutex(gVmdnsGlobals.pMutex);

    return rtnState;
}
