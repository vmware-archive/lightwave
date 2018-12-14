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

#ifndef __LWCA_ERROR_H__
#define __LWCA_ERROR_H__

typedef struct _LWCA_ERROR_CODE_NAME_MAP
{
    DWORD       dwCount;
    PCSTR       pcszName;
    PCSTR       pcszDesc;
} LWCA_ERROR_CODE_NAME_MAP, *PLWCA_DB_ERROR_CODE_NAME_MAP;

typedef struct _LWCA_ERRNO_MAP
{
    DWORD   dwUnixErrno;
    DWORD   dwLwCAError;
} LWCA_ERRNO_MAP;

#define UNKNOWN_STRING                      "UNKNOWN"

#define LWCA_SUCCESS                        0

#define LWCA_ERROR_BASE                     80000
#define LWCA_ERROR_MAX                      9999

#define LWCA_RANGE(n,x,y)                  (((x) <= (n)) && ((n) <= (y)))

// LwCA error space (80000 - 89999)
#define IS_LWCA_ERROR_SPACE(n) \
    LWCA_RANGE((n), (LWCA_ERROR_BASE), (LWCA_ERROR_BASE + LWCA_ERROR_MAX)) || n == LWCA_SUCCESS

#define IS_LWCA_UNKNOWN_ERROR(n) \
    (IS_LWCA_ERROR_SPACE((n)) ? (n) : (LWCA_UNKNOWN_ERROR))

#define LWCA_SYSTEM_ERROR_BASE              0
#define LWCA_AUTH_ERROR_BASE                100
#define LWCA_AUTHZ_ERROR_BASE               200
#define LWCA_POLICY_ERROR_BASE              300
#define LWCA_SSL_ERROR_BASE                 400
#define LWCA_KEY_ERROR_BASE                 500
#define LWCA_STORAGE_ERROR_BASE             600
#define LWCA_REST_ERROR_BASE                700
#define LWCA_ERRNO_BASE                     800
#define LWCA_OIDC_ERROR_BASE                900
#define LWCA_SECURITY_ERROR_BASE            1000
#define LWCA_CURL_ERROR_BASE                1100
#define LWCA_REGEX_ERROR_BASE               1200
#define LWCA_LDAP_ERROR_BASE                1300
#define LWCA_MISC_ERROR_BASE                2000

// System Error Codes (80000 - 80099)
#define LWCA_ERROR_INVALID_PARAMETER        (LWCA_ERROR_BASE + LWCA_SYSTEM_ERROR_BASE +  1)
#define LWCA_OUT_OF_MEMORY_ERROR            (LWCA_ERROR_BASE + LWCA_SYSTEM_ERROR_BASE +  2)
#define LWCA_FILE_IO_ERROR                  (LWCA_ERROR_BASE + LWCA_SYSTEM_ERROR_BASE +  3)
#define LWCA_FILE_TIME_ERROR                (LWCA_ERROR_BASE + LWCA_SYSTEM_ERROR_BASE +  4)
#define LWCA_FILE_REMOVE_ERROR              (LWCA_ERROR_BASE + LWCA_SYSTEM_ERROR_BASE +  5)
#define LWCA_ERROR_TIME_OUT                 (LWCA_ERROR_BASE + LWCA_SYSTEM_ERROR_BASE +  6)
#define LWCA_REQUEST_ERROR                  (LWCA_ERROR_BASE + LWCA_SYSTEM_ERROR_BASE +  7)
#define LWCA_DIR_CREATE_ERROR               (LWCA_ERROR_BASE + LWCA_SYSTEM_ERROR_BASE +  8)
#define LWCA_ERROR_NO_FILE_OR_DIRECTORY     (LWCA_ERROR_BASE + LWCA_SYSTEM_ERROR_BASE +  9)
#define LWCA_NOT_IMPLEMENTED                (LWCA_ERROR_BASE + LWCA_SYSTEM_ERROR_BASE +  10)
#define LWCA_GET_NAME_INFO_FAIL             (LWCA_ERROR_BASE + LWCA_SYSTEM_ERROR_BASE +  11)
#define LWCA_ERROR_DLL_SYMBOL_NOTFOUND      (LWCA_ERROR_BASE + LWCA_SYSTEM_ERROR_BASE +  12)
#define LWCA_ERROR_CANNOT_LOAD_LIBRARY      (LWCA_ERROR_BASE + LWCA_SYSTEM_ERROR_BASE +  13)
#define LWCA_ERROR_ENTRY_NOT_FOUND          (LWCA_ERROR_BASE + LWCA_SYSTEM_ERROR_BASE +  14)
#define LWCA_ERROR_INVALID_STATE            (LWCA_ERROR_BASE + LWCA_SYSTEM_ERROR_BASE +  15)
#define LWCA_ERROR_INVALID_ENTRY            (LWCA_ERROR_BASE + LWCA_SYSTEM_ERROR_BASE +  16)
#define LWCA_ERROR_INVALID_DATA             (LWCA_ERROR_BASE + LWCA_SYSTEM_ERROR_BASE +  17)
#define LWCA_ERROR_BUFFER_OVERFLOW          (LWCA_ERROR_BASE + LWCA_SYSTEM_ERROR_BASE +  18)

// Auth Error Codes (80100 - 80199)
#define LWCA_INVALID_USER_NAME              (LWCA_ERROR_BASE + LWCA_AUTH_ERROR_BASE + 1)
#define LWCA_ERROR_AUTH_BAD_DATA            (LWCA_ERROR_BASE + LWCA_AUTH_ERROR_BASE + 2)
#define LWCA_UNABLE_GET_CRED_CACHE_NAME     (LWCA_ERROR_BASE + LWCA_AUTH_ERROR_BASE + 3)
#define LWCA_NO_CACHE_FOUND                 (LWCA_ERROR_BASE + LWCA_AUTH_ERROR_BASE + 4)
#define LWCA_KRB_ACCESS_DENIED              (LWCA_ERROR_BASE + LWCA_AUTH_ERROR_BASE + 5)
#define LWCA_GET_ADDR_INFO_FAIL             (LWCA_ERROR_BASE + LWCA_AUTH_ERROR_BASE + 6)
#define LWCA_LDAP_UPN_FAIL                  (LWCA_ERROR_BASE + LWCA_AUTH_ERROR_BASE + 7)
#define LWCA_ACCESS_DENIED                  (LWCA_ERROR_BASE + LWCA_AUTH_ERROR_BASE + 8)

// AuthZ Error Codes (80200 - 8299)
#define LWCA_ERROR_AUTHZ_INITIALIZED        (LWCA_ERROR_BASE + LWCA_AUTHZ_ERROR_BASE + 1)
#define LWCA_ERROR_AUTHZ_UNINITIALIZED      (LWCA_ERROR_BASE + LWCA_AUTHZ_ERROR_BASE + 2)
#define LWCA_ERROR_AUTHZ_UNAUTHORIZED       (LWCA_ERROR_BASE + LWCA_AUTHZ_ERROR_BASE + 3)
#define LWCA_ERROR_AUTHZ_INVALID_PLUGIN     (LWCA_ERROR_BASE + LWCA_AUTHZ_ERROR_BASE + 4)

// Policy Error Codes (80300 - 80399)
#define LWCA_SN_POLICY_VIOLATION            (LWCA_ERROR_BASE + LWCA_POLICY_ERROR_BASE + 1)
#define LWCA_SAN_POLICY_VIOLATION           (LWCA_ERROR_BASE + LWCA_POLICY_ERROR_BASE + 2)
#define LWCA_CERT_DURATION_POLICY_VIOLATION (LWCA_ERROR_BASE + LWCA_POLICY_ERROR_BASE + 3)
#define LWCA_KEY_USAGE_POLICY_VIOLATION     (LWCA_ERROR_BASE + LWCA_POLICY_ERROR_BASE + 4)
#define LWCA_POLICY_CONFIG_PARSE_ERROR      (LWCA_ERROR_BASE + LWCA_POLICY_ERROR_BASE + 5)

