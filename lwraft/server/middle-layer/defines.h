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
 * Module Name: Directory middle-layer
 *
 * Filename: defines.h
 *
 * Abstract:
 *
 * Directory middle-layer (main ldap and business) module
 *
 * Definitions
 *
 */

#define MAX_PASSWROD_DIGEST_LEN     128       // max digest size in bytes 1024/8
#define MAX_PASSWD_SALT_LEN         32        // max salt size in bytes

#define PASSWD_DEFAULT_RETENTION_COUNT  5

#define PASSWD_BLOB_LENGTH(pPasswdScheme)   \
    ( 1 + (pPasswdScheme)->uDigestSizeInByte + (pPasswdScheme)->uSaltSizeInByte )

#define PASSWD_DIGEST_OFFSET(pPasswdScheme) \
    (1)

#define PASSWD_SALT_OFFSET(pPasswdScheme)   \
     (1 + (pPasswdScheme)->uDigestSizeInByte)

//TODO, make this config param?
#define MAX_GOOD_CANDIDATE_SIZE     100

#define BAIL_ON_SASL_ERROR(dwError) \
    if (dwError)                                                    \
    {                                                               \
        VmDirLog( LDAP_DEBUG_TRACE, "[%s,%d]",__FILE__, __LINE__);  \
        goto sasl_error;                                            \
    }

#define BAIL_ON_SASL_ERROR_WITH_MSG(dwError, pszErrMsg, Format, ... ) \
    if (dwError)                                                    \
    {                                                               \
        if (pszErrMsg == NULL)                                      \
        {                                                           \
            VmDirAllocateStringAVsnprintf(                          \
                            &(pszErrMsg),                           \
                            Format,                                 \
                            ##__VA_ARGS__);                         \
        }                                                           \
        VmDirLog( LDAP_DEBUG_TRACE, "[%s,%d]",__FILE__, __LINE__);  \
        goto sasl_error;                                            \
    }

//
// The timeout for a client thread waiting for more data from the worker
// thread (we'll loop until more data's available, but we wake up periodically
// to check if the server's shutting down).
//
#define VMDIR_PSCACHE_READ_TIMEOUT 100
