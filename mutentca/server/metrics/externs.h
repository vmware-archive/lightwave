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

extern PVM_METRICS_HISTOGRAM gpRestMetrics[LWCA_METRICS_REQ_URL_COUNT]
                                           [LWCA_METRICS_HTTP_METHOD_COUNT]
                                            [LWCA_METRICS_HTTP_CODE_COUNT];

extern PVM_METRICS_HISTOGRAM gpApiMetrics[LWCA_METRICS_API_COUNT]
                                          [LWCA_METRICS_RESPONSE_COUNT];

extern PVM_METRICS_HISTOGRAM gpSecurityMetrics[LWCA_METRICS_SECURITY_COUNT]
                                               [LWCA_METRICS_RESPONSE_COUNT];
