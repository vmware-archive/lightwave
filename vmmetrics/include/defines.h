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

#ifndef __VM_METRICS_DEFINE_H__
#define __VM_METRICS_DEFINE_H__

#define BUFFER_SIZE 1024

#define VM_METRICS_SAFE_FREE_MEMORY(PTR)       \
    do {                                       \
        if ((PTR)) {                           \
            VmMetricsFreeMemory(PTR);          \
            (PTR) = NULL;                      \
        }                                      \
    } while(0)

#define BAIL_ON_VM_METRICS_ERROR(dwError)      \
    if (dwError)                               \
    {                                          \
        goto error;                            \
    }

#endif /* __VM_METRICS_DEFINE_H__ */
