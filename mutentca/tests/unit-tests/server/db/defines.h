/*
 * Copyright © 2018 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the “License”) you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an “AS IS” BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */


#define LWCA_POST_DB_CONFIG "./test-mutentcadb-config/test-postdb-config.json"

#define LWCA_POST_PLUGIN_ADD_CA             "LwCADbPostPluginAddCA"
#define LWCA_POST_PLUGIN_ADD_CERT           "LwCADbPostPluginAddCertData"
#define LWCA_POST_SERIALIZE_CA_TO_JSON      "LwCASerializeCAToJSON"
#define LWCA_POST_SERIALIZE_CERT_DATA_TO_JSON   "LwCASerializeCertDataToJSON"
#define LWCA_POST_SERIALIZE_CONTAINER_TO_JSON   "LwCASerializeContainerToJSON"
#define LWCA_POST_DESERIALIZE_JSON_TO_CA    "LwCADeserializeJSONToCA"
#define LWCA_POST_DESERIALIZE_JSON_TO_CERT_DATA "LwCADeserializeJSONToCertData"
#define LWCA_POST_CHECK_CA                  "LwCADbPostPluginCheckCA"
#define LWCA_POST_GET_CA                    "LwCADbPostPluginGetCA"
#define LWCA_POST_GET_CERT                  "LwCADbPostPluginGetCertData"
#define LWCA_POST_UPDATE_CA                 "LwCADbPostPluginUpdateCA"
#define LWCA_UPDATE_CA_REQUEST_BODY         "LwCAGenerateCAPatchRequestBody"
#define LWCA_POST_GET_PARENT_CA_ID          "LwCADbPostPluginGetParentCAId"
#define LWCA_GET_STRING_FROM_RESPONSE       "LwCAGetStringAttrFromResponse"
#define LWCA_POST_UPDATE_CERT               "LwCAGenerateCertPatchRequestBody"
#define LWCA_CONFIG_DB_PLUGIN_KEY_NAME   "dbPlugin"
#define LWCA_CONFIG_DB_PLUGIN_PATH       "dbPluginConfigPath"
#define TEST_SUBJECT                "TEST_SUBJECT"
#define TEST_PRIV_KEY               "01000100"
#define TEST_PRIV_KEY_ENCODED       "MDEwMDAxMDA="
#define TEST_CERT_1                 "10101010"
#define TEST_CERT_1_ENCODED         "MTAxMDEwMTA="
#define TEST_CERT_2                 "11110000"
#define TEST_CERT_2_ENCODED         "MTExMTAwMDA="
#define TEST_CERT_3                 "01010101"
#define TEST_CERT_3_ENCODED         "MDEwMTAxMDE="
#define TEST_CRL_NUM                "1500"
#define TEST_LAST_CRL_UPDATE        "20181031223344.0Z"
#define TEST_NEXT_CRL_UPDATE        "20181101223344.0Z"
#define TEST_CA_STATUS              1
#define TEST_CA_ID                  "testId"
#define TEST_PARENT_CA_ID           "testParentId"
#define DUMMY_DOMAIN                "dc=lw-testdom,dc=com"

#define TEST_SERIAL_NUMBER          "1500"
#define TEST_REVOKED_REASON         1
#define TEST_REVOKED_DATE           "20181102223344.0Z"
#define TEST_TIME_VALID_FROM        "20181103223344.0Z"
#define TEST_TIME_VALID_TO          "20181104223344.0Z"
#define TEST_LWCA_CERT_STATUS       1

#define TEST_PARENT_CA_DN           "cn=" TEST_PARENT_CA_ID ",cn=Certificate-Authority," DUMMY_DOMAIN
#define TEST_INTR_CA_DN             "cn=" TEST_CA_ID "," TEST_PARENT_CA_DN

