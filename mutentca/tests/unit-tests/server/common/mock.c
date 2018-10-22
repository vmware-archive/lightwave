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

#include "includes.h"

#define DUMMY_CERTIFICATE "-----BEGIN CERTIFICATE-----\n" \
    "MIID5TCCAs2gAwIBAgIJAONKhmr803trMA0GCSqGSIb3DQEBCwUAMGkxIDAeBgNV\n" \
    "BAMMF0NBLERDPWx3LXRlc3Rkb20sREM9Y29tMQswCQYDVQQGEwJVUzE4MDYGA1UE\n" \
    "CgwvbHd0LXByYXNvb250LWktMDRhNmE5ZDQxYWU2MGU5NTAubHctdGVzdGRvbS5j\n" \
    "b20wHhcNMTgwOTI4MTczMDI5WhcNMjgwOTIyMTcxMTU4WjBGMUQwQgYDVQQDDDtt\n" \
    "dXRlbnRjYS1wb3N0LTY2LWktMGY5MGQwOTU4OWYxMjRmODYubHctdGVzdGRvbS5j\n" \
    "b20tc2VydmljZTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALZKdjuu\n" \
    "EKvpnsr4qU7IPX+lJo6eoSCTX/dZs6gs+pyXshkkfLR1um+vcRNqiJF8SjsRflXv\n" \
    "Si6tzE2Cf2i8k6Tu2EVBhXvrZRVDnjlxVlUam5K9nLonHqT+w77xhcNugHJU6wet\n" \
    "xm/kFOUVqQkCn9ehRNivFSAAQgkUgTQS1b9AeMFFKq6rvjwi/vqWm5KaRbVMNYj7\n" \
    "2T+MT57i5mJpJVgzHRMoMBmEhYNC7kos0Qdej5Ud8o4UuAjRktsN0f+f9Pm9r4ku\n" \
    "4NC2p31B1N5od9fM4Jtj2fWbSoTNMhctJ1GBhCicuPgX3DEu1ASVZnIx9k0CDE/n\n" \
    "uUHSkSwBfwPQYd0CAwEAAaOBsjCBrzALBgNVHQ8EBAMCBeAwHQYDVR0OBBYEFIc1\n" \
    "FeBfbMLmSQNkVffWJUwlzKNhMB8GA1UdIwQYMBaAFK69Ssu1s5g6ixVIN/luNvxm\n" \
    "CyODMGAGCCsGAQUFBwEBBFQwUjBQBggrBgEFBQcwAoZEaHR0cHM6Ly9sd3QtcHJh\n" \
    "c29vbnQtaS0wNGE2YTlkNDFhZTYwZTk1MC5sdy10ZXN0ZG9tLmNvbS9hZmQvdmVj\n" \
    "cy9zc2wwDQYJKoZIhvcNAQELBQADggEBADzpitGGg+nWAmvjU/jxYvRU3GFZjG25\n" \
    "Mot99e+LloRrbpPS3YIVLdNkKG7uU2ndXebJmwG+McgY1ZCgk31FcBFkM/eeeWvI\n" \
    "soR6dcK63nDO7t9tSygYSriTZnMbGhsKCK9bXbPt8RrjmZN8rnRgaXKf1J9vgy6X\n" \
    "SRGV8DAEMzURA7fFPK2R36CyKaBTS+7MfQ9NRKuBU8wETzVWqgFBRYlVzYuVdcCW\n" \
    "v59zouvgwjoQ/qB3LKh4y2w7aPfPXRrI3P3WZZi+xZSsdK2uzjrU+Wh1GNCEKdDz\n" \
    "+D1z/13oztwrHTqAdq8BflLG9gpS1jZ+tVqJTiL2r2TBP27fC/9rNVM=\n" \
    "-----END CERTIFICATE-----"

