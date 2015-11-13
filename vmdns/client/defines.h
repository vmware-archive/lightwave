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
 * Module   : defines.h
 *
 * Abstract :
 *
 *            VMware dns Service
 *
 *            Client API
 *
 *            Local definitions
 *
 */

#define VMDNS_ALIGN_BYTES(marker) \
            ((marker) % sizeof(size_t) ? \
                    sizeof(size_t) - ((marker) % sizeof(size_t)) : 0)

#define VMDNS_RPC_SAFE_FREE_MEMORY(pMemory) \
    if (pMemory) \
    { \
        unsigned32 rpcStatus = RPC_S_OK; \
        rpc_sm_client_free(pMemory, &rpcStatus); \
        (pMemory) = NULL; \
    }

#define VMDNS_RPC_TRY DCETHREAD_TRY
#define VMDNS_RPC_CATCH DCETHREAD_CATCH_ALL(THIS_CATCH)
#define VMDNS_RPC_ENDTRY DCETHREAD_ENDTRY
#define VMDNS_RPC_GETERROR_CODE(dwError) \
    dwError = VmDnsDCEGetErrorCode(THIS_CATCH);

#define VMDNS_RPC_PROTECT_LEVEL_PKT_PRIVACY rpc_c_protect_level_pkt_privacy
#define VMDNS_RPC_AUTHN_GSS_NEGOTIATE rpc_c_authn_gss_negotiate
#define VMDNS_RPC_AUTHZN_NAME rpc_c_authz_name

/*
 * 1.3.6.1.4.1.6876.11711.2.1.1.1
 *
 * {iso(1) identified-organization(3) dod(6) internet(1) private(4)
 *   enterprise(1) 6876 vmwSecurity(11711) vmwAuthentication(2) vmwGSSAPI(1)
 *   vmwSRP(1) vmwSrpCredOptPwd(1)}
 * Official registered GSSAPI_SRP password cred option OID
 */
#ifndef GSSAPI_SRP_CRED_OPT_PW
#define GSSAPI_SRP_CRED_OPT_PW  \
    "\x2b\x06\x01\x04\x01\xb5\x5c\xdb\x3f\x02\x01\x01\x01"
#endif
#ifndef GSSAPI_SRP_CRED_OPT_PW_LEN
#define GSSAPI_SRP_CRED_OPT_PW_LEN  13
#endif


#ifndef SPNEGO_OID
#define SPNEGO_OID_LENGTH 6
#define SPNEGO_OID "\x2b\x06\x01\x05\x05\x02"
#endif

#define VMDNS_CLOSE_BINDING_HANDLE(handle) \
{ \
    if ((handle) != NULL) \
    { \
        VmDnsFreeBindingHandle(handle); \
        (handle) = NULL; \
    } \
}