#define SERIALIZED_ROOT_CA_JSON ("{\n" \
    "    \"dn\": \"cn="TEST_PARENT_CA_ID",cn=Certificate-Authority,dc=lw-testdom,dc=com\",\n" \
    "    \"attributes\": [\n" \
    "        {\n" \
    "            \"type\": \"objectClass\",\n" \
    "            \"value\": [\n" \
    "                \"vmwCertificationAuthority\",\n" \
    "                \"pkiCA\"\n" \
    "            ]\n" \
    "        },\n" \
    "        {\n" \
    "            \"type\": \"cn\",\n" \
    "            \"value\": [\n" \
    "                \""TEST_PARENT_CA_ID"\"\n" \
    "            ]\n" \
    "        },\n" \
    "        {\n" \
    "            \"type\": \"cACertificateDN\",\n" \
    "            \"value\": [\n" \
    "                \""TEST_SUBJECT"\"\n" \
    "            ]\n" \
    "        },\n" \
    "        {\n" \
    "            \"type\": \"cAEncryptedPrivateKey\",\n" \
    "            \"value\": [\n" \
    "                \""TEST_PRIV_KEY_ENCODED"\"\n" \
    "            ]\n" \
    "        },\n" \
    "        {\n" \
    "            \"type\": \"cACRLNumber\",\n" \
    "            \"value\": [\n" \
    "                \""TEST_CRL_NUM"\"\n" \
    "            ]\n" \
    "        },\n" \
    "        {\n" \
    "            \"type\": \"cACertificate\",\n" \
    "            \"value\": [\n" \
    "                \""TEST_CERT_1_ENCODED"\",\n" \
    "                \""TEST_CERT_2_ENCODED"\",\n" \
    "                \""TEST_CERT_3_ENCODED"\"\n" \
    "            ]\n" \
    "        },\n" \
    "        {\n" \
    "            \"type\": \"cALastCRLUpdate\",\n" \
    "            \"value\": [\n" \
    "                \""TEST_LAST_CRL_UPDATE"\"\n" \
    "            ]\n" \
    "        },\n" \
    "        {\n" \
    "            \"type\": \"cANextCRLUpdate\",\n" \
    "            \"value\": [\n" \
    "                \""TEST_NEXT_CRL_UPDATE"\"\n" \
    "            ]\n" \
    "        },\n" \
    "        {\n" \
    "            \"type\": \"cAStatus\",\n" \
    "            \"value\": [\n" \
    "                \"1\"\n" \
    "            ]\n" \
    "        }\n" \
    "    ]\n" \
    "}")

#define SERIALIZED_INTERMEDIATE_CA_JSON ("{\n" \
    "    \"dn\": \"cn="TEST_CA_ID",cn="TEST_PARENT_CA_ID",cn=Certificate-Authority,dc=lw-testdom,dc=com\",\n" \
    "    \"attributes\": [\n" \
    "        {\n" \
    "            \"type\": \"objectClass\",\n" \
    "            \"value\": [\n" \
    "                \"vmwCertificationAuthority\",\n" \
    "                \"pkiCA\"\n" \
    "            ]\n" \
    "        },\n" \
    "        {\n" \
    "            \"type\": \"cn\",\n" \
    "            \"value\": [\n" \
    "                \""TEST_CA_ID"\"\n" \
    "            ]\n" \
    "        },\n" \
    "        {\n" \
    "            \"type\": \"cACertificateDN\",\n" \
    "            \"value\": [\n" \
    "                \""TEST_SUBJECT"\"\n" \
    "            ]\n" \
    "        },\n" \
    "        {\n" \
    "            \"type\": \"cAEncryptedPrivateKey\",\n" \
    "            \"value\": [\n" \
    "                \""TEST_PRIV_KEY_ENCODED"\"\n" \
    "            ]\n" \
    "        },\n" \
    "        {\n" \
    "            \"type\": \"cACRLNumber\",\n" \
    "            \"value\": [\n" \
    "                \""TEST_CRL_NUM"\"\n" \
    "            ]\n" \
    "        },\n" \
    "        {\n" \
    "            \"type\": \"cAParentCAId\",\n" \
    "            \"value\": [\n" \
    "                \""TEST_PARENT_CA_ID"\"\n" \
    "            ]\n" \
    "        },\n" \
    "        {\n" \
    "            \"type\": \"cACertificate\",\n" \
    "            \"value\": [\n" \
    "                \""TEST_CERT_1_ENCODED"\",\n" \
    "                \""TEST_CERT_2_ENCODED"\",\n" \
    "                \""TEST_CERT_3_ENCODED"\"\n" \
    "            ]\n" \
    "        },\n" \
    "        {\n" \
    "            \"type\": \"cALastCRLUpdate\",\n" \
    "            \"value\": [\n" \
    "                \""TEST_LAST_CRL_UPDATE"\"\n" \
    "            ]\n" \
    "        },\n" \
    "        {\n" \
    "            \"type\": \"cANextCRLUpdate\",\n" \
    "            \"value\": [\n" \
    "                \""TEST_NEXT_CRL_UPDATE"\"\n" \
    "            ]\n" \
    "        },\n" \
    "        {\n" \
    "            \"type\": \"cAStatus\",\n" \
    "            \"value\": [\n" \
    "                \"1\"\n" \
    "            ]\n" \
    "        }\n" \
    "    ]\n" \
    "}")