// SSL (CA) Error Codes (80400 - 80499)
#define LWCA_ROOT_CA_MISSING                (LWCA_ERROR_BASE + LWCA_SSL_ERROR_BASE + 1)
#define LWCA_SSL_SET_PUBKEY_ERR             (LWCA_ERROR_BASE + LWCA_SSL_ERROR_BASE + 2)
#define LWCA_ROOT_CA_ALREADY_EXISTS         (LWCA_ERROR_BASE + LWCA_SSL_ERROR_BASE + 3)
#define LWCA_CA_MISSING                     (LWCA_ERROR_BASE + LWCA_SSL_ERROR_BASE + 4)
#define LWCA_CA_REVOKED                     (LWCA_ERROR_BASE + LWCA_SSL_ERROR_BASE + 5)
#define LWCA_CA_ALREADY_REVOKED             (LWCA_ERROR_BASE + LWCA_SSL_ERROR_BASE + 6)
#define LWCA_CA_ALREADY_EXISTS              (LWCA_ERROR_BASE + LWCA_SSL_ERROR_BASE + 7)
#define LWCA_INVALID_CA_DATA                (LWCA_ERROR_BASE + LWCA_SSL_ERROR_BASE + 8)
#define LWCA_INVALID_TIME_SPECIFIED         (LWCA_ERROR_BASE + LWCA_SSL_ERROR_BASE + 9)
#define LWCA_KEY_CREATION_FAILURE           (LWCA_ERROR_BASE + LWCA_SSL_ERROR_BASE + 10)
#define LWCA_CERT_DECODE_FAILURE            (LWCA_ERROR_BASE + LWCA_SSL_ERROR_BASE + 11)
#define LWCA_KEY_IO_FAILURE                 (LWCA_ERROR_BASE + LWCA_SSL_ERROR_BASE + 12)
#define LWCA_CERT_IO_FAILURE                (LWCA_ERROR_BASE + LWCA_SSL_ERROR_BASE + 13)
#define LWCA_NOT_CA_CERT                    (LWCA_ERROR_BASE + LWCA_SSL_ERROR_BASE + 14)
#define LWCA_INVALID_CSR_FIELD              (LWCA_ERROR_BASE + LWCA_SSL_ERROR_BASE + 15)
#define LWCA_SELF_SIGNATURE_FAILED          (LWCA_ERROR_BASE + LWCA_SSL_ERROR_BASE + 16)
#define LWCA_INIT_CA_FAILED                 (LWCA_ERROR_BASE + LWCA_SSL_ERROR_BASE + 17)
#define LWCA_ERROR_INVALID_KEY_LENGTH       (LWCA_ERROR_BASE + LWCA_SSL_ERROR_BASE + 18)
#define LWCA_PKCS12_CREAT_FAIL              (LWCA_ERROR_BASE + LWCA_SSL_ERROR_BASE + 19)
#define LWCA_PKCS12_IO_FAIL                 (LWCA_ERROR_BASE + LWCA_SSL_ERROR_BASE + 20)
#define LWCA_CRL_ERROR                      (LWCA_ERROR_BASE + LWCA_SSL_ERROR_BASE + 21)
#define LWCA_NO_NEW_CRL                     (LWCA_ERROR_BASE + LWCA_SSL_ERROR_BASE + 22)
#define LWCA_ERROR_READING_CRL              (LWCA_ERROR_BASE + LWCA_SSL_ERROR_BASE + 23)
#define LWCA_CRL_LOCAL_ERROR                (LWCA_ERROR_BASE + LWCA_SSL_ERROR_BASE + 24)
#define LWCA_SSL_ADD_EXTENSION              (LWCA_ERROR_BASE + LWCA_SSL_ERROR_BASE + 25)
#define LWCA_SSL_REQ_SIGN_ERR               (LWCA_ERROR_BASE + LWCA_SSL_ERROR_BASE + 26)
#define LWCA_SSL_RAND_ERR                   (LWCA_ERROR_BASE + LWCA_SSL_ERROR_BASE + 27)
#define LWCA_SSL_CERT_SIGN_ERR              (LWCA_ERROR_BASE + LWCA_SSL_ERROR_BASE + 28)
#define LWCA_SSL_TIME_ERROR                 (LWCA_ERROR_BASE + LWCA_SSL_ERROR_BASE + 29)
#define LWCA_SSL_EXT_ERR                    (LWCA_ERROR_BASE + LWCA_SSL_ERROR_BASE + 30)
#define LWCA_SSL_SIGN_FAIL                  (LWCA_ERROR_BASE + LWCA_SSL_ERROR_BASE + 31)
#define LWCA_SSL_SET_ISSUER_NAME            (LWCA_ERROR_BASE + LWCA_SSL_ERROR_BASE + 32)
#define LWCA_SSL_SET_START_TIME             (LWCA_ERROR_BASE + LWCA_SSL_ERROR_BASE + 33)
#define LWCA_SSL_SET_END_TIME               (LWCA_ERROR_BASE + LWCA_SSL_ERROR_BASE + 34)
#define LWCA_SSL_SET_EXT_ERR                (LWCA_ERROR_BASE + LWCA_SSL_ERROR_BASE + 35)
#define LWCA_CERT_PRIVATE_KEY_MISMATCH      (LWCA_ERROR_BASE + LWCA_SSL_ERROR_BASE + 36)
#define LWCA_INVALID_DOMAIN_NAME            (LWCA_ERROR_BASE + LWCA_SSL_ERROR_BASE + 37)
#define LWCA_CRL_SET_SERIAL_FAIL            (LWCA_ERROR_BASE + LWCA_SSL_ERROR_BASE + 38)
#define LWCA_CRL_SET_TIME_FAIL              (LWCA_ERROR_BASE + LWCA_SSL_ERROR_BASE + 39)
#define LWCA_CRL_CERT_ALREADY_REVOKED       (LWCA_ERROR_BASE + LWCA_SSL_ERROR_BASE + 40)
#define LWCA_CRLNUMBER_READ_ERROR           (LWCA_ERROR_BASE + LWCA_SSL_ERROR_BASE + 41)
#define LWCA_CRL_SIGN_FAIL                  (LWCA_ERROR_BASE + LWCA_SSL_ERROR_BASE + 42)
#define LWCA_CRL_REASON_FAIL                (LWCA_ERROR_BASE + LWCA_SSL_ERROR_BASE + 43)
#define LWCA_CRL_SORT_FAILED                (LWCA_ERROR_BASE + LWCA_SSL_ERROR_BASE + 44)
#define LWCA_CRL_NULL_TIME                  (LWCA_ERROR_BASE + LWCA_SSL_ERROR_BASE + 45)
#define LWCA_CRL_DECODE_ERROR               (LWCA_ERROR_BASE + LWCA_SSL_ERROR_BASE + 46)
#define LWCA_VPX_RSUTIL_ERROR               (LWCA_ERROR_BASE + LWCA_SSL_ERROR_BASE + 47)
#define LWCA_ERROR_INVALID_SN               (LWCA_ERROR_BASE + LWCA_SSL_ERROR_BASE + 48)
#define LWCA_ERROR_INVALID_SAN              (LWCA_ERROR_BASE + LWCA_SSL_ERROR_BASE + 49)
#define LWCA_ERROR_INVALID_CERTIFICATE      (LWCA_ERROR_BASE + LWCA_SSL_ERROR_BASE + 50)
#define LWCA_ERROR_INCOMPLETE_CHAIN         (LWCA_ERROR_BASE + LWCA_SSL_ERROR_BASE + 51)
#define LWCA_ERROR_INVALID_CHAIN            (LWCA_ERROR_BASE + LWCA_SSL_ERROR_BASE + 52)
#define LWCA_ERROR_CANNOT_FORM_REQUEST      (LWCA_ERROR_BASE + LWCA_SSL_ERROR_BASE + 53)
#define LWCA_KEY_DECODE_FAILURE             (LWCA_ERROR_BASE + LWCA_SSL_ERROR_BASE + 54)
#define LWCA_ERROR_CN_HOSTNAME_MISMATCH     (LWCA_ERROR_BASE + LWCA_SSL_ERROR_BASE + 55)
#define LWCA_ERROR_SAN_HOSTNAME_MISMATCH    (LWCA_ERROR_BASE + LWCA_SSL_ERROR_BASE + 56)
#define LWCA_ERROR_SAN_IPADDR_INVALID       (LWCA_ERROR_BASE + LWCA_SSL_ERROR_BASE + 57)
#define LWCA_ERROR_EVP_DIGEST               (LWCA_ERROR_BASE + LWCA_SSL_ERROR_BASE + 58)
#define LWCA_OPENSSL_ERROR                  (LWCA_ERROR_BASE + LWCA_SSL_ERROR_BASE + 59)
#define LWCA_ERROR_INVALID_DN               (LWCA_ERROR_BASE + LWCA_SSL_ERROR_BASE + 60)
#define LWCA_SSL_STORE_ADD_CERT_FAIL        (LWCA_ERROR_BASE + LWCA_SSL_ERROR_BASE + 61)
#define LWCA_SSL_STORE_CTX_INIT_FAIL        (LWCA_ERROR_BASE + LWCA_SSL_ERROR_BASE + 62)
#define LWCA_SSL_CERT_VERIFY_ERR            (LWCA_ERROR_BASE + LWCA_SSL_ERROR_BASE + 63)
#define LWCA_SSL_INVALID_NID                (LWCA_ERROR_BASE + LWCA_SSL_ERROR_BASE + 64)
#define LWCA_SSL_NO_EXTENSIONS              (LWCA_ERROR_BASE + LWCA_SSL_ERROR_BASE + 65)

