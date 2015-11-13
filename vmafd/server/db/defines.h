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
 * Module Name: VMware Certificate Server
 *
 * Filename: defines.h
 *
 * Abstract:
 *
 * VMware Certificate Server Database
 *
 * Definitions
 *
 */

#ifndef _VECS_DB_DEFINES_H__
#define _VECS_DB_DEFINES_H__


#define VECS_DB_MAX_NUM_CACHED_CONTEXTS (0)

#define VECS_SAFE_FREE_STRINGA VMAFD_SAFE_FREE_STRINGA
#define VECS_SAFE_FREE_MEMORY VMAFD_SAFE_FREE_MEMORY
#define VECS_LOCK_MUTEX VMAFD_LOCK_MUTEX
#define VECS_UNLOCK_MUTEX VMAFD_UNLOCK_MUTEX
#define VECS_DB_LOCK_MUTEX pthread_mutex_lock
#define VECS_DB_UNLOCK_MUTEX pthread_mutex_unlock

#define VecsAllocateMemory  VmAfdAllocateMemory
#define VecsFreeMemory   VmAfdFreeMemory


#define BAIL_ON_VECS_ERROR(dwError)                 \
    if (dwError)                                    \
    {                                               \
        if (dwError == 2)                           \
        {                                           \
            dwError = ERROR_DATABASE_FAILURE;       \
        }                                           \
        goto error;                                 \
    }


#define UNKNOWN_STRING "UNKNOWN"
#define STORE_TABLE_LIMIT 1024
#define CDC_SERVICE_NAME "cdc"

#ifdef _WIN32
#define VECS_DB_INIT( fieldName, fieldValue ) fieldValue
//#define PTHREAD_MUTEX_INITIALIZER {(void*)-1,-1,0,0,0,0}
#endif

#endif //_VECS_DB_DEFINES_H__
