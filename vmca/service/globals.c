/*
 * Copyright © 2012-2016 VMware, Inc.  All Rights Reserved.
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

VMCA_SERVER_GLOBALS gVMCAServerGlobals =
{
    // NOTE: order of fields MUST stay in sync with struct definition...
    VMCA_SF_INIT(.mutex, PTHREAD_MUTEX_INITIALIZER),
    VMCA_SF_INIT(.mutexCRL, PTHREAD_MUTEX_INITIALIZER),
    VMCA_SF_INIT(.dwCurrentCRLNumber, 0),
    VMCA_SF_INIT(.svcMutex, PTHREAD_RWLOCK_INITIALIZER),
    VMCA_SF_INIT(.fVMCALog, NULL),
    VMCA_SF_INIT(.pRPCServerThread, NULL),
    VMCA_SF_INIT(.vmcadState, VMCAD_STARTUP),
    VMCA_SF_INIT(.pCA, NULL),
    VMCA_SF_INIT(.dwFuncLevel, VMCA_FUNC_LEVEL_INITIAL),
    VMCA_SF_INIT(.pDirSyncParams, NULL),
    VMCA_SF_INIT(.pDirSyncThr, NULL),
    VMCA_SF_INIT(.gpEventLog, NULL)
};

#if 0
VMCA_ACCESS_TOKEN_METHODS gVMCAAccessTokenMethods[] =
{
    {VMCA_AUTHORIZATION_TYPE_BEARER_TOKEN, VMCAVerifyOIDC, VMCAFreeOIDC},
    {VMCA_AUTHORIZATION_TOKEN_TYPE_KRB, VMCARESTVerifyKrbAuth, VMCARESTFreeKrb}
};
#endif
