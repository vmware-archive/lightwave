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


// Sample policy config jsons used for parser testing

// All valid combinations
#define TEST_POLICY_JSON_VALID_ALL_COMBINATIONS \
            "{\"CAPolicy\":{" \
                "\"SNPolicy\":{" \
                    "\"AllowedCNs\":[]" \
                "}," \
                "\"SANPolicy\":{" \
                    "\"multiSAN\":true," \
                    "\"AllowedSANs\":[" \
                        "{\"type\":\"ip\", \"match\":\"constant\", \"value\":\"127.0.0.1\"}," \
                        "{\"type\":\"ip\", \"match\":\"any\"}," \
                        "{\"type\":\"ip\", \"match\":\"regex\", \"value\":\".*\"}," \
                        "{\"type\":\"ip\", \"match\":\"private\"}," \
                        "{\"type\":\"ip\", \"match\":\"public\"}," \
                        "{\"type\":\"ip\", \"match\":\"inzone\"}," \
                        "{\"type\":\"name\", \"match\":\"constant\", \"value\":\"test\"}," \
                        "{\"type\":\"name\", \"match\":\"any\"}," \
                        "{\"type\":\"name\", \"match\":\"regex\", \"value\":\".*\"}," \
                        "{\"type\":\"name\", \"match\":\"req.hostname\"}," \
                        "{\"type\":\"name\", \"match\":\"req.hostname\", \"prefix\":\"test-\"}," \
                        "{\"type\":\"name\", \"match\":\"req.hostname\", \"suffix\":\"-test\"}," \
                        "{\"type\":\"fqdn\", \"match\":\"constant\", \"value\":\"lw.test.com\"}," \
                        "{\"type\":\"fqdn\", \"match\":\"any\"}," \
                        "{\"type\":\"fqdn\", \"match\":\"regex\", \"value\":\".*\"}," \
                        "{\"type\":\"fqdn\", \"match\":\"inzone\"}," \
                        "{\"type\":\"fqdn\", \"match\":\"req.fqdn\"}," \
                        "{\"type\":\"fqdn\", \"match\":\"req.fqdn\", \"prefix\":\"test.\"}," \
                        "{\"type\":\"fqdn\", \"match\":\"req.fqdn\", \"suffix\":\".test\"}" \
                    "]" \
                "}," \
                "\"KeyUsagePolicy\":{" \
                    "\"AllowedKeyUsages\":255" \
                "}," \
                "\"CertDurationPolicy\":{" \
                    "\"MaxAllowedDays\":3650" \
                "}" \
            "}," \
            "\"CertificatePolicy\":{" \
                "\"SNPolicy\":{" \
                    "\"AllowedCNs\":[" \
                        "{\"type\":\"ip\", \"match\":\"constant\", \"value\":\"127.0.0.1\"}," \
                        "{\"type\":\"ip\", \"match\":\"any\"}," \
                        "{\"type\":\"ip\", \"match\":\"regex\", \"value\":\".*\"}," \
                        "{\"type\":\"ip\", \"match\":\"private\"}," \
                        "{\"type\":\"ip\", \"match\":\"public\"}," \
                        "{\"type\":\"ip\", \"match\":\"inzone\"}," \
                        "{\"type\":\"name\", \"match\":\"constant\", \"value\":\"test\"}," \
                        "{\"type\":\"name\", \"match\":\"any\"}," \
                        "{\"type\":\"name\", \"match\":\"regex\", \"value\":\".*\"}," \
                        "{\"type\":\"name\", \"match\":\"req.hostname\"}," \
                        "{\"type\":\"name\", \"match\":\"req.hostname\", \"prefix\":\"test-\"}," \
                        "{\"type\":\"name\", \"match\":\"req.hostname\", \"suffix\":\"-test\"}," \
                        "{\"type\":\"fqdn\", \"match\":\"constant\", \"value\":\"lw.test.com\"}," \
                        "{\"type\":\"fqdn\", \"match\":\"any\"}," \
                        "{\"type\":\"fqdn\", \"match\":\"regex\", \"value\":\".*\"}," \
                        "{\"type\":\"fqdn\", \"match\":\"inzone\"}," \
                        "{\"type\":\"fqdn\", \"match\":\"req.fqdn\"}," \
                        "{\"type\":\"fqdn\", \"match\":\"req.fqdn\", \"prefix\":\"test.\"}," \
                        "{\"type\":\"fqdn\", \"match\":\"req.fqdn\", \"suffix\":\".test\"}" \
                    "]" \
                "}," \
                "\"SANPolicy\":{" \
                    "\"multiSAN\":true," \
                    "\"AllowedSANs\":[]" \
                "}," \
                "\"KeyUsagePolicy\":{" \
                    "\"AllowedKeyUsages\":255" \
                "}," \
                "\"CertDurationPolicy\":{" \
                    "\"MaxAllowedDays\":365" \
                "}" \
            "}" \
        "}"

#define TEST_POLICY_JSON_VALID_ONLY_CERTPOLICY \
            "{\"CertificatePolicy\":{}}"

#define TEST_POLICY_JSON_VALID_ONLY_CAPOLICY \
            "{\"CAPolicy\":{}}"

// Invalid values for type or match
#define TEST_POLICY_JSON_INVALID_TYPE \
            "{\"CertificatePolicy\":{\"SNPolicy\":{\"AllowedCNs\":[{\"type\":\"random\", \"match\":\"constant\", \"value\":\"test\"}]}}}"