#define DUMMY_KEY "-----BEGIN PRIVATE KEY-----\n" \
    "MIIEvwIBADANBgkqhkiG9w0BAQEFAASCBKkwggSlAgEAAoIBAQC2SnY7rhCr6Z7K\n" \
    "+KlOyD1/pSaOnqEgk1/3WbOoLPqcl7IZJHy0dbpvr3ETaoiRfEo7EX5V70ourcxN\n" \
    "gn9ovJOk7thFQYV762UVQ545cVZVGpuSvZy6Jx6k/sO+8YXDboByVOsHrcZv5BTl\n" \
    "FakJAp/XoUTYrxUgAEIJFIE0EtW/QHjBRSquq748Iv76lpuSmkW1TDWI+9k/jE+e\n" \
    "4uZiaSVYMx0TKDAZhIWDQu5KLNEHXo+VHfKOFLgI0ZLbDdH/n/T5va+JLuDQtqd9\n" \
    "QdTeaHfXzOCbY9n1m0qEzTIXLSdRgYQonLj4F9wxLtQElWZyMfZNAgxP57lB0pEs\n" \
    "AX8D0GHdAgMBAAECggEBAKMUPm3prRR+7gZbKuxGC26USsgwdal07teUMjtz8cgB\n" \
    "ld0UXVP8NowQAjMCjabJ8IajgchTSKQ1654z7tO+UL+uxWA/27Eex9GcKoLtN/Tu\n" \
    "1tsSMpH+yVWGbYv2+ln6E4M1IQY/mlUjUOYY2J4s8N6xcawABgAY+vqwk14Is5xi\n" \
    "Bul+fDP3r4paF32x0oWkNsGxrbtrIJm3l9JrYOgQiehU4IWa2ps6PwqPyVoFMN1r\n" \
    "pKnLDy+OfA4SnSIR0dS/A6rKlliCd2fz1dV2TnRgWriejiqbOqGndP0HF/x1To7P\n" \
    "utlnW6nn51sH8Rqm2ifnz8dVQJIbTCNwirods/stTWkCgYEA7VqJdto3iKvFITd7\n" \
    "N144c/M+glk7bRwstW1McAtCBNzameW1lfAjjHrWkeER0yAgTHlMCUIejJrw5bEF\n" \
    "OYD7C3JWWnhUtB7ADHIZ4bPpDSgqETqZy+GLJtmVRh8/e9HSu4uJytH+bGPGKN3D\n" \
    "q4Uhbcy+njazHgKyxZispF3k+t8CgYEAxJyMA6vY5BrX2r3V3oSAb0DTHVP9HWQ2\n" \
    "j3zA/CDYrk5plMfWGhz99/kD383YBn5eoBIwYLJYd4qPzlq0pfv7O8QRnZYcNACs\n" \
    "ukibkBcXDkofAyQA3WyqOb44czSbQ3Bj9o0wLiU+1WCenCwjPydbD0klidGma0ZD\n" \
    "vQhbXkgN9sMCgYEAvYeKNSnTJoNjCj8vWOsX0R7vT07JsTrKSKVaYC3dLdzdgf53\n" \
    "nZ8AA/Ei3aIHhbslj2tcWBhAJK6kpgVQ0rY3wNAwPFXR7XKQkF+cYyoycpsb3F4a\n" \
    "6T7UytT76d6R5uGJkNC/2wEMVKjnV0KeOJw6MV9I5zVumgw1jKt5UFbZvakCgYBm\n" \
    "hnfg5jkI4hKTp5CiI5hR64zra0O/kuV8t4JvN7WmV49pHThBj8LED598knvHDxjL\n" \
    "AhEmK3pEhAXvho9TSu48wUaqVX/JglyQG5K7db75HZ/EZH7q+GJgEBihwtoANdh6\n" \
    "1i88sQFuBZ9QMxgCM8ShWPRxRNkycYabSYNTy+E0PwKBgQCuYvJpnt0v/eX1xhPY\n" \
    "UURVtK7s9vQlCfncJHdRRpzMj7XnZHxG4AgzqMslj0f6rloKoshBJKey+PszNuPw\n" \
    "HOyqj77PvWd/OjElBkI9wRY/0vNZYwUKloNvjAk3xnyMc7JHsiXaWOfEEPDza70O\n" \
    "/qSHelwXqR7ZhXDH2aB5ZJ5m5g==\n" \
    "-----END PRIVATE KEY-----"

