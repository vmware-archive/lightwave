/*
 * Copyright © 2012-2016 VMware, Inc.  All Rights Reserved.
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

#ifndef _PROTOTYPES_H_
#define _PROTOTYPES_H_

// SSO_HTTP_CLIENT_RESPONSE

SSOERROR
SSOHttpClientResponseNew(
    PSSO_HTTP_CLIENT_RESPONSE* pp);

void
SSOHttpClientResponseDelete(
    PSSO_HTTP_CLIENT_RESPONSE p);

PCSTRING
SSOHttpClientResponseGetString(
    PCSSO_HTTP_CLIENT_RESPONSE p);

size_t
SSOHttpClientResponseWriteCallback(
    PSTRING contents,
    size_t size,
    size_t nmemb,
    void* userp);

#endif
