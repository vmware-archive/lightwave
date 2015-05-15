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



typedef struct _VECS_STORE_
{
    LONG  refCount;
    handle_t hBinding;
    BOOLEAN bOwnBinding;
    vecs_store_handle_t pStoreHandle;

} VECS_STORE;

typedef struct _VECS_ENUM_CONTEXT
{
    PVECS_STORE pStore;

    DWORD dwLimit;

    vecs_entry_enum_handle_t pEnumHandle;

} VECS_ENUM_CONTEXT;

/*
 * This is the structure which completes the incomplete type
 * in include/public/vmafdtypes.h.
 */
struct _VMAFD_SERVER
{
    PSTR pszServerName;
    PSTR pszUserName;
    rpc_binding_handle_t hBinding;
};
