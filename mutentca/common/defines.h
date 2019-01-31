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

#ifndef __LWCA_COMMON_DEFINES_H__
#define __LWCA_COMMON_DEFINES_H__

typedef enum
{
    ATTR_NOT_FOUND = 0,
    ATTR_MATCH,
    ATTR_DIFFER,
} ATTR_SEARCH_RESULT;

#define STAT stat
#define UNLINK unlink
#define RENAME rename

#define LWCA_PATH_SEPARATOR "/"
#define LWCA_INSTALL_DIR MUTENTCA_INSTALL_DIR
#define LWCA_ROOT_CERT_DIR MUTENTCA_DB_DIR
#define LWCA_LOG_DIR MUTENTCA_LOG_DIR

#ifdef LIGHTWAVE_BUILD

#define VMAFD_VECS_CLIENT_LIBRARY   "/libvmafdclient.so"
#define VMAFD_KEY_ROOT              VMAFD_CONFIG_KEY_ROOT
#define VMAFD_LIB_KEY               VMAFD_REG_KEY_PATH

#else

#define VMAFD_VECS_CLIENT_LIBRARY   "/lib64/libvmafdclient.so"
#define VMAFD_KEY_ROOT              VMAFD_CONFIG_KEY_ROOT
#define VMAFD_LIB_KEY               VMAFD_REG_KEY_PATH

#endif /* LIGHTWAVE_BUILD */

#define MAX_CN_LENGTH 64

#define LWCA_MAX_PATH_LEN 512

#define LWCA_MACHINE_CERT_STORE_NAME        "MACHINE_SSL_CERT"
#define LWCA_MACHINE_CERT_ALIAS             "__MACHINE_CERT"
#define LWCA_MUTENTCA_STORE_NAME            "mutentca-srv-store"
#define LWCA_MUTENTCA_ALIAS                 "mutentca-srv-creds"

#define LWCA_OIDC_PORT 443

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

// Max domain name length is 256 characters. Make the max DN length 512 to add
// padding to account for a large number of labels (values seperated by `.`).
// The `+1`
#define LWCA_LDAP_DC_DN_MAXLENGTH           sizeof(char)*512 + 1
#define LWCA_UUID_LEN                       37

#define AIA_DATA_FORMAT "caIssuers;URI:%s"
#endif //__LWCA_COMMON_DEFINES_H__