// ERRNO to LwCA Codes (80800 - 80899)
#define LWCA_ERRNO_EPERM                    (LWCA_ERROR_BASE + LWCA_ERRNO_BASE + EPERM)
#define LWCA_ERRNO_ENOENT                   (LWCA_ERROR_BASE + LWCA_ERRNO_BASE + ENOENT)
#define LWCA_ERRNO_ESRCH                    (LWCA_ERROR_BASE + LWCA_ERRNO_BASE + ESRCH)
#define LWCA_ERRNO_EINTR                    (LWCA_ERROR_BASE + LWCA_ERRNO_BASE + EINTR)
#define LWCA_ERRNO_EIO                      (LWCA_ERROR_BASE + LWCA_ERRNO_BASE + EIO)
#define LWCA_ERRNO_ENXIO                    (LWCA_ERROR_BASE + LWCA_ERRNO_BASE + ENXIO)
#define LWCA_ERRNO_E2BIG                    (LWCA_ERROR_BASE + LWCA_ERRNO_BASE + E2BIG)
#define LWCA_ERRNO_ENOEXEC                  (LWCA_ERROR_BASE + LWCA_ERRNO_BASE + ENOEXEC)
#define LWCA_ERRNO_EBADF                    (LWCA_ERROR_BASE + LWCA_ERRNO_BASE + EBADF)
#define LWCA_ERRNO_ECHILD                   (LWCA_ERROR_BASE + LWCA_ERRNO_BASE + ECHILD)
#define LWCA_ERRNO_EAGAIN                   (LWCA_ERROR_BASE + LWCA_ERRNO_BASE + EAGAIN)
#define LWCA_ERRNO_ENOMEM                   (LWCA_ERROR_BASE + LWCA_ERRNO_BASE + ENOMEM)
#define LWCA_ERRNO_EACCES                   (LWCA_ERROR_BASE + LWCA_ERRNO_BASE + EACCES)
#define LWCA_ERRNO_EFAULT                   (LWCA_ERROR_BASE + LWCA_ERRNO_BASE + EFAULT)
#define LWCA_ERRNO_ENOTBLK                  (LWCA_ERROR_BASE + LWCA_ERRNO_BASE + ENOTBLK)
#define LWCA_ERRNO_EBUSY                    (LWCA_ERROR_BASE + LWCA_ERRNO_BASE + EBUSY)
#define LWCA_ERRNO_EEXIST                   (LWCA_ERROR_BASE + LWCA_ERRNO_BASE + EEXIST)
#define LWCA_ERRNO_EXDEV                    (LWCA_ERROR_BASE + LWCA_ERRNO_BASE + EXDEV)
#define LWCA_ERRNO_ENODEV                   (LWCA_ERROR_BASE + LWCA_ERRNO_BASE + ENODEV)
#define LWCA_ERRNO_ENOTDIR                  (LWCA_ERROR_BASE + LWCA_ERRNO_BASE + ENOTDIR)
#define LWCA_ERRNO_EISDIR                   (LWCA_ERROR_BASE + LWCA_ERRNO_BASE + EISDIR)
#define LWCA_ERRNO_EINVAL                   (LWCA_ERROR_BASE + LWCA_ERRNO_BASE + EINVAL)
#define LWCA_ERRNO_ENFILE                   (LWCA_ERROR_BASE + LWCA_ERRNO_BASE + ENFILE)
#define LWCA_ERRNO_EMFILE                   (LWCA_ERROR_BASE + LWCA_ERRNO_BASE + EMFILE)
#define LWCA_ERRNO_ENOTTY                   (LWCA_ERROR_BASE + LWCA_ERRNO_BASE + ENOTTY)
#define LWCA_ERRNO_ETXTBSY                  (LWCA_ERROR_BASE + LWCA_ERRNO_BASE + ETXTBSY)
#define LWCA_ERRNO_EFBIG                    (LWCA_ERROR_BASE + LWCA_ERRNO_BASE + EFBIG)
#define LWCA_ERRNO_ENOSPC                   (LWCA_ERROR_BASE + LWCA_ERRNO_BASE + ENOSPC)
#define LWCA_ERRNO_ESPIPE                   (LWCA_ERROR_BASE + LWCA_ERRNO_BASE + ESPIPE)
#define LWCA_ERRNO_EROFS                    (LWCA_ERROR_BASE + LWCA_ERRNO_BASE + EROFS)
#define LWCA_ERRNO_EMLINK                   (LWCA_ERROR_BASE + LWCA_ERRNO_BASE + EMLINK)
#define LWCA_ERRNO_EPIPE                    (LWCA_ERROR_BASE + LWCA_ERRNO_BASE + EPIPE)
#define LWCA_ERRNO_EDOM                     (LWCA_ERROR_BASE + LWCA_ERRNO_BASE + EDOM)
#define LWCA_ERRNO_ERANGE                   (LWCA_ERROR_BASE + LWCA_ERRNO_BASE + ERANGE)

// REST Error Codes (80700 - 80799)
#define LWCA_ERROR_INVALID_URI              (LWCA_ERROR_BASE + LWCA_REST_ERROR_BASE + 1)
#define LWCA_ERROR_MISSING_PARAMETER        (LWCA_ERROR_BASE + LWCA_REST_ERROR_BASE + 2)
#define LWCA_ERROR_INVALID_METHOD           (LWCA_ERROR_BASE + LWCA_REST_ERROR_BASE + 3)
#define LWCA_ERROR_INVALID_REQUEST          (LWCA_ERROR_BASE + LWCA_REST_ERROR_BASE + 4)
#define LWCA_ERROR_UNAVAILABLE              (LWCA_ERROR_BASE + LWCA_REST_ERROR_BASE + 5)
#define LWCA_ERROR_REST_UNAUTHENTICATED     (LWCA_ERROR_BASE + LWCA_REST_ERROR_BASE + 6)

// Storage Error codes (80600 - 80699)
#define LWCA_DB_NOT_INITIALIZED             (LWCA_ERROR_BASE + LWCA_STORAGE_ERROR_BASE + 1)
#define LWCA_DB_ALREADY_INITIALIZED         (LWCA_ERROR_BASE + LWCA_STORAGE_ERROR_BASE + 2)
#define LWCA_DB_INVALID_PLUGIN              (LWCA_ERROR_BASE + LWCA_STORAGE_ERROR_BASE + 3)

