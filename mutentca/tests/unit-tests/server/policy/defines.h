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

// policy.c 
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
            "{\"CertificatePolicy\":{\"KeyUsagePolicy\":{\"AllowedKeyUsages\":512}}}"

#define TEST_POLICY_JSON_INVALID_KEYUSAGE2 \
            "{\"CAPolicy\":{\"KeyUsagePolicy\":{\"AllowedKeyUsages\":-1}}}"


// validate.c
// Sample policy config jsons and CSRs used for validation testing
#define TEST_SN_JSON_VALID1 \
            "{\"CertificatePolicy\":{\"SNPolicy\":{\"AllowedCNs\":[{\"type\":\"name\", \"match\":\"constant\", \"value\":\"test\"}]}}}"

#define TEST_SN_JSON_VALID2 \
            "{\"CertificatePolicy\":{\"SNPolicy\":{\"AllowedCNs\":[{\"type\":\"name\", \"match\":\"req.hostname\", \"prefix\":\"t\"}]}}}"

#define TEST_SN_JSON_VALID3 \
            "{\"CertificatePolicy\":{\"SNPolicy\":{\"AllowedCNs\":[{\"type\":\"ip\", \"match\":\"private\"}]}}}"

#define TEST_SN_JSON_INVALID1 \
            "{\"CertificatePolicy\":{\"SNPolicy\":{\"AllowedCNs\":[{\"type\":\"name\", \"match\":\"constant\", \"value\":\"dummy\"}]}}}"

#define TEST_SN_JSON_INVALID2 \
            "{\"CertificatePolicy\":{\"SNPolicy\":{\"AllowedCNs\":[{\"type\":\"ip\", \"match\":\"public\"}]}}}"

#define TEST_SAN_JSON_VALID1 \
            "{\"CertificatePolicy\":{\"SANPolicy\":{" \
                "\"multiSAN\": true," \
                "\"AllowedSANs\":[{\"type\":\"fqdn\", \"match\":\"constant\", \"value\":\"san.domain1.com\"}," \
                                 "{\"type\":\"fqdn\", \"match\":\"constant\", \"value\":\"san.domain2.com\"}]}}}"

#define TEST_SAN_JSON_VALID2 \
            "{\"CertificatePolicy\":{\"SANPolicy\":{" \
                "\"multiSAN\": true," \
                "\"AllowedSANs\":[{\"type\":\"fqdn\", \"match\":\"regex\", \"value\":\".*.com$\"}]}}}"

#define TEST_SAN_JSON_VALID3 \
            "{\"CertificatePolicy\":{\"SANPolicy\":{" \
                "\"multiSAN\": true," \
                "\"AllowedSANs\":[{\"type\":\"name\", \"match\":\"req.hostname\", \"prefix\":\"t\"}," \
                                 "{\"type\":\"fqdn\", \"match\":\"req.fqdn\", \"prefix\":\"t\"}]}}}"

#define TEST_SAN_JSON_INVALID1 \
            "{\"CertificatePolicy\":{\"SANPolicy\":{" \
                "\"multiSAN\": false," \
                "\"AllowedSANs\":[{\"type\":\"fqdn\", \"match\":\"constant\", \"value\":\"san.domain1.com\"}," \
                                 "{\"type\":\"fqdn\", \"match\":\"constant\", \"value\":\"san.domain2.com\"}]}}}"

#define TEST_SAN_JSON_INVALID2 \
            "{\"CertificatePolicy\":{\"SANPolicy\":{" \
                "\"multiSAN\": true," \
                "\"AllowedSANs\":[{\"type\":\"fqdn\", \"match\":\"constant\", \"value\":\"san.domain1.com\"}," \
                                 "{\"type\":\"fqdn\", \"match\":\"constant\", \"value\":\"dummy.domain2.com\"}]}}}"

#define TEST_SAN_JSON_INVALID3 \
            "{\"CertificatePolicy\":{\"SANPolicy\":{" \
                "\"multiSAN\": true," \
                "\"AllowedSANs\":[{\"type\":\"fqdn\", \"match\":\"regex\", \"value\":\".*.dummy.com$\"}]}}}"

#define TEST_KEY_USAGE_JSON_VALID1 \
            "{\"CertificatePolicy\":{\"KeyUsagePolicy\":{\"AllowedKeyUsages\": 385}}}"

#define TEST_KEY_USAGE_JSON_INVALID1 \
            "{\"CertificatePolicy\":{\"KeyUsagePolicy\":{\"AllowedKeyUsages\": 259}}}"

#define TEST_DURATION_JSON_VALID1 \
            "{\"CertificatePolicy\":{\"CertDurationPolicy\":{\"MaxAllowedDays\": 10}}}"

#define TEST_DURATION_JSON_INVALID1 \
            "{\"CertificatePolicy\":{\"CertDurationPolicy\":{\"MaxAllowedDays\": 5}}}"