#define SERIALIZED_CONTAINER_JSON ("{\n" \
    "    \"dn\": \""TEST_PARENT_CA_DN"\",\n"  \
    "    \"attributes\": [\n"   \
    "        {\n" \
    "            \"type\": \"objectClass\",\n" \
    "            \"value\": [\n" \
    "                \"container\",\n" \
    "                \"top\"\n" \
    "            ]\n" \
    "        },\n" \
    "        {\n"   \
    "            \"type\": \"cn\",\n"   \
    "            \"value\": [\n"    \
    "                \""TEST_PARENT_CA_ID"\"\n"    \
    "            ]\n"   \
    "        }\n"   \
    "    ]\n"   \
    "}")

#define ROOT_CA_JSON_RESPONSE (\
    "{\n" \
    "    \"result\": [{\n" \
    "        \"dn\": \"cn="TEST_PARENT_CA_ID",cn=Certificate-Authority,dc=lw-testdom,dc=com\",\n" \
    "        \"attributes\": [{\n" \
    "            \"type\": \"nTSecurityDescriptor\",\n" \
    "            \"value\": [\"\\u0001\"]\n" \
    "        }, {\n" \
    "            \"type\": \"cAStatus\",\n" \
    "            \"value\": [\"1\"]\n" \
    "        }, {\n" \
    "            \"type\": \"cACertificate\",\n" \
    "            \"value\": [\""TEST_CERT_1_ENCODED"\", \""TEST_CERT_2_ENCODED"\", \""TEST_CERT_3_ENCODED"\"]\n" \
    "        }, {\n" \
    "            \"type\": \"cACRLNumber\",\n" \
    "            \"value\": [\""TEST_CRL_NUM"\"]\n" \
    "        }, {\n" \
    "            \"type\": \"cAEncryptedPrivateKey\",\n" \
    "            \"value\": [\""TEST_PRIV_KEY_ENCODED"\"]\n" \
    "        }, {\n" \
    "            \"type\": \"cACertificateDN\",\n" \
    "            \"value\": [\""TEST_SUBJECT"\"]\n" \
    "        }, {\n" \
    "            \"type\": \"cn\",\n" \
    "            \"value\": [\""TEST_PARENT_CA_ID"\"]\n" \
    "        }, {\n" \
    "            \"type\": \"cALastCRLUpdate\",\n" \
    "            \"value\": [\""TEST_LAST_CRL_UPDATE"\"]\n" \
    "        }, {\n" \
    "            \"type\": \"cANextCRLUpdate\",\n" \
    "            \"value\": [\""TEST_NEXT_CRL_UPDATE"\"]\n" \
    "        }, {\n" \
    "            \"type\": \"objectClass\",\n" \
    "            \"value\": [\"vmwCertificationAuthority\", \"pkiCA\"]\n" \
    "        }]\n" \
    "    }],\n" \
    "    \"result_count\": 1\n" \
    "}")

