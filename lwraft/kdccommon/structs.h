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



#ifndef _WIN32

typedef struct _VMKDC_MUTEX
{
    BOOLEAN bInitialized;
    pthread_mutex_t critSect;
} VMKDC_MUTEX;

typedef struct _VMKDC_COND
{
    BOOLEAN bInitialized;
    pthread_cond_t cond;
} VMKDC_COND;

#else

typedef struct _VMKDC_MUTEX
{
    BOOLEAN bInitialized;
    CRITICAL_SECTION critSect;
} VMKDC_MUTEX;

typedef struct _VMKDC_COND
{
    BOOLEAN bInitialized;
    CONDITION_VARIABLE cond;
} VMKDC_COND;

#endif

typedef struct _VMKDC_THREAD_START_INFO
{
    VmKdcStartRoutine* pStartRoutine;
    PVOID pArgs;
} VMKDC_THREAD_START_INFO, * PVMKDC_THREAD_START_INFO;
