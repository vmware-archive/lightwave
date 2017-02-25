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

PSSO_JSON pJsonFromHttpResponse;

SSOERROR
RestDebugJsonObject(
    const void* pDataObject,
    DataObjectToJsonFunc fDataObjectToJson)
{
    SSOERROR e = SSOERROR_NONE;
    PSSO_JSON pJson = NULL;
    PSTRING string = NULL;
    bool isEquals = false;

    if (fDataObjectToJson == (DataObjectToJsonFunc) RestBooleanDataToJson)
    {
        e = SSOJsonBooleanNew(&pJson, *((bool*) pDataObject));
        BAIL_ON_ERROR(e);
    }
    else
    {
        e = SSOJsonObjectNew(&pJson);
        BAIL_ON_ERROR(e);

        e = fDataObjectToJson(pDataObject, pJson);
        BAIL_ON_ERROR(e);
    }

    e = SSOJsonToString(pJson, &string);
    BAIL_ON_ERROR(e);

    fprintf(stdout, "%s\n", "convert return struct back to json:");
    fprintf(stdout, "%s\n\n", string);
    fprintf(stdout, "%s", "compare json from http response and return struct: ");

    e = SSOJsonEquals(pJsonFromHttpResponse, pJson, &isEquals);
    BAIL_ON_ERROR(e);

    fprintf(stdout, "%s\n\n", isEquals ? "PASS" : "FAIL");

    error:

    // cleanup
    SSOJsonDelete(pJson);
    SSOStringFree(string);
    SSOJsonDelete(pJsonFromHttpResponse);

    return e;
}

SSOERROR
RestDebugJsonArray(
    const void* pDataObject,
    DataObjectToJsonFunc fDataObjectToJson)
{
    SSOERROR e = SSOERROR_NONE;
    PSSO_JSON pJson = NULL;
    PSTRING string = NULL;
    bool isEquals = false;

    e = SSOJsonArrayNew(&pJson);
    BAIL_ON_ERROR(e);

    e = fDataObjectToJson(pDataObject, pJson);
    BAIL_ON_ERROR(e);

    e = SSOJsonToString(pJson, &string);
    BAIL_ON_ERROR(e);

    fprintf(stdout, "%s\n", "convert return struct back to json:");
    fprintf(stdout, "%s\n\n", string);
    fprintf(stdout, "%s", "compare json from http response and return struct: ");

    e = SSOJsonEquals(pJsonFromHttpResponse, pJson, &isEquals);
    BAIL_ON_ERROR(e);

    fprintf(stdout, "%s\n\n", isEquals ? "PASS" : "FAIL");

    error:

    // cleanup
    SSOJsonDelete(pJson);
    SSOStringFree(string);
    SSOJsonDelete(pJsonFromHttpResponse);

    return e;
}
