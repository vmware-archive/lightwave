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

#ifndef _VM_METRICS_MEMORY_H__
#define _VM_METRICS_MEMORY_H__

DWORD
VmMetricsAllocateMemory(
    size_t  dwSize,
    PVOID*  ppMemory
    );

DWORD
VmMetricsReallocateMemory(
    PVOID  ppMemory,
    PVOID*  ppNewMemory,
    size_t  dwSize
    );

VOID
VmMetricsFreeMemory(
    PVOID   pMemory
    );

#endif /* __VM_METRICS_MEMORY_H__ */