#define INTR_CA_JSON_RESPONSE (\
    "{\n" \
    "    \"result\": [{\n" \
    "        \"dn\": \"cn="TEST_CA_ID",cn="TEST_PARENT_CA_ID",cn=Certificate-Authority,dc=lw-testdom,dc=com\",\n" \
    "        \"attributes\": [{\n" \
    "            \"type\": \"nTSecurityDescriptor\",\n" \
    "            \"value\": [\"\\u0001\"]\n" \
    "        }, {\n" \
    "            \"type\": \"cAStatus\",\n" \
    "            \"value\": [\"1\"]\n" \
    "        }, {\n" \
    "            \"type\": \"cACertificate\",\n" \
    "            \"value\": [\""TEST_CERT_1_ENCODED"\", \""TEST_CERT_2_ENCODED"\", \""TEST_CERT_3_ENCODED"\"]\n" \
    "        }, {\n" \
    "            \"type\": \"cAParentCAId\",\n" \
    "            \"value\": [\""TEST_PARENT_CA_ID"\"]\n" \
    "        }, {\n" \
    "            \"type\": \"cACRLNumber\",\n" \
    "            \"value\": [\""TEST_CRL_NUM"\"]\n" \
    "        }, {\n" \
    "            \"type\": \"cAEncryptedPrivateKey\",\n" \
    "            \"value\": [\""TEST_PRIV_KEY_ENCODED"\"]\n" \
    "        }, {\n" \
    "            \"type\": \"cACertificateDN\",\n" \
    "            \"value\": [\""TEST_SUBJECT"\"]\n" \
    "        }, {\n" \
    "            \"type\": \"cn\",\n" \
    "            \"value\": [\""TEST_CA_ID"\"]\n" \
    "        }, {\n" \
    "            \"type\": \"cALastCRLUpdate\",\n" \
    "            \"value\": [\""TEST_LAST_CRL_UPDATE"\"]\n" \
    "        }, {\n" \
    "            \"type\": \"cANextCRLUpdate\",\n" \
    "            \"value\": [\""TEST_NEXT_CRL_UPDATE"\"]\n" \
    "        }, {\n" \
    "            \"type\": \"objectClass\",\n" \
    "            \"value\": [\"vmwCertificationAuthority\", \"pkiCA\"]\n" \
    "        }]\n" \
    "    }],\n" \
    "    \"result_count\": 1\n" \
    "}")

#define CERT_DATA_JSON_RESPONSE ("{\n" \
    "    \"result\": [{\n" \
    "        \"dn\": \"cn=2000,cn=Certs,cn=testId,cn=testParentId,cn=Certificate-Authority,dc=lw-testdom,dc=com\",\n" \
    "        \"attributes\": [{\n" \
    "            \"type\": \"nTSecurityDescriptor\",\n" \
    "            \"value\": [\"\\u0001\"]\n" \
    "        }, {\n" \
    "            \"type\": \"certStatus\",\n" \
    "            \"value\": [\"1\"]\n" \
    "        }, {\n" \
    "            \"type\": \"certTimeValidTo\",\n" \
    "            \"value\": [\"20181104223344.0Z\"]\n" \
    "        }, {\n" \
    "            \"type\": \"certTimeValidFrom\",\n" \
    "            \"value\": [\"20181103223344.0Z\"]\n" \
    "        }, {\n" \
    "            \"type\": \"certRevokedDate\",\n" \
    "            \"value\": [\"20181102223344.0Z\"]\n" \
    "        }, {\n" \
    "            \"type\": \"certRevokedReason\",\n" \
    "            \"value\": [\"1\"]\n" \
    "        }, {\n" \
    "            \"type\": \"certIssuer\",\n" \
    "            \"value\": [\"testId\"]\n" \
    "        }, {\n" \
    "            \"type\": \"certSerialNumber\",\n" \
    "            \"value\": [\"2000\"]\n" \
    "        }, {\n" \
    "            \"type\": \"cn\",\n" \
    "            \"value\": [\"2000\"]\n" \
    "        }, {\n" \
    "            \"type\": \"objectClass\",\n" \
    "            \"value\": [\"vmwCerts\"]\n" \
    "        }]\n" \
    "    }, {\n" \
    "        \"dn\": \"cn=1500,cn=Certs,cn=testId,cn=testParentId,cn=Certificate-Authority,dc=lw-testdom,dc=com\",\n" \
    "        \"attributes\": [{\n" \
    "            \"type\": \"nTSecurityDescriptor\",\n" \
    "            \"value\": [\"\\u0001\"]\n" \
    "        }, {\n" \
    "            \"type\": \"certStatus\",\n" \
    "            \"value\": [\"1\"]\n" \
    "        }, {\n" \
    "            \"type\": \"certTimeValidTo\",\n" \
    "            \"value\": [\"20181104223344.0Z\"]\n" \
    "        }, {\n" \
    "            \"type\": \"certTimeValidFrom\",\n" \
    "            \"value\": [\"20181103223344.0Z\"]\n" \
    "        }, {\n" \
    "            \"type\": \"certRevokedDate\",\n" \
    "            \"value\": [\"20181102223344.0Z\"]\n" \
    "        }, {\n" \
    "            \"type\": \"certRevokedReason\",\n" \
    "            \"value\": [\"1\"]\n" \
    "        }, {\n" \
    "            \"type\": \"certIssuer\",\n" \
    "            \"value\": [\"testId\"]\n" \
    "        }, {\n" \
    "            \"type\": \"certSerialNumber\",\n" \
    "            \"value\": [\"1500\"]\n" \
    "        }, {\n" \
    "            \"type\": \"cn\",\n" \
    "            \"value\": [\"1500\"]\n" \
    "        }, {\n" \
    "            \"type\": \"objectClass\",\n" \
    "            \"value\": [\"vmwCerts\"]\n" \
    "        }]\n" \
    "    }],\n" \
    "    \"result_count\": 2\n" \
    "}")