// OIDC Error codes (80900 - 80999)
#define LWCA_ERROR_OIDC_UNAVAILABLE         (LWCA_ERROR_BASE + LWCA_OIDC_ERROR_BASE + 1)
#define LWCA_ERROR_OIDC_BAD_AUTH_DATA       (LWCA_ERROR_BASE + LWCA_OIDC_ERROR_BASE + 2)
#define LWCA_ERROR_OIDC_UNKNOWN_TOKEN       (LWCA_ERROR_BASE + LWCA_OIDC_ERROR_BASE + 3)
#define LWCA_OIDC_RESPONSE_ERROR            (LWCA_ERROR_BASE + LWCA_OIDC_ERROR_BASE + 4)
#define LWCA_ERROR_OIDC_INVALID_POP         (LWCA_ERROR_BASE + LWCA_OIDC_ERROR_BASE + 5)

// Security error Codes (81000 - 81099)
#define LWCA_SECURITY_NOT_INITIALIZED       (LWCA_ERROR_BASE + LWCA_SECURITY_ERROR_BASE + 1)
#define LWCA_SECURITY_ALREADY_INITIALIZED   (LWCA_ERROR_BASE + LWCA_SECURITY_ERROR_BASE + 2)
#define LWCA_SECURITY_INVALID_PLUGIN        (LWCA_ERROR_BASE + LWCA_SECURITY_ERROR_BASE + 3)
#define LWCA_SECURITY_KEY_ALREADY_IN_CACHE  (LWCA_ERROR_BASE + LWCA_SECURITY_ERROR_BASE + 4)
#define LWCA_SECURITY_KEY_NOT_IN_CACHE      (LWCA_ERROR_BASE + LWCA_SECURITY_ERROR_BASE + 5)
#define LWCA_SECURITY_KEY_NOT_IN_DB         (LWCA_ERROR_BASE + LWCA_SECURITY_ERROR_BASE + 6)

// CURL Error codes (81100 - 81199)
#define LWCA_ERROR_CURL_FAILED_INIT         (LWCA_ERROR_BASE + LWCA_CURL_ERROR_BASE + 1)
#define LWCA_ERROR_CURL_SEND_ERROR          (LWCA_ERROR_BASE + LWCA_CURL_ERROR_BASE + 2)
#define LWCA_ERROR_CURL_RECV_ERROR          (LWCA_ERROR_BASE + LWCA_CURL_ERROR_BASE + 3)
#define LWCA_ERROR_CURL_GENERIC_ERROR       (LWCA_ERROR_BASE + LWCA_CURL_ERROR_BASE + 4)

// Regex Error Codes (81200 - 81299)
#define LWCA_REGEX_ERROR_NOMATCH            (LWCA_ERROR_BASE + LWCA_REGEX_ERROR_BASE + 1)
#define LWCA_REGEX_ERROR_BADPAT             (LWCA_ERROR_BASE + LWCA_REGEX_ERROR_BASE + 2)
#define LWCA_REGEX_ERROR_ECOLLATE           (LWCA_ERROR_BASE + LWCA_REGEX_ERROR_BASE + 3)
#define LWCA_REGEX_ERROR_ECTYPE             (LWCA_ERROR_BASE + LWCA_REGEX_ERROR_BASE + 4)
#define LWCA_REGEX_ERROR_EESCAPE            (LWCA_ERROR_BASE + LWCA_REGEX_ERROR_BASE + 5)
#define LWCA_REGEX_ERROR_ESUBREG            (LWCA_ERROR_BASE + LWCA_REGEX_ERROR_BASE + 6)
#define LWCA_REGEX_ERROR_EBRACK             (LWCA_ERROR_BASE + LWCA_REGEX_ERROR_BASE + 7)
#define LWCA_REGEX_ERROR_EPAREN             (LWCA_ERROR_BASE + LWCA_REGEX_ERROR_BASE + 8)
#define LWCA_REGEX_ERROR_EBRACE             (LWCA_ERROR_BASE + LWCA_REGEX_ERROR_BASE + 9)
#define LWCA_REGEX_ERROR_BADBR              (LWCA_ERROR_BASE + LWCA_REGEX_ERROR_BASE + 10)
#define LWCA_REGEX_ERROR_ERANGE             (LWCA_ERROR_BASE + LWCA_REGEX_ERROR_BASE + 11)
#define LWCA_REGEX_ERROR_ESPACE             (LWCA_ERROR_BASE + LWCA_REGEX_ERROR_BASE + 12)
#define LWCA_REGEX_ERROR_BADRPT             (LWCA_ERROR_BASE + LWCA_REGEX_ERROR_BASE + 13)
#define LWCA_REGEX_ERROR_UNKNOWN            (LWCA_ERROR_BASE + LWCA_REGEX_ERROR_BASE + 14)

// LDAP Error codes (81300 - 81399)
#define LWCA_LDAP_ADD_FAILED                (LWCA_ERROR_BASE + LWCA_LDAP_ERROR_BASE + 1)
#define LWCA_LDAP_GET_FAILED                (LWCA_ERROR_BASE + LWCA_LDAP_ERROR_BASE + 2)
#define LWCA_LDAP_PATCH_FAILED              (LWCA_ERROR_BASE + LWCA_LDAP_ERROR_BASE + 3)
#define LWCA_LDAP_UNKNOWN_OP                (LWCA_ERROR_BASE + LWCA_LDAP_ERROR_BASE + 4)
#define LWCA_LDAP_DELETE_FAILED             (LWCA_ERROR_BASE + LWCA_LDAP_ERROR_BASE + 5)

// Misc. Error Codes (82000 - 82999)
#define LWCA_UNKNOWN_ERROR                  (LWCA_ERROR_BASE + LWCA_MISC_ERROR_BASE + 1)
#define LWCA_JSON_FILE_LOAD_ERROR           (LWCA_ERROR_BASE + LWCA_MISC_ERROR_BASE + 2)
#define LWCA_JSON_PARSE_ERROR               (LWCA_ERROR_BASE + LWCA_MISC_ERROR_BASE + 3)
#define LWCA_JSON_ERROR                     (LWCA_ERROR_BASE + LWCA_MISC_ERROR_BASE + 4)
#define LWCA_PLUGIN_FAILURE                 (LWCA_ERROR_BASE + LWCA_MISC_ERROR_BASE + 5)
#define LWCA_COAPI_ERROR                    (LWCA_ERROR_BASE + LWCA_MISC_ERROR_BASE + 6)
#define LWCA_CREST_ENGINE_ERROR             (LWCA_ERROR_BASE + LWCA_MISC_ERROR_BASE + 7)
#define LWCA_ERROR_VMAFD_UNAVAILABLE        (LWCA_ERROR_BASE + LWCA_MISC_ERROR_BASE + 9)
#define LWCA_ERROR_UUID_GENERATE            (LWCA_ERROR_BASE + LWCA_MISC_ERROR_BASE + 10)
#define LWCA_LOCK_APPLY_FAILED              (LWCA_ERROR_BASE + LWCA_MISC_ERROR_BASE + 11)

#define LWCA_ERRNO_TO_LWCAERROR(err)                                        \
    ((err) ? (LWCA_ERROR_BASE + LWCA_ERRNO_BASE + (err)) : (LWCA_SUCCESS))

