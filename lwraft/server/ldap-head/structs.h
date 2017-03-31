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
 * Module Name: Directory ldap-head
 *
 * Filename: structs.h
 *
 * Abstract:
 *
 * Directory ldap-head module
 *
 * Private Structures
 *
 */

typedef struct _VMDIR_OP_STATISTIC_GLOBALS
{
    VMDIR_OPERATION_STATISTIC    opBind;
    VMDIR_OPERATION_STATISTIC    opAdd;
    VMDIR_OPERATION_STATISTIC    opSearch;
    VMDIR_OPERATION_STATISTIC    opModify;
    VMDIR_OPERATION_STATISTIC    opDelete;
    VMDIR_OPERATION_STATISTIC    opUnbind;

} VMDIR_OP_STATISTIC_GLOBALS, *PVMDIR_OP_STATISTIC_GLOBALS;

typedef struct _VMDIR_OPENSSL_GLOBALS
{
    pthread_mutex_t*        pMutexBuf;
    DWORD                   dwMutexBufSize;
    BOOLEAN                 bSSLInitialized;

} VMDIR_OPENSSL_GLOBALS, *PVMDIR_OPENSSL_GLOBALS;