#define TEST_POLICY_JSON_INVALID_MATCH \
            "{\"CAPolicy\":{\"SANPolicy\":{\"AllowedSANs\":[{\"type\":\"name\", \"match\":\"random\"}]}}}"


// Invalid type-match combinations
#define TEST_POLICY_JSON_INVALID_TYPE_MATCH_COMBO1 \
            "{\"CertificatePolicy\":{\"SNPolicy\":{\"AllowedCNs\":[{\"type\":\"ip\", \"match\":\"req.hostanme\"}]}}}"

#define TEST_POLICY_JSON_INVALID_TYPE_MATCH_COMBO2 \
            "{\"CertificatePolicy\":{\"SANPolicy\":{\"AllowedSANs\":[{\"type\":\"ip\", \"match\":\"req.fqdn\"}]}}}"

#define TEST_POLICY_JSON_INVALID_TYPE_MATCH_COMBO3 \
            "{\"CAPolicy\":{\"SNPolicy\":{\"AllowedCNs\":[{\"type\":\"name\", \"match\":\"private\"}]}}}"

#define TEST_POLICY_JSON_INVALID_TYPE_MATCH_COMBO4 \
            "{\"CAPolicy\":{\"SANPolicy\":{\"AllowedSANs\":[{\"type\":\"name\", \"match\":\"public\"}]}}}"

#define TEST_POLICY_JSON_INVALID_TYPE_MATCH_COMBO5 \
            "{\"CertificatePolicy\":{\"SNPolicy\":{\"AllowedCNs\":[{\"type\":\"name\", \"match\":\"inzone\"}]}}}"

#define TEST_POLICY_JSON_INVALID_TYPE_MATCH_COMBO6 \
            "{\"CertificatePolicy\":{\"SNPolicy\":{\"AllowedCNs\":[{\"type\":\"name\", \"match\":\"req.fqdn\"}]}}}"

#define TEST_POLICY_JSON_INVALID_TYPE_MATCH_COMBO7 \
            "{\"CAPolicy\":{\"SNPolicy\":{\"AllowedCNs\":[{\"type\":\"fqdn\", \"match\":\"private\"}]}}}"

#define TEST_POLICY_JSON_INVALID_TYPE_MATCH_COMBO8 \
            "{\"CertificatePolicy\":{\"SANPolicy\":{\"AllowedSANs\":[{\"type\":\"fqdn\", \"match\":\"public\"}]}}}"

#define TEST_POLICY_JSON_INVALID_TYPE_MATCH_COMBO9 \
            "{\"CAPolicy\":{\"SANPolicy\":{\"AllowedSANs\":[{\"type\":\"fqdn\", \"match\":\"req.hostname\"}]}}}"

// Missing type or match keys
#define TEST_POLICY_JSON_INVALID_MISSING_TYPE \
            "{\"CertificatePolicy\":{\"SNPolicy\":{\"AllowedCNs\":[{\"match\":\"any\"}]}}}"

#define TEST_POLICY_JSON_INVALID_MISSING_MATCH \
            "{\"CAPolicy\":{\"SANPolicy\":{\"AllowedSANs\":[{\"type\":\"ip\"}]}}}"

// Missing value key for type-match combinations that require a value
#define TEST_POLICY_JSON_INVALID_MISSING_VALUE1 \
            "{\"CertificatePolicy\":{\"SNPolicy\":{\"AllowedCNs\":[{\"type\":\"ip\", \"match\":\"constant\"}]}}}"

#define TEST_POLICY_JSON_INVALID_MISSING_VALUE2 \
            "{\"CertificatePolicy\":{\"SANPolicy\":{\"AllowedSANs\":[{\"type\":\"ip\", \"match\":\"regex\"}]}}}"

#define TEST_POLICY_JSON_INVALID_MISSING_VALUE3 \
            "{\"CAPolicy\":{\"SNPolicy\":{\"AllowedCNs\":[{\"type\":\"name\", \"match\":\"constant\"}]}}}"

#define TEST_POLICY_JSON_INVALID_MISSING_VALUE4 \
            "{\"CAPolicy\":{\"SANPolicy\":{\"AllowedSANs\":[{\"type\":\"name\", \"match\":\"regex\"}]}}}"

#define TEST_POLICY_JSON_INVALID_MISSING_VALUE5 \
            "{\"CertificatePolicy\":{\"SANPolicy\":{\"AllowedSANs\":[{\"type\":\"fqdn\", \"match\":\"constant\"}]}}}"

#define TEST_POLICY_JSON_INVALID_MISSING_VALUE6 \
            "{\"CAPolicy\":{\"SANPolicy\":{\"AllowedSANs\":[{\"type\":\"fqdn\", \"match\":\"regex\"}]}}}"

// Invalid key usage values (max 8 bit allowed)
#define TEST_POLICY_JSON_INVALID_KEYUSAGE1 \
            "{\"CertificatePolicy\":{\"KeyUsagePolicy\":{\"AllowedKeyUsages\":256}}}"

#define TEST_POLICY_JSON_INVALID_KEYUSAGE2 \
            "{\"CAPolicy\":{\"KeyUsagePolicy\":{\"AllowedKeyUsages\":-1}}}"