#define DUMMY_ID_TOKEN "eyJhbGciOiJSUzI1NiJ9.eyJzdWIiOiJtdXRlbnRjYS1wb3" \
    "N0LTY2LWktMGY5MGQwOTU4OWYxMjRmODYubHctdGVzdGRvbS5jb20tc2VydmljZUBs" \
    "dy10ZXN0ZG9tLmNvbSIsImlzcyI6Imh0dHBzOlwvXC9sd3QtcHJhc29vbnQtRUxCLT" \
    "k5OTA0NjU0MC51cy13ZXN0LTIuZWxiLmFtYXpvbmF3cy5jb21cL29wZW5pZGNvbm5l" \
    "Y3RcL2x3LXRlc3Rkb20uY29tIiwiZ3JvdXBzIjpbImx3LXRlc3Rkb20uY29tXFxDQU" \
    "FkbWlucyIsImx3LXRlc3Rkb20uY29tXFxTb2x1dGlvblVzZXJzIiwibHctdGVzdGRv" \
    "bS5jb21cXEV2ZXJ5b25lIl0sInRva2VuX2NsYXNzIjoiaWRfdG9rZW4iLCJ0b2tlbl" \
    "90eXBlIjoiaG90ay1wayIsImhvdGsiOnsia2V5cyI6W3sia3R5IjoiUlNBIiwiZSI6" \
    "IkFRQUIiLCJ1c2UiOiJzaWciLCJhbGciOiJSUzI1NiIsIm4iOiJ0a3AyTzY0UXEtbW" \
    "V5dmlwVHNnOWY2VW1qcDZoSUpOZjkxbXpxQ3o2bkpleUdTUjh0SFc2YjY5eEUycUlr" \
    "WHhLT3hGLVZlOUtMcTNNVFlKX2FMeVRwTzdZUlVHRmUtdGxGVU9lT1hGV1ZScWJrcj" \
    "JjdWljZXBQN0R2dkdGdzI2QWNsVHJCNjNHYi1RVTVSV3BDUUtmMTZGRTJLOFZJQUJD" \
    "Q1JTQk5CTFZ2MEI0d1VVcXJxdS1QQ0wtLXBhYmtwcEZ0VXcxaVB2WlA0eFBudUxtWW" \
    "1rbFdETWRFeWd3R1lTRmcwTHVTaXpSQjE2UGxSM3lqaFM0Q05HUzJ3M1JfNV8wLWIy" \
    "dmlTN2cwTGFuZlVIVTNtaDMxOHpnbTJQWjladEtoTTB5RnkwblVZR0VLSnk0LUJmY0" \
    "1TN1VCSlZtY2pIMlRRSU1ULWU1UWRLUkxBRl9BOUJoM1EifV19LCJhdWQiOiJtdXRl" \
    "bnRjYS1wb3N0LTY2LWktMGY5MGQwOTU4OWYxMjRmODYubHctdGVzdGRvbS5jb20tc2" \
    "VydmljZUBsdy10ZXN0ZG9tLmNvbSIsInNjb3BlIjoiYXRfZ3JvdXBzIHJzX2FkbWlu" \
    "X3NlcnZlciBvcGVuaWQgcnNfdm1jYSByc192bWRpciBpZF9ncm91cHMgcnNfcG9zdC" \
    "IsIm11bHRpX3RlbmFudCI6dHJ1ZSwiZXhwIjoxNTQxMjI3MTgwLCJpYXQiOjE1Mzg2" \
    "MzUxODAsImp0aSI6IlZsWXR4VkwzbFoxRTAwcnhkLTNUTVNqQ3pTc0s1NlZWNFptSE" \
    "Q3SkZKeDQiLCJ0ZW5hbnQiOiJsdy10ZXN0ZG9tLmNvbSJ9.MUbb4WEE9zOgqmM0XOL" \
    "30GLBTST3eb4DklT1SMw_uyxXTKzv4_Q93vrDVSnz9TeKai-70l_406XHv5NzBkpT0" \
    "lARZcq5c9xg6rtabwya6lxCCPrZDdvZsxe0CgB5SJa1sOJjmzNkvgJwMx7U80UX7Aj" \
    "OLixUK_v3218wTMltKnscEp4lreA8oZJ-KudvmCCHduTCTKVmeUKPHzT1hxoYABxLl" \
    "2ukbsiMbyLPNZoyUcCSX3vNDElcSZUmDAoIsVtowFQYDiTyE0A59Bpcvqw7_8RoW3N" \
    "4NR9PqNT99BBwZRMxAWrWJ95njqHY742fDuGZ0T7kEfMT8Pmkm9_J6Mj0aA"

