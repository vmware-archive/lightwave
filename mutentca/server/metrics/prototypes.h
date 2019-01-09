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

#ifdef __cplusplus
extern "C" {
#endif

// util.c
PSTR
LwCAMetricsReqUrlString(
    LWCA_METRICS_REQ_URLS           reqUrl
    );

PSTR
LwCAMetricsHttpMethodString(
    LWCA_METRICS_HTTP_METHODS       method
    );

PSTR
LwCAMetricsHttpStatusCodeString(
    LWCA_METRICS_HTTP_CODES         code
    );

PSTR
LwCAMetricsApiNameString(
    LWCA_METRICS_API_NAMES          api
    );

PSTR
LwCAMetricsSecurityApiNameString(
    LWCA_METRICS_SECURITY_APIS api
    );

PSTR
LwCAMetricsResponseString(
    LWCA_METRICS_RESPONSE_CODES     code
    );

#ifdef __cplusplus
}
#endif
