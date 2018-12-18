/*
 * Copyright © 2018 VMware, Inc.  All Rights Reserved.
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

VMCA_SECURITY_CONTEXT gSecurityCtx =
{
    // NOTE: order of fields MUST stay in sync with struct definition...
    VMCA_SF_INIT(.securityMutex, PTHREAD_RWLOCK_INITIALIZER),
    VMCA_SF_INIT(.pszPlugin, NULL),
    VMCA_SF_INIT(.pInterface, NULL),
    VMCA_SF_INIT(.pPluginHandle, NULL),
    VMCA_SF_INIT(.pHandle, NULL),
    VMCA_SF_INIT(.isInitialized, FALSE),
};
