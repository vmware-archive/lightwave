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

#ifndef _LWCA_MUTENTCA_PKCS_DEFINES_H_
#define _LWCA_MUTENTCA_PKCS_DEFINES_H_

#define X509_CERT_PRIVATE_KEY_MISMATCH 0
#define X509_CERT_PRIVATE_KEY_MATCH 1

#define LWCA_COMMON_NAME "CN"

typedef enum _LWCA_OPENSSL_NID_TYPES
{
    LWCA_OPENSSL_NID_O = 0,
    LWCA_OPENSSL_NID_CN
} LWCA_OPENSSL_NID_TYPES;

#endif /* _LWCA_MUTENTCA_PKCS_DEFINES_H_ */
