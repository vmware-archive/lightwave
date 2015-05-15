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

#define VMAUTHSVC_ALIGN_BYTES(marker) \
            ((marker) % sizeof(size_t) ? \
                    sizeof(size_t) - ((marker) % sizeof(size_t)) : 0)

#define VMAUTHSVC_RPC_SAFE_FREE_MEMORY(pMemory) \
    if (pMemory) \
    { \
        unsigned32 rpcStatus = RPC_S_OK; \
        rpc_sm_client_free(pMemory, &rpcStatus); \
        (pMemory) = NULL; \
    }

#define VMAUTHSVC_RPC_TRY DCETHREAD_TRY
#define VMAUTHSVC_RPC_CATCH DCETHREAD_CATCH_ALL(THIS_CATCH)
#define VMAUTHSVC_RPC_ENDTRY DCETHREAD_ENDTRY
#define VMAUTHSVC_RPC_GETERROR_CODE(dwError) \
    dwError = VmAuthsvcDCEGetErrorCode(THIS_CATCH);

#define VMAUTHSVC_RPC_PROTECT_LEVEL_PKT_PRIVACY rpc_c_protect_level_pkt_privacy
#define VMAUTHSVC_RPC_AUTHN_GSS_NEGOTIATE rpc_c_authn_gss_negotiate
#define VMAUTHSVC_RPC_AUTHZN_NAME rpc_c_authz_name

#else

#define VMAUTHSVC_RPC_TRY RpcTryExcept
#define VMAUTHSVC_RPC_CATCH RpcExcept(1)
#define VMAUTHSVC_RPC_ENDTRY RpcEndExcept
#define VMAUTHSVC_RPC_GETERROR_CODE(dwError) \
    dwError = RpcExceptionCode();

#define VMAUTHSVC_RPC_PROTECT_LEVEL_PKT_PRIVACY RPC_C_AUTHN_LEVEL_PKT_PRIVACY
#define VMAUTHSVC_RPC_AUTHN_GSS_NEGOTIATE RPC_C_AUTHN_GSS_NEGOTIATE
#define VMAUTHSVC_RPC_AUTHZN_NAME RPC_C_AUTHZ_NAME

/*
MSDN: gethostname
The maximum length, in bytes, of the string returned in the buffer
pointed to by the name parameter is dependent on the namespace provider,
but this string must be 256 bytes or less.
So if a buffer of 256 bytes is passed in the name parameter
and the namelen parameter is set to 256,
the buffer size will always be adequate.
*/
#define HOST_NAME_MAX 256

#endif

#define VMAUTHSVC_CLOSE_BINDING_HANDLE(handle) \
{ \
    if ((handle) != NULL) \
    { \
        VmAuthsvcFreeBindingHandle(handle); \
        (handle) = NULL; \
    } \
}