#define CA_GENERATED_PATCH (\
    "[\n" \
    "    {\n" \
    "        \"operation\": \"replace\",\n" \
    "        \"attribute\": {\n" \
    "            \"type\": \"cACertificateDN\",\n" \
    "            \"value\": [\n" \
    "                \""TEST_SUBJECT"\"\n" \
    "            ]\n" \
    "        }\n" \
    "    },\n" \
    "    {\n" \
    "        \"operation\": \"replace\",\n" \
    "        \"attribute\": {\n" \
    "            \"type\": \"cACertificate\",\n" \
    "            \"value\": [\n" \
    "                \""TEST_CERT_1_ENCODED"\",\n" \
    "                \""TEST_CERT_2_ENCODED"\",\n" \
    "                \""TEST_CERT_3_ENCODED"\"\n" \
    "            ]\n" \
    "        }\n" \
    "    },\n" \
    "    {\n" \
    "        \"operation\": \"replace\",\n" \
    "        \"attribute\": {\n" \
    "            \"type\": \"cAEncryptedPrivateKey\",\n" \
    "            \"value\": [\n" \
    "                \""TEST_PRIV_KEY_ENCODED"\"\n" \
    "            ]\n" \
    "        }\n" \
    "    },\n" \
    "    {\n" \
    "        \"operation\": \"replace\",\n" \
    "        \"attribute\": {\n" \
    "            \"type\": \"cACRLNumber\",\n" \
    "            \"value\": [\n" \
    "                \""TEST_CRL_NUM"\"\n" \
    "            ]\n" \
    "        }\n" \
    "    },\n" \
    "    {\n" \
    "        \"operation\": \"replace\",\n" \
    "        \"attribute\": {\n" \
    "            \"type\": \"cALastCRLUpdate\",\n" \
    "            \"value\": [\n" \
    "                \""TEST_LAST_CRL_UPDATE"\"\n" \
    "            ]\n" \
    "        }\n" \
    "    },\n" \
    "    {\n" \
    "        \"operation\": \"replace\",\n" \
    "        \"attribute\": {\n" \
    "            \"type\": \"cANextCRLUpdate\",\n" \
    "            \"value\": [\n" \
    "                \""TEST_NEXT_CRL_UPDATE"\"\n" \
    "            ]\n" \
    "        }\n" \
    "    },\n" \
    "    {\n" \
    "        \"operation\": \"replace\",\n" \
    "        \"attribute\": {\n" \
    "            \"type\": \"cAStatus\",\n" \
    "            \"value\": [\n" \
    "                \"1\"\n" \
    "            ]\n" \
    "        }\n" \
    "    }\n" \
    "]")

