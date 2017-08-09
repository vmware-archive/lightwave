/*
 * Copyright © 2017 VMware, Inc.  All Rights Reserved.
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

/*
 * REST_MODULE (from copenapitypes.h)
 * callback indices must correspond to:
 *      GET, PUT, POST, DELETE, PATCH
 */
REST_MODULE _etcd_rest_module[] =
{
    {
        "/v1/post/etcd/KV/put",
        { NULL, NULL, VmDirRESTEtcdPut, NULL, NULL}
    },
    {
        "/v1/post/etcd/KV/range",
        { NULL, NULL, VmDirRESTEtcdGet, NULL, NULL}
    },
    {
        "/v1/post/etcd/KV/deleteRange",
        { NULL, NULL, VmDirRESTEtcdDelete, NULL, NULL}
    }
};

DWORD
VmDirRESTGetEtcdModule(
    PREST_MODULE* ppRestModule
    )
{
    *ppRestModule = _etcd_rest_module;
    return 0;
}

DWORD
VmDirRESTEtcdPut(
    void*   pIn,
    void**  ppOut
    )
{
    // NOT IMPLEMENTED
    return 0;
}

DWORD
VmDirRESTEtcdGet(
    void*   pIn,
    void**  ppOut
    )
{
    // NOT IMPLEMENTED
    return 0;
}

DWORD
VmDirRESTEtcdDelete(
    void*   pIn,
    void**  ppOut
    )
{
    // NOT IMPLEMENTED
    return 0;
}
