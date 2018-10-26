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

#define LWCA_CERT_EXTENSION_DIGITAL_SIGNATURE "digitalSignature"
#define LWCA_CERT_EXTENSION_NON_REPUDIATION "nonRepudiation"
#define LWCA_CERT_EXTENSION_KEY_ENCIPHERMENT "keyEncipherment"
#define LWCA_CERT_EXTENSION_DATA_ENCIPHERMENT "dataEncipherment"
#define LWCA_CERT_EXTENSION_KEY_AGREEMENT "keyAgreement"
#define LWCA_CERT_EXTENSION_KEY_CERT_SIGN "critical, keyCertSign"
#define LWCA_CERT_EXTENSION_CRL_SIGN "cRLSign"
#define LWCA_CERT_EXTENSION_ENCIPHER_ONLY "encipherOnly"
#define LWCA_CERT_EXTENSION_DECIPHER_ONLY "decipherOnly"
#define LWCA_CERT_EXTENSION_KEY_CERT_SIGN_NID_VALUE "critical,CA:TRUE"
#define LWCA_CERT_ALT_STRING_KEY_NAME_DNS "DNS:"
#define LWCA_CERT_ALT_STRING_KEY_NAME_EMAIL "email:"
#define LWCA_CERT_ALT_STRING_KEY_NAME_IP "IP:"
#define LWCA_CERT_ALT_STRING_KEY_NAME_URI "URI:"
#define LWCA_CERT_EXTENSION_NID_KEY_IDENTIFIER "hash"
#define LWCA_CERT_EXTENSION_NID_AUTHORITY_KEY_IDENTIFIER "keyid"

#define LWCA_MIN_CA_CERT_PRIV_KEY_LENGTH 2048

#define LWCA_VALIDITY_SYNC_BACK_DATE        (LWCA_TIME_SECS_PER_WEEK * 2)
#define LWCA_MAX_CERT_DURATION              (LWCA_TIME_SECS_PER_YEAR * 10)

typedef enum _LWCA_OPENSSL_NID_TYPES
{
    LWCA_OPENSSL_NID_O = 0,
    LWCA_OPENSSL_NID_CN
} LWCA_OPENSSL_NID_TYPES;

#endif /* _LWCA_MUTENTCA_PKCS_DEFINES_H_ */