#define SERIALIZED_CERT_DATA_JSON ("{\n" \
    "    \"dn\": \"cn="TEST_SERIAL_NUMBER",cn=Certs,cn="TEST_CA_ID",cn="TEST_PARENT_CA_ID",cn=Certificate-Authority,dc=lw-testdom,dc=com\",\n" \
    "    \"attributes\": [\n" \
    "        {\n" \
    "            \"type\": \"objectClass\",\n" \
    "            \"value\": [\n" \
    "                \"vmwCerts\"\n" \
    "            ]\n" \
    "        },\n" \
    "        {\n" \
    "            \"type\": \"cn\",\n" \
    "            \"value\": [\n" \
    "                \""TEST_SERIAL_NUMBER"\"\n" \
    "            ]\n" \
    "        },\n" \
    "        {\n" \
    "            \"type\": \"certSerialNumber\",\n" \
    "            \"value\": [\n" \
    "                \""TEST_SERIAL_NUMBER"\"\n" \
    "            ]\n" \
    "        },\n" \
    "        {\n" \
    "            \"type\": \"certIssuer\",\n" \
    "            \"value\": [\n" \
    "                \""TEST_CA_ID"\"\n" \
    "            ]\n" \
    "        },\n" \
    "        {\n" \
    "            \"type\": \"certRevokedReason\",\n" \
    "            \"value\": [\n" \
    "                \"1\"\n" \
    "            ]\n" \
    "        },\n" \
    "        {\n" \
    "            \"type\": \"certRevokedDate\",\n" \
    "            \"value\": [\n" \
    "                \""TEST_REVOKED_DATE"\"\n" \
    "            ]\n" \
    "        },\n" \
    "        {\n" \
    "            \"type\": \"certTimeValidFrom\",\n" \
    "            \"value\": [\n" \
    "                \""TEST_TIME_VALID_FROM"\"\n" \
    "            ]\n" \
    "        },\n" \
    "        {\n" \
    "            \"type\": \"certTimeValidTo\",\n" \
    "            \"value\": [\n" \
    "                \""TEST_TIME_VALID_TO"\"\n" \
    "            ]\n" \
    "        },\n" \
    "        {\n" \
    "            \"type\": \"certStatus\",\n" \
    "            \"value\": [\n" \
    "                \"1\"\n" \
    "            ]\n" \
    "        }\n" \
    "    ]\n" \
    "}")

#define CERT_GENERATED_PATCH ("[\n" \
    "    {\n" \
    "        \"operation\": \"replace\",\n" \
    "        \"attribute\": {\n" \
    "            \"type\": \"certRevokedReason\",\n" \
    "            \"value\": [\n" \
    "                \"1\"\n" \
    "            ]\n" \
    "        }\n" \
    "    },\n" \
    "    {\n" \
    "        \"operation\": \"replace\",\n" \
    "        \"attribute\": {\n" \
    "            \"type\": \"certRevokedDate\",\n" \
    "            \"value\": [\n" \
    "                \"20181102223344.0Z\"\n" \
    "            ]\n" \
    "        }\n" \
    "    },\n" \
    "    {\n" \
    "        \"operation\": \"replace\",\n" \
    "        \"attribute\": {\n" \
    "            \"type\": \"certTimeValidFrom\",\n" \
    "            \"value\": [\n" \
    "                \"20181103223344.0Z\"\n" \
    "            ]\n" \
    "        }\n" \
    "    },\n" \
    "    {\n" \
    "        \"operation\": \"replace\",\n" \
    "        \"attribute\": {\n" \
    "            \"type\": \"certTimeValidTo\",\n" \
    "            \"value\": [\n" \
    "                \"20181104223344.0Z\"\n" \
    "            ]\n" \
    "        }\n" \
    "    },\n" \
    "    {\n" \
    "        \"operation\": \"replace\",\n" \
    "        \"attribute\": {\n" \
    "            \"type\": \"certStatus\",\n" \
    "            \"value\": [\n" \
    "                \"1\"\n" \
    "            ]\n" \
    "        }\n" \
    "    }\n" \
    "]")
