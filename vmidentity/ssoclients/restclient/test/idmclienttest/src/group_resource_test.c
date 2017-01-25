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
static PSTRING name = "test_group";
static PSTRING domain = "test_tenant_name";

PCSTRING
IdmGroupGetTest()
{
    SSOERROR e = SSOERROR_NONE;
    REST_SERVER_ERROR* pServerError = NULL;

    IDM_GROUP_DATA* pGroupReturn = NULL;

    e = IdmGroupGet(pBearerTokenClient, tenant, name, domain, &pGroupReturn, &pServerError);
    BAIL_ON_ERROR(e);

    error:

    return RestTestGenerateErrorMessage(__FUNCTION__, e, pServerError);
}

PCSTRING
IdmGroupGetMembersTest()
{
    SSOERROR e = SSOERROR_NONE;
    REST_SERVER_ERROR* pServerError = NULL;

    IDM_MEMBER_TYPE memberType = IDM_MEMBER_TYPE_USER;
    size_t limit = 10;
    IDM_SEARCH_RESULT_DATA* pSearchResultReturn = NULL;

    e = IdmGroupGetMembers(pBearerTokenClient, tenant, name, domain, memberType, limit, &pSearchResultReturn, &pServerError);
    BAIL_ON_ERROR(e);

    error:

    return RestTestGenerateErrorMessage(__FUNCTION__, e, pServerError);
}

PCSTRING
IdmGroupGetParentsTest()
{
    SSOERROR e = SSOERROR_NONE;
    REST_SERVER_ERROR* pServerError = NULL;

    IDM_GROUP_ARRAY_DATA* pGroupArrayReturn = NULL;

    e = IdmGroupGetParents(pBearerTokenClient, tenant, name, domain, &pGroupArrayReturn, &pServerError);
    BAIL_ON_ERROR(e);

    error:

    return RestTestGenerateErrorMessage(__FUNCTION__, e, pServerError);
}