#define LWCA_ERROR_TABLE_INITIALIZER \
{ \
    { LWCA_SUCCESS                      ,   "LWCA_SUCCESS"                      ,   "The request succeded without any errors"}, \
    { LWCA_ROOT_CA_MISSING              ,   "LWCA_ROOT_CA_MISSING"              ,   "The Root CA certificate is missing or failed to Initialize" }, \
    { LWCA_ROOT_CA_ALREADY_EXISTS       ,   "LWCA_ROOT_CA_ALREADY_EXISTS"       ,   "Root CA Certificate is already present, Please use --force if you want to overwrite." }, \
    { LWCA_CA_MISSING                   ,   "LWCA_CA_MISSING"                   ,   "The CA certificate is missing or failed to Initialize" }, \
    { LWCA_CA_ALREADY_EXISTS            ,   "LWCA_CA_ALREADY_EXISTS"            ,   "CA Certificate is already present." }, \
    { LWCA_CA_REVOKED                   ,   "LWCA_CA_REVOKED"                   ,   "The CA is revoked" }, \
    { LWCA_CA_ALREADY_REVOKED           ,   "LWCA_CA_ALREADY_REVOKED"           ,   "The CA is already revoked" }, \
    { LWCA_INVALID_CA_DATA              ,   "LWCA_INVALID_CA_DATA"              ,   "CA data is missing or not initialized." }, \
    { LWCA_INVALID_TIME_SPECIFIED       ,   "LWCA_INVALID_TIME_SPECIFIED"       ,   "Invalid time specified for the Certififcate" }, \
    { LWCA_ERROR_INVALID_PARAMETER      ,   "LWCA_ERROR_INVALID_PARAMETER"      ,   "Invalid parameter presented" }, \
    { LWCA_ERROR_TIME_OUT               ,   "LWCA_ERROR_TIME_OUT"               ,   "Time out occurred before specified Event." }, \
    { LWCA_OUT_OF_MEMORY_ERROR          ,   "LWCA_OUT_OF_MEMORY_ERROR"          ,   "Unable to allocate Memory" }, \
    { LWCA_REQUEST_ERROR                ,   "LWCA_REQUEST_ERROR"                ,   "Unable decode CSR" }, \
    { LWCA_KEY_CREATION_FAILURE         ,   "LWCA_KEY_CREATION_FAILURE"         ,   "Key Creation failure" }, \
    { LWCA_CERT_DECODE_FAILURE          ,   "LWCA_CERT_DECODE_FAILURE"          ,   "Cert Decode failure" }, \
    { LWCA_KEY_IO_FAILURE               ,   "LWCA_KEY_IO_FAILURE"               ,   "Key I/O failure" }, \
    { LWCA_CERT_IO_FAILURE              ,   "LWCA_CERT_IO_FAILURE"              ,   "Cert I/O failure" }, \
    { LWCA_NOT_CA_CERT                  ,   "LWCA_NOT_CA_CERT"                  ,   "Not a CA Cert" }, \
    { LWCA_INVALID_CSR_FIELD            ,   "LWCA_INVALID_CSR_FIELD"            ,   "Invalid CSR field" }, \
    { LWCA_SELF_SIGNATURE_FAILED        ,   "LWCA_SELF_SIGNATURE_FAILED"        ,   "Self Signature failed" }, \
    { LWCA_INIT_CA_FAILED               ,   "LWCA_INIT_CA_FAILED"               ,   "Init CA failure" }, \
    { LWCA_ERROR_INVALID_KEY_LENGTH     ,   "LWCA_ERROR_INVALID_KEY_LENGTH"     ,   "Key length has to be between 2048(2KB) and 16384(16KB)" }, \
    { LWCA_PKCS12_CREAT_FAIL            ,   "LWCA_PKCS12_CREAT_FAIL"            ,   "PKCS12 creation Failure" }, \
    { LWCA_PKCS12_IO_FAIL               ,   "LWCA_PKCS12_IO_FAIL"               ,   "PCKS12 I/O failure" }, \
    { LWCA_CRL_ERROR                    ,   "LWCA_CRL_ERROR"                    ,   "CRL update failed" }, \
    { LWCA_NO_NEW_CRL                   ,   "LWCA_NO_NEW_CRL"                   ,   "Client already has the latest CRL" }, \
    { LWCA_CRL_LOCAL_ERROR              ,   "LWCA_CRL_LOCAL_ERROR"              ,   "Failed in File I/O, Please Check Path / Permission" }, \
    { LWCA_FILE_IO_ERROR                ,   "LWCA_FILE_IO_ERROR"                ,   "File I/O Error" }, \
    { LWCA_FILE_TIME_ERROR              ,   "LWCA_FILE_TIME_ERROR"              ,   "Unable to parse time" }, \
    { LWCA_FILE_REMOVE_ERROR            ,   "LWCA_FILE_REMOVE_ERROR"            ,   "Unable to Remove File" }, \
    { LWCA_INVALID_DOMAIN_NAME          ,   "LWCA_INVALID_DOMAIN_NAME"          ,   "Invalid Domain Name" }, \
    { LWCA_INVALID_USER_NAME            ,   "LWCA_INVALID_USER_NAME"            ,   "Invalid User Name" }, \
    { LWCA_UNABLE_GET_CRED_CACHE_NAME   ,   "LWCA_UNABLE_GET_CRED_CACHE_NAME"   ,   "Failed to get cache Name" }, \
    { LWCA_NO_CACHE_FOUND               ,   "LWCA_NO_CACHE_FOUND"               ,   "Cred Cache not found" }, \
    { LWCA_GET_ADDR_INFO_FAIL           ,   "LWCA_GET_ADDR_INFO_FAIL"           ,   "getaddrinfo failure" }, \
    { LWCA_NOT_IMPLEMENTED              ,   "LWCA_NOT_IMPLEMENTED"              ,   "Not Implemented" }, \
    { LWCA_GET_NAME_INFO_FAIL           ,   "LWCA_GET_NAME_INFO_FAIL"           ,   "getnameinfo failure" }, \
    { LWCA_SSL_SET_PUBKEY_ERR           ,   "LWCA_SSL_SET_PUBKEY_ERR"           ,   "Set Public Key Failed" }, \
    { LWCA_SSL_ADD_EXTENSION            ,   "LWCA_SSL_ADD_EXTENSION"            ,   "Adding Extesions to cert failed" }, \
    { LWCA_SSL_REQ_SIGN_ERR             ,   "LWCA_SSL_REQ_SIGN_ERR"             ,   "Request Signing Failed" }, \
    { LWCA_SSL_RAND_ERR                 ,   "LWCA_SSL_RAND_ERR"                 ,   "Rand generation failed" }, \
    { LWCA_SSL_CERT_SIGN_ERR            ,   "LWCA_SSL_CERT_SIGN_ERR"            ,   "Certificate Signing failed" }, \
    { LWCA_SSL_TIME_ERROR               ,   "LWCA_SSL_TIME_ERROR"               ,   "Invalid Time Argument" }, \
    { LWCA_SSL_EXT_ERR                  ,   "LWCA_SSL_EXT_ERR"                  ,   "Unable to add this Extension" }, \
    { LWCA_SSL_SIGN_FAIL                ,   "LWCA_SSL_SIGN_FAIL"                ,   "Failed to sign the certificate" }, \
    { LWCA_SSL_SET_ISSUER_NAME          ,   "LWCA_SSL_SET_ISSUER_NAME"          ,   "Failed to set issuer name" }, \
    { LWCA_SSL_SET_START_TIME           ,   "LWCA_SSL_SET_START_TIME"           ,   "Start Time Error" }, \
    { LWCA_SSL_SET_END_TIME             ,   "LWCA_SSL_SET_END_TIME"             ,   "End Time Error" }, \
    { LWCA_SSL_SET_EXT_ERR              ,   "LWCA_SSL_SET_EXT_ERR"              ,   "Set Extenion Failed" }, \
    { LWCA_CERT_PRIVATE_KEY_MISMATCH    ,   "LWCA_CERT_PRIVATE_KEY_MISMATCH"    ,   "Cert/Key pair does not match" }, \
    { LWCA_INVALID_DOMAIN_NAME          ,   "LWCA_INVALID_DOMAIN_NAME"          ,   "Domain name error" }, \
    { LWCA_INVALID_USER_NAME            ,   "LWCA_INVALID_USER_NAME"            ,   "User name error" }, \
    { LWCA_UNABLE_GET_CRED_CACHE_NAME   ,   "LWCA_UNABLE_GET_CRED_CACHE_NAME"   ,   "Krb cred cache name error" }, \
    { LWCA_NO_CACHE_FOUND               ,   "LWCA_NO_CACHE_FOUND"               ,   "Krb cache not found" }, \
    { LWCA_KRB_ACCESS_DENIED            ,   "LWCA_KRB_ACCESS_DENIED"            ,   "Kerb access denied" }, \
    { LWCA_ACCESS_DENIED                ,   "LWCA_ACCESS_DENIED"                ,   "Access denied" }, \
    { LWCA_ERROR_AUTHZ_INITIALIZED      ,   "LWCA_ERROR_AUTHZ_INITIALIZED"      ,   "AuthZ context is already initialized" }, \
    { LWCA_ERROR_AUTHZ_UNINITIALIZED    ,   "LWCA_ERROR_AUTHZ_UNINITIALIZED"    ,   "AuthZ context is not initialized" }, \
    { LWCA_ERROR_AUTHZ_UNAUTHORIZED     ,   "LWCA_ERROR_AUTHZ_UNAUTHORIZED"     ,   "Requestor is not authorized" }, \
    { LWCA_ERROR_AUTHZ_INVALID_PLUGIN   ,   "LWCA_ERROR_AUTHZ_INVALID_PLUGIN"   ,   "AuthZ plugin is not valid" }, \
    { LWCA_GET_ADDR_INFO_FAIL           ,   "LWCA_GET_ADDR_INFO_FAIL"           ,   "Network - Get addr info call failed" }, \
    { LWCA_NOT_IMPLEMENTED              ,   "LWCA_NOT_IMPLEMENTED"              ,   "Not implemented" }, \
    { LWCA_GET_NAME_INFO_FAIL           ,   "LWCA_GET_NAME_INFO_FAIL"           ,   "Network - Get name info call failed" }, \
    { LWCA_CRL_SET_SERIAL_FAIL          ,   "LWCA_CRL_SET_SERIAL_FAIL"          ,   "CRL - Setting serial number failed" }, \
    { LWCA_CRL_SET_TIME_FAIL            ,   "LWCA_CRL_SET_TIME_FAIL"            ,   "CRL - Setting time failed" }, \
    { LWCA_CRL_CERT_ALREADY_REVOKED     ,   "LWCA_CRL_CERT_ALREADY_REVOKED"     ,   "This is already revoked cert" }, \
    { LWCA_CRLNUMBER_READ_ERROR         ,   "LWCA_CRLNUMBER_READ_ERROR"         ,   "Unable to read CRL serial" }, \
    { LWCA_CRL_SIGN_FAIL                ,   "LWCA_CRL_SIGN_FAIL"                ,   "CRL - signing failed" }, \
    { LWCA_CRL_REASON_FAIL              ,   "LWCA_CRL_REASON_FAIL"              ,   "CRL - Unable to set reason" }, \
    { LWCA_CRL_SORT_FAILED              ,   "LWCA_CRL_SORT_FAILED"              ,   "CRL - Sorting failed" }, \
    { LWCA_CRL_NULL_TIME                ,   "LWCA_CRL_NULL_TIME"                ,   "CRL - Null time encountered" }, \
    { LWCA_CRL_DECODE_ERROR             ,   "LWCA_CRL_DECODE_ERROR"             ,   "CRL - Unable to decode CRL" }, \
    { LWCA_VPX_RSUTIL_ERROR             ,   "LWCA_VPX_RSUTIL_ERROR"             ,   "Unable to find a dependency." }, \
    { LWCA_DIR_CREATE_ERROR             ,   "LWCA_DIR_CREATE_ERROR"             ,   "Directory creation failed" }, \
    { LWCA_LDAP_UPN_FAIL                ,   "LWCA_LDAP_UPN_FAIL"                ,   "LDAP call to dn2upn failed." }, \
    { LWCA_ERROR_INVALID_SN             ,   "LWCA_ERROR_INVALID_SN"             ,   "Invalid Subject Name specified" }, \
    { LWCA_ERROR_INVALID_SAN            ,   "LWCA_ERROR_INVALID_SAN"            ,   "Invalid Subject Alternate Name specified." }, \
    { LWCA_ERROR_INVALID_CERTIFICATE    ,   "LWCA_ERROR_INVALID_CERTIFICATE"    ,   "Certificate is not valid." }, \
    { LWCA_ERROR_INCOMPLETE_CHAIN       ,   "LWCA_ERROR_INCOMPLETE_CERT_CHAIN"  ,   "Certificate Chain is not complete" }, \
    { LWCA_ERROR_INVALID_CHAIN          ,   "LWCA_ERROR_INVALID_CERT_CHAIN"     ,   "Invalid Certificate Chain was gives as input" }, \
    { LWCA_ERROR_CANNOT_FORM_REQUEST    ,   "LWCA_ERROR_CANNOT_FORM_REQUEST"    ,   "Could not create the CSR from the certificate" }, \
    { LWCA_KEY_DECODE_FAILURE           ,   "LWCA_KEY_DECODE_FAILURE"           ,   "Could not decode the Private Key from the given format" }, \
    { LWCA_ERROR_CN_HOSTNAME_MISMATCH   ,   "LWCA_ERROR_CN_HOSTNAME_MISMATCH"   ,   "CSR CN does not match to hostname" }, \
    { LWCA_ERROR_SAN_HOSTNAME_MISMATCH  ,   "LWCA_ERROR_SAN_HOSTNAME_MISMATCH"  ,   "CSR SAN does not match to hostname" }, \
    { LWCA_ERROR_SAN_IPADDR_INVALID     ,   "LWCA_ERROR_SAN_IPADDR_INVALID"     ,   "CSR SAN has an invalid ip" }, \
    { LWCA_OPENSSL_ERROR                ,   "LWCA_OPENSSL_ERROR"                ,   "OpenSSL Error" }, \
    { LWCA_ERROR_INVALID_DN             ,   "LWCA_ERROR_INVALID_DN"             ,   "Invalid DN specified" }, \
    { LWCA_SSL_STORE_ADD_CERT_FAIL      ,   "LWCA_SSL_STORE_ADD_CERT_FAIL"      ,   "Adding certificate to SSL store failed" }, \
    { LWCA_SSL_STORE_CTX_INIT_FAIL      ,   "LWCA_SSL_STORE_CTX_INIT_FAIL"      ,   "Init SSL store context failed" }, \
    { LWCA_SSL_CERT_VERIFY_ERR          ,   "LWCA_SSL_CERT_VERIFY_ERR"          ,   "Unable to verify certficate" }, \
    { LWCA_SSL_INVALID_NID              ,   "LWCA_SSL_INVALID_NID"              ,   "NID doesn't correspond to a valid OID" }, \
    { LWCA_SSL_NO_EXTENSIONS            ,   "LWCA_SSL_NO_EXTENSIONS"            ,   "Could not get any extensions" }, \
    { LWCA_ERROR_INVALID_URI            ,   "LWCA_ERROR_INVALID_URI"            ,   "Unknown request URI" }, \
    { LWCA_ERROR_MISSING_PARAMETER      ,   "LWCA_ERROR_MISSING_PARAMETER"      ,   "Missing expected parameter" }, \
    { LWCA_ERROR_INVALID_METHOD         ,   "LWCA_ERROR_INVALID_METHOD"         ,   "Invalid HTTP method" }, \
    { LWCA_ERROR_CANNOT_LOAD_LIBRARY    ,   "LWCA_ERROR_CANNOT_LOAD_LIBRARY"    ,   "Unable to load library" }, \
    { LWCA_ERROR_NO_FILE_OR_DIRECTORY   ,   "LWCA_ERROR_NO_FILE_OR_DIRECTORY"   ,   "Unable to find the specified file or directory" }, \
    { LWCA_ERROR_AUTH_BAD_DATA          ,   "LWCA_ERROR_AUTH_BAD_DATA"          ,   "Bad auth data presented" }, \
    { LWCA_ERROR_INVALID_REQUEST        ,   "LWCA_ERROR_INVALID_REQUEST"        ,   "Bad request caused by client error"}, \
    { LWCA_ERROR_UNAVAILABLE            ,   "LWCA_ERROR_UNAVAILABLE"            ,   "Server is unavailable/shutdown"}, \
    { LWCA_ERROR_REST_UNAUTHENTICATED   ,   "LWCA_ERROR_REST_UNAUTHENTICATED"   ,   "Unauthenticated HTTP request"}, \
    { LWCA_ERROR_DLL_SYMBOL_NOTFOUND    ,   "LWCA_ERROR_DLL_SYMBOL_NOTFOUND"    ,   "Unable to find symbol in library" }, \
    { LWCA_ERROR_EVP_DIGEST             ,   "LWCA_ERROR_EVP_DIGEST"             ,   "Error processing EVP digest" }, \
    { LWCA_JSON_FILE_LOAD_ERROR         ,   "LWCA_JSON_FILE_LOAD_ERROR"         ,   "Unable to load JSON file" }, \
    { LWCA_JSON_PARSE_ERROR             ,   "LWCA_JSON_PARSE_ERROR"             ,   "Failed to parse JSON file" }, \
    { LWCA_SN_POLICY_VIOLATION          ,   "LWCA_SN_POLICY_VIOLATION"          ,   "Request does not comply with the defined SN policies" }, \
    { LWCA_SAN_POLICY_VIOLATION         ,   "LWCA_SAN_POLICY_VIOLATION"         ,   "Request does not comply with the defined SAN policies" }, \
    { LWCA_KEY_USAGE_POLICY_VIOLATION   ,   "LWCA_KEY_USAGE_POLICY_VIOLATION"   ,   "Request does not comply with the defined KeyUsage policies" }, \
    { LWCA_CERT_DURATION_POLICY_VIOLATION,  "LWCA_CERT_DURATION_POLICY_VIOLATION",  "Request does not comply with the defined CertDuration policies" }, \
    { LWCA_POLICY_CONFIG_PARSE_ERROR    ,   "LWCA_POLICY_CONFIG_PARSE_ERROR"    ,   "Invalid content in policy config file" }, \
    { LWCA_DB_NOT_INITIALIZED           ,   "LWCA_DB_NOT_INITIALIZED"           ,   "Db context is not initialized" }, \
    { LWCA_DB_ALREADY_INITIALIZED       ,   "LWCA_DB_ALREADY_INITIALIZED"       ,   "Db context is already initialized" }, \
    { LWCA_DB_INVALID_PLUGIN            ,   "LWCA_DB_INVALID_PLUGIN"            ,   "Db plugin is not valid" }, \
    { LWCA_ERROR_ENTRY_NOT_FOUND        ,   "LWCA_ERROR_ENTRY_NOT_FOUND"        ,   "Unable to find requested entry" }, \
    { LWCA_ERROR_INVALID_STATE          ,   "LWCA_ERROR_INVALID_STATE"          ,   "Invalid state of service" }, \
    { LWCA_ERROR_INVALID_ENTRY          ,   "LWCA_ERROR_INVALID_ENTRY"          ,   "Requested entry is invalid" }, \
    { LWCA_ERROR_INVALID_DATA           ,   "LWCA_ERROR_INVALID_DATA"           ,   "Invalid data" }, \
    { LWCA_ERROR_BUFFER_OVERFLOW        ,   "LWCA_ERROR_BUFFER_OVERFLOW"        ,   "Buffer overflow" }, \
    { LWCA_ERRNO_EPERM                  ,   "LWCA_ERRNO_EPERM"                  ,   "Operation not permitted" }, \
    { LWCA_ERRNO_ENOENT                 ,   "LWCA_ERRNO_ENOENT"                 ,   "No such file or directory" }, \
    { LWCA_ERRNO_ESRCH                  ,   "LWCA_ERRNO_ESRCH"                  ,   "No such process" }, \
    { LWCA_ERRNO_EINTR                  ,   "LWCA_ERRNO_EINTR"                  ,   "Interrupted system call" }, \
    { LWCA_ERRNO_EIO                    ,   "LWCA_ERRNO_EIO"                    ,   "I/O error" }, \
    { LWCA_ERRNO_ENXIO                  ,   "LWCA_ERRNO_ENXIO"                  ,   "No such device or address" }, \
    { LWCA_ERRNO_E2BIG                  ,   "LWCA_ERRNO_E2BIG"                  ,   "Argument list too long" }, \
    { LWCA_ERRNO_ENOEXEC                ,   "LWCA_ERRNO_ENOEXEC"                ,   "Exec format error" }, \
    { LWCA_ERRNO_EBADF                  ,   "LWCA_ERRNO_EBADF"                  ,   "Bad file number" }, \
    { LWCA_ERRNO_ECHILD                 ,   "LWCA_ERRNO_ECHILD"                 ,   "No child processes" }, \
    { LWCA_ERRNO_EAGAIN                 ,   "LWCA_ERRNO_EAGAIN"                 ,   "Try again" }, \
    { LWCA_ERRNO_ENOMEM                 ,   "LWCA_ERRNO_ENOMEM"                 ,   "Out of memory" }, \
    { LWCA_ERRNO_EACCES                 ,   "LWCA_ERRNO_EACCES"                 ,   "Permission denied" }, \
    { LWCA_ERRNO_EFAULT                 ,   "LWCA_ERRNO_EFAULT"                 ,   "Bad address" }, \
    { LWCA_ERRNO_ENOTBLK                ,   "LWCA_ERRNO_ENOTBLK"                ,   "Block device required" }, \
    { LWCA_ERRNO_EBUSY                  ,   "LWCA_ERRNO_EBUSY"                  ,   "Device or resource busy" }, \
    { LWCA_ERRNO_EEXIST                 ,   "LWCA_ERRNO_EEXIST"                 ,   "File exists" }, \
    { LWCA_ERRNO_EXDEV                  ,   "LWCA_ERRNO_EXDEV"                  ,   "Cross-device link" }, \
    { LWCA_ERRNO_ENODEV                 ,   "LWCA_ERRNO_ENODEV"                 ,   "No such device" }, \
    { LWCA_ERRNO_ENOTDIR                ,   "LWCA_ERRNO_ENOTDIR"                ,   "Not a directory" }, \
    { LWCA_ERRNO_EISDIR                 ,   "LWCA_ERRNO_EISDIR"                 ,   "Is a directory" }, \
    { LWCA_ERRNO_EINVAL                 ,   "LWCA_ERRNO_EINVAL"                 ,   "Invalid argument" }, \
    { LWCA_ERRNO_ENFILE                 ,   "LWCA_ERRNO_ENFILE"                 ,   "File table overflow" }, \
    { LWCA_ERRNO_EMFILE                 ,   "LWCA_ERRNO_EMFILE"                 ,   "Too many open files" }, \
    { LWCA_ERRNO_ENOTTY                 ,   "LWCA_ERRNO_ENOTTY"                 ,   "Not a typewriter" }, \
    { LWCA_ERRNO_ETXTBSY                ,   "LWCA_ERRNO_ETXTBSY"                ,   "Text file busy" }, \
    { LWCA_ERRNO_EFBIG                  ,   "LWCA_ERRNO_EFBIG"                  ,   "File too large" }, \
    { LWCA_ERRNO_ENOSPC                 ,   "LWCA_ERRNO_ENOSPC"                 ,   "No space left on device" }, \
    { LWCA_ERRNO_ESPIPE                 ,   "LWCA_ERRNO_ESPIPE"                 ,   "Illegal seek" }, \
    { LWCA_ERRNO_EROFS                  ,   "LWCA_ERRNO_EROFS"                  ,   "Read-only file system" }, \
    { LWCA_ERRNO_EMLINK                 ,   "LWCA_ERRNO_EMLINK"                 ,   "Too many links" }, \
    { LWCA_ERRNO_EPIPE                  ,   "LWCA_ERRNO_EPIPE"                  ,   "Broken pipe" }, \
    { LWCA_ERRNO_EDOM                   ,   "LWCA_ERRNO_EDOM"                   ,   "Math argument out of domain func" }, \
    { LWCA_ERRNO_ERANGE                 ,   "LWCA_ERRNO_ERANGE"                 ,   "Math result not representable" }, \
    { LWCA_UNKNOWN_ERROR                ,   "LWCA_UNKNOWN_ERROR"                ,   "Certificate Server Unknown Error" }, \
    { LWCA_JSON_ERROR                   ,   "LWCA_JSON_ERROR"                   ,   "Error from jansson api" }, \
    { LWCA_COAPI_ERROR                  ,   "LWCA_COAPI_ERROR"                  ,   "Error from copenapi" }, \
    { LWCA_CREST_ENGINE_ERROR           ,   "LWCA_CREST_ENGINE_ERROR"           ,   "Error from c-rest-engine" }, \
    { LWCA_ERROR_OIDC_UNAVAILABLE       ,   "LWCA_ERROR_OIDC_UNAVAILABLE"       ,   "OIDC authentication is not available" }, \
    { LWCA_ERROR_OIDC_BAD_AUTH_DATA     ,   "LWCA_ERROR_OIDC_BAD_AUTH_DATA"     ,   "Bad data presented for OIDC auth" }, \
    { LWCA_ERROR_OIDC_UNKNOWN_TOKEN     ,   "LWCA_ERROR_OIDC_UNKNOWN_TOKEN"     ,   "Unsupported OIDC token type presented" }, \
    { LWCA_ERROR_OIDC_INVALID_POP       ,   "LWCA_ERROR_OIDC_INVALID_POP"       ,   "Failed to verify HTTP request POP" }, \
    { LWCA_OIDC_RESPONSE_ERROR          ,   "LWCA_OIDC_RESPONSE_ERROR"          ,   "Error in the response from OIDC" }, \
    { LWCA_SECURITY_NOT_INITIALIZED     ,   "LWCA_SECURITY_NOT_INITIALIZED"     ,   "Error initializing security plugin" }, \
    { LWCA_SECURITY_ALREADY_INITIALIZED ,   "LWCA_SECURITY_ALREADY_INITIALIZED" ,   "Initialize of security plugin is not allowed when already initialized" }, \
    { LWCA_SECURITY_INVALID_PLUGIN      ,   "LWCA_SECURITY_INVALID_PLUGIN"      ,   "Loaded security plugin is invalid. Check plugin init state." }, \
    { LWCA_SECURITY_KEY_ALREADY_IN_CACHE,   "LWCA_SECURITY_KEY_ALREADY_IN_CACHE",   "An encrypted key is already present in local cache for this ca id." }, \
    { LWCA_SECURITY_KEY_NOT_IN_CACHE    ,   "LWCA_SECURITY_KEY_NOT_IN_CACHE"    ,   "An encrypted key is not present in local cache for this ca id." }, \
    { LWCA_SECURITY_KEY_NOT_IN_DB       ,   "LWCA_SECURITY_KEY_NOT_IN_DB"       ,   "An encrypted key is not present in db for this ca id." }, \
    { LWCA_ERROR_CURL_FAILED_INIT       ,   "LWCA_ERROR_CURL_FAILED_INIT"       ,   "CURL Init Failed" } , \
    { LWCA_ERROR_CURL_SEND_ERROR        ,   "LWCA_ERROR_CURL_SEND_ERROR"        ,   "CURL failed to send request" } , \
    { LWCA_ERROR_CURL_RECV_ERROR        ,   "LWCA_ERROR_CURL_RECV_ERROR"        ,   "CURL failed to receive request" } , \
    { LWCA_ERROR_CURL_GENERIC_ERROR     ,   "LWCA_ERROR_CURL_GENERIC_ERROR"     ,   "CURL generic failure" } , \
    { LWCA_LDAP_ADD_FAILED              ,   "LWCA_LDAP_ADD_FAILED"              ,   "LDAP add could not be completed"}, \
    { LWCA_LDAP_GET_FAILED              ,   "LWCA_LDAP_GET_FAILED"              ,   "LDAP search could not be completed"}, \
    { LWCA_LDAP_PATCH_FAILED            ,   "LWCA_LDAP_PATCH_FAILED"            ,   "LDAP update could not be completed"}, \
    { LWCA_LDAP_UNKNOWN_OP              ,   "LWCA_LDAP_UNKNOWN_OP"              ,   "LDAP operation not known"}, \
    { LWCA_LDAP_DELETE_FAILED           ,   "LWCA_LDAP_DELETE_FAILED"           ,   "LDAP delete could not be completed"}, \
    { LWCA_ERROR_VMAFD_UNAVAILABLE      ,   "LWCA_ERROR_VMAFD_UNAVAILABLE"      ,   "Error calling libvmafdclient function" }, \
    { LWCA_ERROR_UUID_GENERATE          ,   "LWCA_ERROR_UUID_GENERATE"          ,   "Error while generating UUID" }, \
    { LWCA_LOCK_APPLY_FAILED            ,   "LWCA_LOCK_APPLY_FAILED"            ,   "Could not apply lock on DN" }, \
    { LWCA_REGEX_ERROR_NOMATCH          ,   "LWCA_REGEX_ERROR_NOMATCH"          ,   "regexec() failed to match" }, \
    { LWCA_REGEX_ERROR_BADPAT           ,   "LWCA_REGEX_ERROR_BADPAT"           ,   "Invalid regular expression" }, \
    { LWCA_REGEX_ERROR_ECOLLATE         ,   "LWCA_REGEX_ERROR_ECOLLATE"         ,   "Invalid collating element referenced" }, \
    { LWCA_REGEX_ERROR_ECTYPE           ,   "LWCA_REGEX_ERROR_ECTYPE"           ,   "Invalid character class type referenced" }, \
    { LWCA_REGEX_ERROR_EESCAPE          ,   "LWCA_REGEX_ERROR_EESCAPE"          ,   "Trailing '\\' in pattern" }, \
    { LWCA_REGEX_ERROR_ESUBREG          ,   "LWCA_REGEX_ERROR_ESUBREG"          ,   "Number in '\\digit' invalid or in error" }, \
    { LWCA_REGEX_ERROR_EBRACK           ,   "LWCA_REGEX_ERROR_EBRACK"           ,   "'[]' imbalance" }, \
    { LWCA_REGEX_ERROR_EPAREN           ,   "LWCA_REGEX_ERROR_EPAREN"           ,   "'\\(\\)' or '()' imbalance" }, \
    { LWCA_REGEX_ERROR_EBRACE           ,   "LWCA_REGEX_ERROR_EBRACE"           ,   "'\\{\\}' imbalance" }, \
    { LWCA_REGEX_ERROR_BADBR            ,   "LWCA_REGEX_ERROR_BADBR"            ,   "Content of '\\{\\}' invalid: not a number, number too large, more than two numbers, first larger than second" }, \
    { LWCA_REGEX_ERROR_ERANGE           ,   "LWCA_REGEX_ERROR_ERANGE"           ,   "Invalid endpoint in range expression" }, \
    { LWCA_REGEX_ERROR_ESPACE           ,   "LWCA_REGEX_ERROR_ESPACE"           ,   "Out of memory" }, \
    { LWCA_REGEX_ERROR_BADRPT           ,   "LWCA_REGEX_ERROR_BADRPT"           ,   "'?', '*', or '+' not preceded by valid regular expression" }, \
    { LWCA_REGEX_ERROR_UNKNOWN          ,   "LWCA_REGEX_ERROR_UNKNOWN"          ,   "Unknwown regex error" }, \
};

PCSTR
LwCAGetErrorDescription(
    DWORD dwErrorCode
    );

#endif //__LWCA_ERROR_H__
