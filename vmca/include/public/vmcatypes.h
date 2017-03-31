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

#ifndef __VMCATYPES_H__
#define __VMCATYPES_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _VMCA_SERVER_CONTEXT VMCA_SERVER_CONTEXT, *PVMCA_SERVER_CONTEXT;

#ifndef _VMCA_CRL_REASON_DEFINED
#define _VMCA_CRL_REASON_DEFINED 1
typedef enum
{
    VMCA_CRL_REASON_UNSPECIFIED = 0,
    VMCA_CRL_REASON_KEY_COMPROMISE =1,
    VMCA_CRL_REASON_CA_COMPROMISE =2,
    VMCA_CRL_REASON_AFFILIATION_CHANGED =3,
    VMCA_CRL_REASON_SUPERSEDED =4,
    VMCA_CRL_REASON_CESSATION_OF_OPERATION =5,
    VMCA_CRL_REASON_CERTIFICATE_HOLD =6,
    VMCA_CRL_REASON_REMOVE_FROM_CRL =8,
    VMCA_CRL_REASON_PRIVILEGE_WITHDRAWN =9,
    VMCA_CRL_REASON_AA_COMPROMISE =10
} VMCA_CRL_REASON, *PVMCA_CRL_REASON;
#endif /* _VMCA_CRL_REASON_DEFINED */

#ifndef _VMCA_CERTIFICATE_FLAGS_DEFINED
#define _VMCA_CERTIFICATE_FLAGS_DEFINED 1
typedef enum {
      VMCA_CERTIFICATE_ACTIVE   = 0,
      VMCA_CERTIFICATE_REVOKED  = 1,
      VMCA_CERTIFICATE_EXPIRED  = 2,
      VMCA_CERTIFICATE_ALL      = 4
 } VMCA_CERTIFICATE_STATUS;
#endif // _VMCA_CERTIFICATE_FLAGS_DEFINED

#ifndef _VMCA_SERVER_OPTION_DEFINED
#define _VMCA_SERVER_OPTION_DEFINED 1
typedef enum {
    VMCA_SERVER_OPT_ALLOW_MULTIPLE_SAN  = 1,
    VMCA_SERVER_OPT_COUNT = 2
 } VMCA_SERVER_OPTION;
#endif // _VMCA_SERVER_OPTION_DEFINED


#ifdef __cplusplus
}
#endif

#endif