#define DUMMY_ACCESS_TOKEN "eyJhbGciOiJSUzI1NiJ9.eyJzdWIiOiJtdXRlbnRjYS" \
    "1wb3N0LTY2LWktMGY5MGQwOTU4OWYxMjRmODYubHctdGVzdGRvbS5jb20tc2Vydmlj" \
    "ZUBsdy10ZXN0ZG9tLmNvbSIsImlzcyI6Imh0dHBzOlwvXC9sd3QtcHJhc29vbnQtRU" \
    "xCLTk5OTA0NjU0MC51cy13ZXN0LTIuZWxiLmFtYXpvbmF3cy5jb21cL29wZW5pZGNv" \
    "bm5lY3RcL2x3LXRlc3Rkb20uY29tIiwiZ3JvdXBzIjpbImx3LXRlc3Rkb20uY29tXF" \
    "xDQUFkbWlucyIsImx3LXRlc3Rkb20uY29tXFxTb2x1dGlvblVzZXJzIiwibHctdGVz" \
    "dGRvbS5jb21cXEV2ZXJ5b25lIl0sInRva2VuX2NsYXNzIjoiYWNjZXNzX3Rva2VuIi" \
    "widG9rZW5fdHlwZSI6ImhvdGstcGsiLCJob3RrIjp7ImtleXMiOlt7Imt0eSI6IlJT" \
    "QSIsImUiOiJBUUFCIiwidXNlIjoic2lnIiwiYWxnIjoiUlMyNTYiLCJuIjoidGtwMk" \
    "82NFFxLW1leXZpcFRzZzlmNlVtanA2aElKTmY5MW16cUN6Nm5KZXlHU1I4dEhXNmI2" \
    "OXhFMnFJa1h4S094Ri1WZTlLTHEzTVRZSl9hTHlUcE83WVJVR0ZlLXRsRlVPZU9YRl" \
    "dWUnFia3IyY3VpY2VwUDdEdnZHRncyNkFjbFRyQjYzR2ItUVU1UldwQ1FLZjE2RkUy" \
    "SzhWSUFCQ0NSU0JOQkxWdjBCNHdVVXFycXUtUENMLS1wYWJrcHBGdFV3MWlQdlpQNH" \
    "hQbnVMbVlta2xXRE1kRXlnd0dZU0ZnMEx1U2l6UkIxNlBsUjN5amhTNENOR1MydzNS" \
    "XzVfMC1iMnZpUzdnMExhbmZVSFUzbWgzMTh6Z20yUFo5WnRLaE0weUZ5MG5VWUdFS0" \
    "p5NC1CZmNNUzdVQkpWbWNqSDJUUUlNVC1lNVFkS1JMQUZfQTlCaDNRIn1dfSwiYXVk" \
    "IjpbIm11dGVudGNhLXBvc3QtNjYtaS0wZjkwZDA5NTg5ZjEyNGY4Ni5sdy10ZXN0ZG" \
    "9tLmNvbS1zZXJ2aWNlQGx3LXRlc3Rkb20uY29tIiwicnNfYWRtaW5fc2VydmVyIiwi" \
    "cnNfdm1jYSIsInJzX3ZtZGlyIiwicnNfcG9zdCJdLCJzY29wZSI6ImF0X2dyb3Vwcy" \
    "Byc19hZG1pbl9zZXJ2ZXIgb3BlbmlkIHJzX3ZtY2EgcnNfdm1kaXIgaWRfZ3JvdXBz" \
    "IHJzX3Bvc3QiLCJtdWx0aV90ZW5hbnQiOnRydWUsImV4cCI6MTU0MTIyNzE4MCwiaW" \
    "F0IjoxNTM4NjM1MTgwLCJqdGkiOiJTYlpFMjFuejZIZzBoZmJSc0d6azEtbGhoLTh5" \
    "Q2dtZkRZU0hFcUl1Y2hFIiwidGVuYW50IjoibHctdGVzdGRvbS5jb20iLCJhZG1pbl" \
    "9zZXJ2ZXJfcm9sZSI6Ikd1ZXN0VXNlciJ9.IwmxijWz78zjl9Bbh8iirkgMVlVlhRr" \
    "6VmEMlee8LzJhrqKnVO_dHpbXeUPrlOSos2WbSudeHmcVwQkDmyEG5KGw_Z4ODKfzc" \
    "JxYIB2Lfk7KPN0pN6N3R0KI1Zv77Vp7Ucg1U_TV3G8bBTF_ZkQnK6yjnm87HvAEuDT" \
    "rpNtu7X-M2WqXPLlbw7sScZ5nXFdr1xPHwtViVR9kNLLnKX_Ua4p99dhDoKCg9iUYX" \
    "AeXmFvpiE4krdo4GTrhwCeZt3588Chwzw1uyfrSc8SBL82Yxhlb1GJOAtgo4yo6PrJ" \
    "hX3kLgSsMb0opr8kgNq8SOKwQe4UXp6vlhweqdbuxeFxskg"

