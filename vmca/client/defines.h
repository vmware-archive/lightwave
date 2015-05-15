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
#define VMCA_SF_INIT( fieldName, fieldValue ) fieldName = fieldValue
#else
#define VMCA_SF_INIT( fieldName, fieldValue ) fieldValue
#endif

#ifndef RPC_C_AUTHN_LEVEL_PKT_PRIVACY
#define RPC_C_AUTHN_LEVEL_PKT_PRIVACY rpc_c_protect_level_pkt_privacy
#endif

#ifndef RPC_C_AUTHN_GSS_NEGOTIATE
#define RPC_C_AUTHN_GSS_NEGOTIATE   rpc_c_authn_gss_negotiate
#endif

#ifndef RPC_C_AUTHZ_NAME
#define RPC_C_AUTHZ_NAME    rpc_c_authz_name
#endif

#define VMCA_RPC_PROTECT_LEVEL_NONE        RPC_C_AUTHN_LEVEL_NONE
#define VMCA_RPC_PROTECT_LEVEL_PKT_PRIVACY RPC_C_AUTHN_LEVEL_PKT_PRIVACY

#define VMCA_RPC_AUTHN_NONE                RPC_C_AUTHN_NONE
#define VMCA_RPC_AUTHN_GSS_NEGOTIATE       RPC_C_AUTHN_GSS_NEGOTIATE

#define VMCA_RPC_AUTHZN_NONE               RPC_C_AUTHZ_NONE
#define VMCA_RPC_AUTHZN_NAME               RPC_C_AUTHZ_NAME

#define VMCA_RPC_SAFE_FREE_MEMORY(pMemory) \
    if (pMemory) \
    { \
         unsigned32 rpcStatus = 0; \
         rpc_sm_client_free(pMemory, &rpcStatus); \
         (pMemory) = NULL; \
    }
