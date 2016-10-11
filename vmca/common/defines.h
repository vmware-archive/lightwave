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


#ifndef __VMCA_COMMON_DEFINES_H__
#define __VMCA_COMMON_DEFINES_H__

typedef enum
{
    ATTR_NOT_FOUND = 0,
    ATTR_MATCH,
    ATTR_DIFFER,
} ATTR_SEARCH_RESULT;

#ifndef _WIN32
#define VMCA_PATH_SEPARATOR "/"
#define VMCA_KEY_PARAMETERS "\\Services\\vmca\\Parameters"
#define VMCA_ROOT_CERT "RootCert"
#define VMCA_ROOT_PRIVATE_KEY "RootPrivateKey"
#define VMCA_ROOT_PRIVATE_KEY_PASS_PHRASE "RootPrivateKeyPassPhrase"
#define VMCA_ROOT_CERT_DIR VMCA_DB_DIR
#define VMCA_LOG_DIR "/var/log/vmware"
#else
#define VMCA_PATH_SEPARATOR "\\"
#define VMCA_KEY_PARAMETERS "\\Services\\vmca\\Parameters"
#define VMCA_ROOT_CERT "RootCert"
#define VMCA_ROOT_PRIVATE_KEY "RootPrivateKey"
#define VMCA_ROOT_PRIVATE_KEY_PASS_PHRASE "RootPrivateKeyPassPhrase"
#define VMCA_ROOT_CERT_DIR "C:\\ProgramData\\VMware\\cis\\data\\vmcad"
#define VMCA_LOG_DIR "C:\\ProgramData\\VMware\\CIS\\logs"
#endif

#define ROOT_CER "root.cer"
#define PRIVATE_KEY "privatekey.pem"
#define PRIVATE_PASSWORD "privatekey.password"
#define CERTS_DB "certs.db"
#define VMCA_CRL_NAME "vmca.crl"
#define VMCA_CRL_TMP_NAME "vmca.crl.tmp"
#define VMCA_CRL_DEFAULT_CRL_VALIDITY 30 // days

#define VMCA_ROOT_CERT_FILE VMCA_ROOT_CERT_DIR VMCA_PATH_SEPARATOR ROOT_CER
#define VMCA_ROOT_CERT_PRIVATE_KEY VMCA_ROOT_CERT_DIR VMCA_PATH_SEPARATOR PRIVATE_KEY
#define VMCA_ROOT_CERT_PASSWORD_FILE VMCA_ROOT_CERT_DIR VMCA_PATH_SEPARATOR PRIVATE_PASSWORD
#define VMCA_CERT_DB_FILE VMCA_ROOT_CERT_DIR VMCA_PATH_SEPARATOR CERTS_DB
#define VMCA_CRL_FILE VMCA_ROOT_CERT_DIR VMCA_PATH_SEPARATOR VMCA_CRL_NAME
#define VMCA_CRL_TMP  VMCA_ROOT_CERT_DIR VMCA_PATH_SEPARATOR VMCA_CRL_TMP_NAME

#define ATTR_CA_CERTIFICATE "cACertificate"
#define ATTR_CA_CERTIFICATE_DN "cACertificateDN"
#define ATTR_CN             "CN"
#define ATTR_OBJECTCLASS    "objectclass"
#define ATTR_CRL            "certificateRevocationList"
#define ATTR_NAME_CA_CERTIFICATE_DN "cACertificateDN"

#define CA_CONTAINER_NAME   "Certificate-Authorities"

#define MAX_CN_LENGTH 64
#define VMCA_MIN_CERT_PRIV_KEY_LENGTH        2048

#define VMCA_TIME_SECS_PER_MINUTE           ( 60)
#define VMCA_TIME_SECS_PER_HOUR             ( 60 * VMCA_TIME_SECS_PER_MINUTE)
#define VMCA_TIME_SECS_PER_DAY              ( 24 * VMCA_TIME_SECS_PER_HOUR)
#define VMCA_TIME_SECS_PER_WEEK             (  7 * VMCA_TIME_SECS_PER_DAY)
#define VMCA_TIME_SECS_PER_YEAR             (366 * VMCA_TIME_SECS_PER_DAY)

#define VMCA_VALIDITY_SYNC_BACK_DATE        (VMCA_TIME_SECS_PER_WEEK * 2)
#define VMCA_MAX_CERT_DURATION              (VMCA_TIME_SECS_PER_YEAR * 10)

#endif //__VMCA_COMMON_DEFINES_H__
