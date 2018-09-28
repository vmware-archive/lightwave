/*
 * Copyright © 2018 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the ?~@~\License?~@~]); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an ?~@~\AS IS?~@~] BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */

#include "includes.h"
#include "vmsignaturetest.h"

#define REQUEST_URI "/v1/vmdir/ldap?dn=cn%3DDSE%20Root"
#define EMPTY_REQUEST_BODY ""
#define EMPTY_WHITESPACE_REQUEST_BODY "    "
#define NON_EMPTY_REQUEST_BODY "{\"dn\": \"cn=Root\"}"
#define NON_EMPTY_WHITESPACE_REQUEST_BODY "    {\"dn\": \"cn=Root\"}   "
#define PEM_VALUE "-----BEGIN PRIVATE KEY-----\n" \
    "MIIEvQIBADANBgkqhkiG9w0BAQEFAASCBKcwggSjAgEAAoIBAQDPgmNO3WhXEzGb\n" \
    "wYwSa4Rtv6WjwMk/4N4YKx+BNjNHsqtftnxWgtSaJ5kZf5GbzCntltCRJrZAi+LL\n" \
    "Xl9+bwpbfvEHcqGTis0XpAVAJi2FoV7Q1qvZaL1LuzqrBsrj6e7kFXg+SdVJYEMl\n" \
    "huz2kaVld+gIwWoZGkJXV01hLBvCvhbf6jUM8py/LPIRTafUCXYU91SMV3hAJlUK\n" \
    "EC9ef5KhEcG/tKJTJiCjRhUzSVn+DNIt6oqkuZuzrG0GxjH4Es0HUKUlfVAKNGGg\n" \
    "uiNBFv0wGLd8r6no0RZjZ/f+26jsd+KSufMxA4/hv8NUWHYX+KjXBxFeymF/35NE\n" \
    "AfOR1IGRAgMBAAECggEALypDyDkq7h++tthXLhiiEQ/uZjn7hDloQbYLgyL+pN1H\n" \
    "donT2RYDnMZDVjhYsEDHhukwns1dv24MLo1UxzgV8pX30umLMC3sT+NIDjyfPDBh\n" \
    "jLY+eTwFSqFTxGvW0dbqJY17tyyw/eyTVoREeAbCwT1v0A2mP+5oBdIiFGQiKzEA\n" \
    "1hw1ZH2uRINhrm2nk5RY+Fo+1iQ0R8P/FoGDapwhek+prs5ASUz6RiaB29fXnUOu\n" \
    "U/alcoJadTSwMGKs+MyB9NxvePMdzynvibudDFKamMVDr+8rH4q1W6MUbPeNG+6t\n" \
    "iMmRQWLTUSNSAt3IXU1TIy9msqio9iGn0UW/pGDBwQKBgQDrLYRyhSSVyhqQcLcZ\n" \
    "Y/73IUt44dmiX3hJd7o2papF61vPoOD471GbPN8d9+1ZTPmmK89rKC7JGmTwnrAc\n" \
    "vyWUxEKsEHVPgioAW3GIHUvCFJ+3QKd31TJG6FtC6U7Ipkg7Jdg1fz8dAsGkDQzr\n" \
    "M+a2qPbnVGCQhr5QyvTnsBTzSwKBgQDh4b5g5yrrsn/9OyJqHrgQMcKOqrbZy+ol\n" \
    "g3oY9eKQyt0yw/MGM3teTND3vc4OoKhIeVva3F+0OMdBeaKjvHnSOS7lMQi1+Xiq\n" \
    "oBurOhKa4KF7m6xwhIi6eUE55Tli05hBjpTZ1BGipfWLPJMeFgR2+7MNrtu6/v8y\n" \
    "v8LJtth5EwKBgD+b+hz3giO5nGHA+uU1NLsnGEDD5ZeTdvd4GLe7K0jqFdUc0HzE\n" \
    "TjBM0JRMgLgMp+GYK+vx3GWsa+zhHwVHdiS8yMO9V91/eVjLYvPL6Le/2Es6g5FX\n" \
    "6tde9YAtlo+S2iqVW9tsZWe1XSbAGORtD1QMc2uyWMP+je9QVdlsuN3tAoGBAImL\n" \
    "b+UL/uvFJwCpWJxsFkLuOf2r4GohLHQpalj2qT7n+Ofnv/1/rdjPR9QDxQzNMYvo\n" \
    "MlHELZbNmC+7hJjVtQyY+4BeamJ9BchIdHDAjQG4VTyyn79oROXf3iMF/Pq7xBPv\n" \
    "YZ5+7zoueqWNfjwsxg052V80oeJtUjKx6EfySkD3AoGAYdyCsAy0c9BCfwl1wLOe\n" \
    "rnxYoAp5gOyTTUpcSFWLsX6ErNpSB/+GMSUGywGrK9ygqlbrKcWDqpoSYH6R9THk\n" \
    "xiwLn0bzvBRMA3h0omXHlOqqs7oy3OUWmu8E1zmwb77z0QON0OW4dmuXLD9FwYkb\n" \
    "9ajbCEBBTXZzaOmFYpTNBJ4=\n" \
    "-----END PRIVATE KEY-----"
