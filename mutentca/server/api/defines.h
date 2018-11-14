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

#ifndef _LWCA_SERVICE_API_DEFINES_H_
#define _LWCA_SERVICE_API_DEFINES_H_

#define LWCA_CA_CONFIG_KEY_NAME                     "name"
#define LWCA_CA_CONFIG_KEY_DOMAIN_NAME              "domain"
#define LWCA_CA_CONFIG_KEY_ORGANIZATION             "organization"
#define LWCA_CA_CONFIG_KEY_ORGANIZATION_UNIT        "organizationUnit"
#define LWCA_CA_CONFIG_KEY_COUNTRY                  "country"
#define LWCA_CA_CONFIG_KEY_STATE                    "state"
#define LWCA_CA_CONFIG_KEY_LOCALITY                 "locality"
#define LWCA_CA_CONFIG_KEY_CERTIFICATE_FILE         "certificateFile"
#define LWCA_CA_CONFIG_KEY_PRIVATEKEY_FILE          "privateKeyFile"
#define LWCA_CA_CONFIG_KEY_PASSPHRASE_FILE          "passphraseFile"

#define LWCA_MIN_CA_CERT_PRIV_KEY_LENGTH 2048

#endif //_LWCA_SERVICE_API_DEFINES_H_
