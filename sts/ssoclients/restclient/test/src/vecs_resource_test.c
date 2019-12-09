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

#include "includes.h"

PCSTRING
AfdVecsGetSSLCertificatesTest()
{
    SSOERROR e = SSOERROR_NONE;
    PREST_CLIENT pRestClient = NULL;

    REST_SERVER_ERROR* pServerError = NULL;

    REST_CERTIFICATE_ARRAY_DATA* pCertificateArrayReturn = NULL;

    e = RestTestBearerTokenClientNew(pscHost, pscPort, testTenant, testTenantUsername, testTenantPassword, false, &pRestClient);
    BAIL_ON_ERROR(e);

    e = AfdVecsGetSSLCertificates(pRestClient, &pCertificateArrayReturn, &pServerError);
    BAIL_ON_ERROR(e);

    error:

    // cleanup
    RestClientDelete(pRestClient);
    RestCertificateArrayDataDelete(pCertificateArrayReturn);

    return RestTestGenerateErrorMessage(__FUNCTION__, e, pServerError);
}
