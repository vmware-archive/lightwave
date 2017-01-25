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
static PCSTRING name = "rs_test_resource_server_name";

PCSTRING
IdmResourceServerRegisterTest()
{
    SSOERROR e = SSOERROR_NONE;
    REST_SERVER_ERROR* pServerError = NULL;

    IDM_RESOURCE_SERVER_DATA* pResourceServerReturn = NULL;

    PSTRING pGroupFilterArray[] =
    {
        "test_tenant_name\\group1",
        "test_tenant_name\\group2"
    };
    IDM_STRING_ARRAY_DATA* pGroupFilter = NULL;

    IDM_RESOURCE_SERVER_DATA* pResourceServer = NULL;

    e = IdmStringArrayDataNew(&pGroupFilter, pGroupFilterArray, (size_t) (sizeof(pGroupFilterArray) / sizeof(PSTRING)));
    BAIL_ON_ERROR(e);

    e = IdmResourceServerDataNew(&pResourceServer, name, pGroupFilter);
    BAIL_ON_ERROR(e);

    e = IdmResourceServerRegister(pBearerTokenClient, tenant, pResourceServer, &pResourceServerReturn, &pServerError);
    BAIL_ON_ERROR(e);

    error:

    return RestTestGenerateErrorMessage(__FUNCTION__, e, pServerError);
}

PCSTRING
IdmResourceServerGetAllTest()
{
    SSOERROR e = SSOERROR_NONE;
    REST_SERVER_ERROR* pServerError = NULL;

    IDM_RESOURCE_SERVER_ARRAY_DATA* pResourceServerArrayReturn = NULL;

    e = IdmResourceServerGetAll(pBearerTokenClient, tenant, &pResourceServerArrayReturn, &pServerError);
    BAIL_ON_ERROR(e);

    error:

    return RestTestGenerateErrorMessage(__FUNCTION__, e, pServerError);
}

PCSTRING
IdmResourceServerGetTest()
{
    SSOERROR e = SSOERROR_NONE;
    REST_SERVER_ERROR* pServerError = NULL;

    IDM_RESOURCE_SERVER_DATA* pResourceServerReturn = NULL;

    e = IdmResourceServerGet(pBearerTokenClient, tenant, name, &pResourceServerReturn, &pServerError);
    BAIL_ON_ERROR(e);

    error:

    return RestTestGenerateErrorMessage(__FUNCTION__, e, pServerError);
}

PCSTRING
IdmResourceServerUpdateTest()
{
    SSOERROR e = SSOERROR_NONE;
    REST_SERVER_ERROR* pServerError = NULL;

    IDM_RESOURCE_SERVER_DATA* pResourceServerReturn = NULL;

    PSTRING pGroupFilterArray[] =
    {
        "test_tenant_name\\group3",
        "test_tenant_name\\group4"
    };
    IDM_STRING_ARRAY_DATA* pGroupFilter = NULL;

    IDM_RESOURCE_SERVER_DATA* pResourceServer = NULL;

    e = IdmStringArrayDataNew(&pGroupFilter, pGroupFilterArray, (size_t) (sizeof(pGroupFilterArray) / sizeof(PSTRING)));
    BAIL_ON_ERROR(e);

    e = IdmResourceServerDataNew(&pResourceServer, name, pGroupFilter);
    BAIL_ON_ERROR(e);

    e = IdmResourceServerUpdate(pBearerTokenClient, tenant, name, pResourceServer, &pResourceServerReturn, &pServerError);
    BAIL_ON_ERROR(e);

    error:

    return RestTestGenerateErrorMessage(__FUNCTION__, e, pServerError);
}

PCSTRING
IdmResourceServerDeleteTest()
{
    SSOERROR e = SSOERROR_NONE;
    REST_SERVER_ERROR* pServerError = NULL;

    e = IdmResourceServerDelete(pBearerTokenClient, tenant, name, &pServerError);
    BAIL_ON_ERROR(e);

    error:

    return RestTestGenerateErrorMessage(__FUNCTION__, e, pServerError);
}
