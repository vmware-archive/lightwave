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

#define VMAFD_ALIGN_BYTES(marker) \
            ((marker) % sizeof(size_t) ? \
                    sizeof(size_t) - ((marker) % sizeof(size_t)) : 0)

#else

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

#define VECS_LOCAL_HOST "localhost"
#define VECS_LOCAL_HOST_W {'l','o','c','a','l','h','o','s','t',0}

#define VMAFD_RPC_SAFE_FREE_MEMORY(pMemory) \
    if (pMemory) \
    { \
        unsigned32 rpcStatus = rpc_s_ok; \
        rpc_sm_client_free(pMemory, &rpcStatus); \
        (pMemory) = NULL; \
    }

#define VMAFD_CLOSE_BINDING_HANDLE(handle) \
{ \
    if ((handle) != NULL) \
    { \
        VmAfdFreeBindingHandle(handle); \
        (handle) = NULL; \
    } \
}

/* Defines related to GSS_SRP authentication */
#ifndef VMAFD_GSS_SRP_MECH_OID
#define VMAFD_GSS_SRP_MECH_OID_LENGTH 9
#define VMAFD_GSS_SRP_MECH_OID "\x2a\x86\x48\x86\xf7\x12\x01\x02\x0a"
#endif

#ifndef VMAFD_GSS_SRP_PASSWORD_OID
#define VMAFD_GSS_SRP_PASSWORD_OID "\x2b\x06\x01\x04\x01\x81\xd6\x29\x03\x01"
#define VMAFD_GSS_SRP_PASSWORD_LEN 10
#endif

#ifndef VMAFD_SPNEGO_OID
#define VMAFD_SPNEGO_OID_LENGTH 6
#define VMAFD_SPNEGO_OID "\x2b\x06\x01\x05\x05\x02"
#endif
