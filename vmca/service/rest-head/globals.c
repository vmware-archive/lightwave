/*
 * Copyright © 2018 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the [0m~@~\License[0m~@~]); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an [0m~@~\AS IS[0m~@~] BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */

#include "includes.h"

PREST_API_DEF gpVMCARestApiDef = NULL;

PVMREST_HANDLE gpVMCARestHTTPSHandle = NULL;

VMCA_SERVER_OPENSSL_GLOBALS gVMCAOpensslGlobals =
    {
        VMCA_SF_INIT(.pMutexBuf,        NULL),
        VMCA_SF_INIT(.dwMutexBufSize,   0),
        VMCA_SF_INIT(.bSSLInitialized,  FALSE),
    };