#define EMPTY_BODY_SIGNATURE "722e65643306548c7cbbfae9a00c217c615a352a3" \
    "5555b3ceae856f8c868caa34deac85df7b8ae91f2c7264c89697f5fc3e5d9f27bd" \
    "94bf0d540832f1cc67e3f5c83492839058477e6de7491b6700296040868daf5d57" \
    "3cd33ad995efdfabe4ed2d649469f963c40d93dd48aed1b09ac6d532bb517d7550" \
    "c5676b5f965a55f79c4a99006917c603ce283294bb9a0bd1a42aa471b4b03be929" \
    "6d72ac03582b0913359c9e08b58d01d0f494c6af727117e90589bd8e6a38907de1" \
    "10e2b1a3dbae4d04102dc5bf6a54a65091ce3a25d97c51412b08b285bc159dcbe9" \
    "ea7aec986201a63afb95bde76a119bb36966c1968139dc9e5a5bc5f182779fbf0a" \
    "597b7153f"
#define NON_EMPTY_BODY_SIGNATURE "9b6d6e30b5759188c6446bfcc35d7fb8c4b12" \
    "d029693b7a9f27e089d1ba85795664c1b79833910aa03738c937c1af8fd19dc409" \
    "14d1e6c769ff4d6de83ac410f29c8d7e09601e1d2da4977d2ad25ff03a0e1794cd" \
    "a9e9b598b1812aab3c66d9f9a8daad3af3b1d105645ed111312eb2e523f98a51d5" \
    "9094cbb4ab5ca5735a34fd650daa247f2411f73e4c445a7001da2b7f305fe9e398" \
    "e65eedc191525ad7889f6b6813d132e57cc2400b1a4c2490bc373e97c98fb93291" \
    "9ef0283cb29630bd705fc8f03e69cc2805a1a5f692c7d59ae304d8c189ac4fb20f" \
    "09985a5665eb06373749593f2a2449cee1f7cc6bb987f204cc6f989be38cc6ea57" \
    "b503786f6a9ed"

#define REQUEST_TIMESTAMP "Thu, 11 Oct 2018 17:42:39 GMT"

static
DWORD
_VmHttpClientRequestPOPSignatureTest(
    PSTR pTestMsg,
    PSTR pRequestBody,
    PSTR pExpectedSignature
    )
{
    DWORD           dwError = 0;
    VM_HTTP_METHOD  vmHttpMethod = 0;
    PSTR            pszRequestURI = NULL;
    PSTR            pszPEM = NULL;
    PSTR            pszRequestTime = NULL;
    PSTR            pszSignature = NULL;

    vmHttpMethod = VMHTTP_METHOD_GET;
    pszRequestURI = REQUEST_URI;
    pszPEM = PEM_VALUE;
    pszRequestTime = REQUEST_TIMESTAMP;

    dwError = VmHttpClientRequestPOPSignature(
        vmHttpMethod,
        pszRequestURI,
        pRequestBody,
        pszPEM,
        pszRequestTime,
        &pszSignature
        );
    BAIL_ON_VM_COMMON_ERROR(dwError);

    if (strcmp(pszSignature, pExpectedSignature))
    {
        printf("FAIL: %s HOTK signature does not match with the expected.\n",
               pTestMsg);
        // setting the error to a recognizable code
        dwError = VM_COMMON_ERROR_OPENSSL_FAILURE;
        BAIL_ON_VM_COMMON_ERROR(dwError);
    }
    else
    {
        printf("PASS: %s HOTK signature matches with the expected.\n",
               pTestMsg);
    }

cleanup:
    VM_COMMON_SAFE_FREE_STRINGA(pszSignature);
    return dwError;

error:
    goto cleanup;
}

DWORD
VmSignatureTest(
    VOID
    )
{
    DWORD   dwError = 0;

    dwError = _VmHttpClientRequestPOPSignatureTest("Empty Body",
                                                   EMPTY_REQUEST_BODY,
                                                   EMPTY_BODY_SIGNATURE
                                                   );

    dwError = _VmHttpClientRequestPOPSignatureTest("Empty Whitespace Body",
                                                   EMPTY_WHITESPACE_REQUEST_BODY,
                                                   EMPTY_BODY_SIGNATURE
                                                   );
    dwError += _VmHttpClientRequestPOPSignatureTest("Non Empty Body",
                                                    NON_EMPTY_REQUEST_BODY,
                                                    NON_EMPTY_BODY_SIGNATURE
                                                    );
    dwError += _VmHttpClientRequestPOPSignatureTest("Non Empty Whitespace Body",
                                                    NON_EMPTY_WHITESPACE_REQUEST_BODY,
                                                    NON_EMPTY_BODY_SIGNATURE
                                                    );
    dwError += _VmHttpClientRequestPOPSignatureTest("Null Body Request",
                                                    NULL,
                                                    EMPTY_BODY_SIGNATURE);

    return dwError;
}
