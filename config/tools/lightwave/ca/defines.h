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

#define VMW_DEFAULT_AFD_SERVER "localhost"

#define VMW_OIDC_PORT 443
#define VMW_OIDC_CA_SCOPE "openid at_groups rs_vmca"

#define VMW_CA_REST_URL_FORMAT_IPV4 "https://%s:%s%s"
#define VMW_CA_REST_URL_FORMAT_IPV6 "https://[%s]:%s%s"
#define VMW_CA_BEARER_AUTH_FORMAT "Authorization: Bearer %s"

#define VMW_CA_HTTPS_PORT "7778"
#define VMW_CA_CERT_REST_ENDPOINT "/v1/vmca/certificates"
#define VMW_CA_CERT_REST_REQUEST_METHOD "PUT"
#define VMW_CA_CERT_BUNDLE_PATH "/etc/ssl/certs"

#define VMW_CA_DEFAULT_KEYSIZE 2048

#define VMW_CA_ASCII_DIGIT(c)    ( (c) >= '0' && (c) <= '9' )
#define VMW_CA_ASCII_aTof(c)     ( (c) >= 'a' && (c) <= 'f' )
#define VMW_CA_ASCII_AToF(c)     ( (c) >= 'A' && (c) <= 'F' )

#define VMW_CA_TIME_SECS_PER_MINUTE     ( 60 )
#define VMW_CA_TIME_SECS_PER_HOUR       ( 60 * VMW_CA_TIME_SECS_PER_MINUTE )
#define VMW_CA_TIME_SECS_PER_DAY        ( 24 * VMW_CA_TIME_SECS_PER_HOUR )
#define VMW_CA_TIME_SECS_PER_WEEK       (  7 * VMW_CA_TIME_SECS_PER_DAY )
#define VMW_CA_TIME_SECS_PER_YEAR       (365 * VMW_CA_TIME_SECS_PER_DAY )

#define VMW_CA_CERT_EXPIRY_START_LAG    (  1 * VMW_CA_TIME_SECS_PER_DAY )
#define VMW_CA_DEFAULT_CERT_VALIDITY    ( 10 * VMW_CA_TIME_SECS_PER_YEAR )

#define VMW_CA_DEFAULT_ERROR 1
