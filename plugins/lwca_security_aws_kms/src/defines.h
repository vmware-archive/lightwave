/*
 * Copyright © 2018 VMware, Inc.  All Rights Reserved.
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

#ifndef _LWCA_SECURITY_AWS_KMS_DEFINES_H_
#define _LWCA_SECURITY_AWS_KMS_DEFINES_H_

#define BAIL_ON_SECURITY_AWS_KMS_ERROR(dwError) \
    if (dwError)                                \
    {                                           \
        goto error;                             \
    }

#ifndef IsNullOrEmptyString
#define IsNullOrEmptyString(str) (!(str) || !*(str))
#endif

#define AES_256_STR "AES_256"

#define KEY_REGION         "region"
#define KEY_CMK_ID         "cmk_id"
#define KEY_KEY_SPEC       "key_spec"

#define LWCA_SECURITY_AWS_KMS "lwca_security_aws_kms"

#endif /* _LWCA_SECURITY_AWS_KMS_DEFINES_H_ */
