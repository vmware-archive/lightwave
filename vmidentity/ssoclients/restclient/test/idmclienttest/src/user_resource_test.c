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
static PCSTRING name = "test_user_name";
static PSTRING domain = "test_tenant_name";

PCSTRING
IdmUserCreateTest()
{
    SSOERROR e = SSOERROR_NONE;
    REST_SERVER_ERROR* pServerError = NULL;

    bool* pCreated = NULL;

    IDM_PRINCIPAL_DATA* pPrincipal = NULL;
    e = IdmPrincipalDataNew(&pPrincipal, name, domain);
    BAIL_ON_ERROR(e);

    e = IdmUserCreate(pBearerTokenClient, tenant, pPrincipal, &pCreated, &pServerError);
    BAIL_ON_ERROR(e);

    error:

    return RestTestGenerateErrorMessage(__FUNCTION__, e, pServerError);
}

PCSTRING
IdmUserGetTest()
{
    SSOERROR e = SSOERROR_NONE;
    REST_SERVER_ERROR* pServerError = NULL;

    IDM_USER_DATA* pUserReturn = NULL;

    e = IdmUserGet(pBearerTokenClient, tenant, name, domain, &pUserReturn, &pServerError);
    BAIL_ON_ERROR(e);

    error:

    return RestTestGenerateErrorMessage(__FUNCTION__, e, pServerError);
}

PCSTRING
IdmUserGetGroupsTest()
{
    SSOERROR e = SSOERROR_NONE;
    REST_SERVER_ERROR* pServerError = NULL;

    IDM_GROUP_ARRAY_DATA* pGroupArrayReturn = NULL;

    e = IdmUserGetGroups(pBearerTokenClient, tenant, name, domain, true, &pGroupArrayReturn, &pServerError);
    BAIL_ON_ERROR(e);

    error:

    return RestTestGenerateErrorMessage(__FUNCTION__, e, pServerError);
}

PCSTRING
IdmUserDeleteTest()
{
    SSOERROR e = SSOERROR_NONE;
    REST_SERVER_ERROR* pServerError = NULL;

    e = IdmUserDelete(pBearerTokenClient, tenant, name, domain, &pServerError);
    BAIL_ON_ERROR(e);

    error:

    return RestTestGenerateErrorMessage(__FUNCTION__, e, pServerError);
}

PCSTRING
IdmUserGetTestHA()
{
    SSOERROR e = SSOERROR_NONE;
    REST_SERVER_ERROR* pServerError = NULL;

    IDM_USER_DATA* pUserReturn = NULL;

    e = IdmUserGet(pBearerTokenHAClient, tenant, name, domain, &pUserReturn, &pServerError);
    BAIL_ON_ERROR(e);

    error:

    return RestTestGenerateErrorMessage(__FUNCTION__, e, pServerError);
}