#define DUMMY_REFRESH_TOKEN ""

#define DUMMY_JSON_RESPONSE "{" \
    "\"id_token\": \"" DUMMY_ID_TOKEN "\", " \
    "\"access_token\": \"" DUMMY_ACCESS_TOKEN "\"" \
    "}"

#define DUMMY_CERT_SUBJECT_DN \
    "CN=mutentca-post-66-i-0f90d09589f124f86.lw-testdom.com-service"

DWORD
__wrap_LwCAGetVecsMutentCACert(
    PLWCA_CERTIFICATE   *ppszCert,
    PSTR                *ppszKey
    )
{
    DWORD               mock_val = mock();
    DWORD               dwError = 0;
    PLWCA_CERTIFICATE   pszCert = NULL;
    PSTR                pszKey = NULL;

    dwError = LwCAAllocateStringA(DUMMY_CERTIFICATE, &pszCert);
    assert_int_equal(dwError, 0);
    dwError = LwCAAllocateStringA(DUMMY_KEY, &pszKey);
    assert_int_equal(dwError, 0);

    *ppszCert = pszCert;
    *ppszKey = pszKey;

    return mock_val;
}

SSOERROR
__wrap_OidcClientBuild(
    POIDC_CLIENT* pp,
    PCSTRING pszServer,
    int portNumber,
    PCSTRING pszTenant,
    PCSTRING pszClientID,
    PCSTRING pszTlsCAPath
    )
{
    DWORD mock_val = mock();

    return mock_val;
}

SSOERROR
__wrap_OidcClientAcquireTokensBySolutionUserCredentials(
    PCOIDC_CLIENT pOidcClient,
    PCSTRING pszCertificateSubjectDN,
    PCSTRING pszPrivateKeyPEM,
    PCSTRING pszScope,
    POIDC_TOKEN_SUCCESS_RESPONSE* ppOutTokenSuccessResponse,
    POIDC_ERROR_RESPONSE* ppOutTokenErrorResponse
    )
{
    SSOERROR                        dwError = 0;
    POIDC_TOKEN_SUCCESS_RESPONSE    pSuccess = NULL;
    DWORD                           mock_val = 0;

    assert_string_equal(pszCertificateSubjectDN, DUMMY_CERT_SUBJECT_DN);

    mock_val = mock();
    if (mock_val)
    {
        goto error;
    }
    dwError = OidcTokenSuccessResponseParse(
        &pSuccess,
        DUMMY_JSON_RESPONSE
        );
    assert_int_equal(dwError, 0);

    *ppOutTokenSuccessResponse = pSuccess;

error:
    return mock_val;
}
