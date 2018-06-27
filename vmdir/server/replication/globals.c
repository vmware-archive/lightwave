/*
 * Copyright © 2017 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the ?~@~\License?~@~]); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an ?~@~\AS IS?~@~] BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */

#include "includes.h"

VMDIR_REPLICATION_METRICS_CACHE gVdirReplMetricsCache =
{
        // NOTE: order of fields MUST stay in sync with struct definition...
        VMDIR_SF_INIT(.pHashMap, NULL),
        VMDIR_SF_INIT(.pLock, NULL)
};

PVMDIR_THREAD_LOG_CONTEXT   gReplThrLogCtx = NULL;
