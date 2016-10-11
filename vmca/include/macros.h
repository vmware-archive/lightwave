/*
 * Copyright © 2012-2016 VMware, Inc.  All Rights Reserved.
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
 * Module Name: ThinAppRepoClient
 *
 * Filename: defines.h
 *
 * Abstract:
 *
 * Definitions
 *
 */

#ifdef __cplusplus
extern "C"
{
#endif

#define BAIL_ON_ERROR(dwError) \
	do { \
	if (dwError) goto error; \
	}while(0)

#define REPO_ALIGN_BYTES(marker) \
            ((marker) % sizeof(size_t) ? \
                    sizeof(size_t) - ((marker) % sizeof(size_t)) : 0)

#define REPO_RPC_SAFE_FREE_MEMORY(mem) \
    if (mem) \
    { \
        unsigned32 rpcStatus = RPC_S_OK; \
        rpc_sm_client_free(mem, &rpcStatus); \
        (mem) = NULL; \
    }

#ifdef __cplusplus
}
#endif
