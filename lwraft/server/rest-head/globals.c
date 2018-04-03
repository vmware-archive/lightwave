/*
 * Copyright © 2017 VMware, Inc.  All Rights Reserved.
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

PREST_API_DEF gpVdirRestApiDef = NULL;

PVMREST_HANDLE gpVdirRestHTTPHandle = NULL;
PVMREST_HANDLE gpVdirRestHTTPSHandle = NULL;

PVDIR_VMAFD_API gpVdirVmAfdApi = NULL;

PVDIR_REST_HEAD_CACHE gpVdirRestCache = NULL;

PVDIR_REST_CURL_HANDLE_CACHE gpVdirRestCurlHandleCache = NULL;

PVM_METRICS_HISTOGRAM gpRestLdapMetrics[METRICS_LDAP_OP_COUNT]
                                        [METRICS_LDAP_ERROR_COUNT]
                                         [METRICS_LAYER_COUNT];
