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

static PCSTRING tenant = "test_tenant_name";

PCSTRING
IdmDiagnosticsClearEventLogTest()
{
    SSOERROR e = SSOERROR_NONE;
    REST_SERVER_ERROR* pServerError = NULL;

    e = IdmDiagnosticsClearEventLog(pBearerTokenClient, tenant, &pServerError);
    BAIL_ON_ERROR(e);

    error:

    return RestTestGenerateErrorMessage(__FUNCTION__, e, pServerError);
}

PCSTRING
IdmDiagnosticsGetEventLogTest()
{
    SSOERROR e = SSOERROR_NONE;
    REST_SERVER_ERROR* pServerError = NULL;

    IDM_EVENT_LOG_ARRAY_DATA* ppEventLogArrayReturn = NULL;

    e = IdmDiagnosticsGetEventLog(pBearerTokenClient, tenant, &ppEventLogArrayReturn, &pServerError);
    BAIL_ON_ERROR(e);

    error:

    return RestTestGenerateErrorMessage(__FUNCTION__, e, pServerError);
}

PCSTRING
IdmDiagnosticsGetEventLogStatusTest()
{
    SSOERROR e = SSOERROR_NONE;
    REST_SERVER_ERROR* pServerError = NULL;

    IDM_EVENT_LOG_STATUS_DATA* ppEventLogStatusReturn = NULL;

    e = IdmDiagnosticsGetEventLogStatus(pBearerTokenClient, tenant, &ppEventLogStatusReturn, &pServerError);
    BAIL_ON_ERROR(e);

    error:

    return RestTestGenerateErrorMessage(__FUNCTION__, e, pServerError);
}

PCSTRING
IdmDiagnosticsStartEventLogTest()
{
    SSOERROR e = SSOERROR_NONE;
    REST_SERVER_ERROR* pServerError = NULL;

    size_t size = 100;

    e = IdmDiagnosticsStartEventLog(pBearerTokenClient, tenant, size, &pServerError);
    BAIL_ON_ERROR(e);

    error:

    return RestTestGenerateErrorMessage(__FUNCTION__, e, pServerError);
}

PCSTRING
IdmDiagnosticsStopEventLogTest()
{
    SSOERROR e = SSOERROR_NONE;
    REST_SERVER_ERROR* pServerError = NULL;

    e = IdmDiagnosticsStopEventLog(pBearerTokenClient, tenant, &pServerError);
    BAIL_ON_ERROR(e);

    error:

    return RestTestGenerateErrorMessage(__FUNCTION__, e, pServerError);
}
