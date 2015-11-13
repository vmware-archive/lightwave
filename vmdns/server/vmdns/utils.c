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


/*
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
