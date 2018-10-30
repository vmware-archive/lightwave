/*
 * Copyright © 2018 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the [0m~@~\License[0m~@~]); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an [0m~@~\AS IS[0m~@~] BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */

// Policy config json keys
#define LWCA_CA_POLICY_KEY                  "CAPolicy"
#define LWCA_CERT_POLICY_KEY                "CertificatePolicy"
#define LWCA_SN_POLICY_KEY                  "SNPolicy"
#define LWCA_SAN_POLICY_KEY                 "SANPolicy"
#define LWCA_KEY_USAGE_POLICY_KEY           "KeyUsagePolicy"
#define LWCA_CERT_DURATION_POLICY_KEY       "CertDurationPolicy"
#define LWCA_ALLOWED_CN_KEY                 "AllowedCNs"
#define LWCA_ALLOWED_SAN_KEY                "AllowedSANs"
#define LWCA_ALLOWED_KEY_USAGES_KEY         "AllowedKeyUsages"
#define LWCA_ALLOWED_DAYS_KEY               "MaxAllowedDays"
#define LWCA_MULTI_SAN_KEY                  "multiSAN"
#define LWCA_CFG_OBJ_TYPE_KEY               "type"
#define LWCA_CFG_OBJ_MATCH_KEY              "match"
#define LWCA_CFG_OBJ_VALUE_KEY              "value"
#define LWCA_CFG_OBJ_PREFIX_KEY             "prefix"
#define LWCA_CFG_OBJ_SUFFIX_KEY             "suffix"

// Policy config possible values for key: type
#define LWCA_TYPE_VALUE_IP                  "ip"
#define LWCA_TYPE_VALUE_NAME                "name"
#define LWCA_TYPE_VALUE_FQDN                "fqdn"

// Policy config possible values for key: match
#define LWCA_MATCH_VALUE_CONSTANT           "constant"
#define LWCA_MATCH_VALUE_ANY                "any"
#define LWCA_MATCH_VALUE_REGEX              "regex"
#define LWCA_MATCH_VALUE_PRIVATE            "private"
#define LWCA_MATCH_VALUE_PUBLIC             "public"
#define LWCA_MATCH_VALUE_INZONE             "inzone"
#define LWCA_MATCH_VALUE_REQ_HOSTNAME       "req.hostname"
#define LWCA_MATCH_VALUE_REQ_FQDN           "req.fqdn"