#define TEST_CSR1 \
    "-----BEGIN CERTIFICATE REQUEST-----\n" \
    "MIICzTCCAbUCAQAwPDELMAkGA1UEBhMCVVMxCzAJBgNVBAgMAkNBMREwDwYDVQQH\n" \
    "DAhCZWxsZXZ1ZTENMAsGA1UEAwwEdGVzdDCCASIwDQYJKoZIhvcNAQEBBQADggEP\n" \
    "ADCCAQoCggEBAKG5ZN5dUbz+RvmmyHzOZMP9kNMGSOKOVzbo4W1dfiCIDP0rXzbo\n" \
    "nfM/OzzOsO5RpP/EusVNxL63PbbFRsd2U0l7m5bMhB8dhDmZVAVQ7lqzF4YA79tq\n" \
    "vL/IvBBq7EXXiX9sFbIFuHndvtYCjfy6G30NFS57Uff4i8MsIZNXtcWxBgPLKUU+\n" \
    "ByQ3Wnp6UA/KSJrishH7KpEYcV8mytnXJnGhhQtCjgaW78pL/RBPo4xWIlwJN8jT\n" \
    "MkNioatfpIJ5CAmZvalU9u94dxbhOoe7CACL2up4help+KMf7U+kK/bEbvYIEB9D\n" \
    "JmK1LyS+WqzJzfROgNf3NpmQTPXwJMR02kcCAwEAAaBMMEoGCSqGSIb3DQEJDjE9\n" \
    "MDswDAYDVR0PBAUDAweBgDArBgNVHREEJDAigg9zYW4uZG9tYWluMS5jb22CD3Nh\n" \
    "bi5kb21haW4yLmNvbTANBgkqhkiG9w0BAQsFAAOCAQEAcz5MHOIdW2at+H3a8iUA\n" \
    "4jEgMWGx6C09KWekgIBNiUn9HPM8Qv9MMXH7p9sZKcthlczYadSpVMZ9UlzXl9uO\n" \
    "9BP/fOKncVdafyHRZE9zMnNPfTgmFa4bILj8vSKHcaeqwQAagVGwr3bT0waRR4wp\n" \
    "yh5UIZyRJXkR1cni18wmMG7WZGA9jKBZtydSstrBcnkKzCFl28aBmW1/Xd5CsKQR\n" \
    "ghCUdSZyrhPwgVQ8KHDJnIcvZVueOL+8x8htXNfLhSyyHQBUIXe1CMFFLqTY6eV1\n" \
    "XYEKpXwrVKBskAQV98fdEjeLwdddLhCHKoLSTLubxobwsFrQhQo0TU4bVeF+6Vkk\n" \
    "bA==\n" \
    "-----END CERTIFICATE REQUEST-----\n"

#define TEST_CSR2 \
    "-----BEGIN CERTIFICATE REQUEST-----\n" \
    "MIIC2zCCAcMCAQAwQzELMAkGA1UEBhMCVVMxCzAJBgNVBAgMAkNBMREwDwYDVQQH\n" \
    "DAhCZWxsZXZ1ZTEUMBIGA1UEAwwLMTkyLjE2OC4wLjEwggEiMA0GCSqGSIb3DQEB\n" \
    "AQUAA4IBDwAwggEKAoIBAQChuWTeXVG8/kb5psh8zmTD/ZDTBkjijlc26OFtXX4g\n" \
    "iAz9K1826J3zPzs8zrDuUaT/xLrFTcS+tz22xUbHdlNJe5uWzIQfHYQ5mVQFUO5a\n" \
    "sxeGAO/bary/yLwQauxF14l/bBWyBbh53b7WAo38uht9DRUue1H3+IvDLCGTV7XF\n" \
    "sQYDyylFPgckN1p6elAPykia4rIR+yqRGHFfJsrZ1yZxoYULQo4Glu/KS/0QT6OM\n" \
    "ViJcCTfI0zJDYqGrX6SCeQgJmb2pVPbveHcW4TqHuwgAi9rqeIXpafijH+1PpCv2\n" \
    "xG72CBAfQyZitS8kvlqsyc30ToDX9zaZkEz18CTEdNpHAgMBAAGgUzBRBgkqhkiG\n" \
    "9w0BCQ4xRDBCMAsGA1UdDwQEAwIHgDATBgNVHSUEDDAKBggrBgEFBQcDATAeBgNV\n" \
    "HREEFzAVgg10ZXN0Lmx3LmxvY2FsggR0ZXN0MA0GCSqGSIb3DQEBCwUAA4IBAQB9\n" \
    "WjqxukPbEO7sFRVAJo9WU/iKIcvJoZbLWJanICqmu7OhtLNc3O2SFQti4j4c6FBD\n" \
    "CxH6KPuq7zCLlWz/8Rc6g2ap3QelLMNOGZqeWyiRNf6dWjFCXKtZrqVwXPDaoksd\n" \
    "wL12BnLHPrWa+PXBJK6B8flPnxCNXnj+Q63XVeJUbRmH2Bl0+ARXST0RLuDVnO/A\n" \
    "F/F0y4oHcHuWJ3ccfpq1Q0cbS4Ty7aGMAPBpEhQG6+a5HtEEsmK2LFIXqOTX2JUF\n" \
    "JCzzVjzw4P/oBoF+RIJObVTKICrJfBwKaWeRtTjO7oiNAOE43p9s44OMTo0QV50u\n" \
    "xaZQOlczEro/hNl5Nmzh\n" \
    "-----END CERTIFICATE REQUEST-----"
