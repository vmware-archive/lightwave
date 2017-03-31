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

SSOERROR
IdmPasswordResetRequestDataNew(
    IDM_PASSWORD_RESET_REQUEST_DATA** ppPasswordResetRequest,
    PCSTRING currentPassword,
    PCSTRING newPassword)
{
    SSOERROR e = SSOERROR_NONE;
    IDM_PASSWORD_RESET_REQUEST_DATA* pPasswordResetRequest = NULL;

    if (ppPasswordResetRequest == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = SSOMemoryAllocate(sizeof(IDM_PASSWORD_RESET_REQUEST_DATA), (void**) &pPasswordResetRequest);
    BAIL_ON_ERROR(e);

    if (currentPassword != NULL)
    {
        e = RestStringDataNew(&(pPasswordResetRequest->currentPassword), currentPassword);
        BAIL_ON_ERROR(e);
    }

    if (newPassword != NULL)
    {
        e = RestStringDataNew(&(pPasswordResetRequest->newPassword), newPassword);
        BAIL_ON_ERROR(e);
    }

    *ppPasswordResetRequest = pPasswordResetRequest;

    error:

    if (e != SSOERROR_NONE)
    {
        IdmPasswordResetRequestDataDelete(pPasswordResetRequest);
    }

    return e;
}

void
IdmPasswordResetRequestDataDelete(
    IDM_PASSWORD_RESET_REQUEST_DATA* pPasswordResetRequest)
{
    if (pPasswordResetRequest != NULL)
    {
        RestStringDataDelete(pPasswordResetRequest->currentPassword);
        RestStringDataDelete(pPasswordResetRequest->newPassword);
        SSOMemoryFree(pPasswordResetRequest, sizeof(IDM_PASSWORD_RESET_REQUEST_DATA));
    }
}

SSOERROR
IdmPasswordResetRequestDataToJson(
    const IDM_PASSWORD_RESET_REQUEST_DATA* pPasswordResetRequest,
    PSSO_JSON pJson)
{
    SSOERROR e = SSOERROR_NONE;

    if (pJson == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    if (pPasswordResetRequest != NULL)
    {
        e = RestDataToJson(
            pPasswordResetRequest->currentPassword,
            REST_JSON_OBJECT_TYPE_STRING,
            NULL,
            "currentPassword",
            pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(
            pPasswordResetRequest->newPassword,
            REST_JSON_OBJECT_TYPE_STRING,
            NULL,
            "newPassword",
            pJson);
        BAIL_ON_ERROR(e);
    }

    error:

    return e;
}

SSOERROR
IdmJsonToPasswordResetRequestData(
    PCSSO_JSON pJson,
    IDM_PASSWORD_RESET_REQUEST_DATA** ppPasswordResetRequest)
{
    SSOERROR e = SSOERROR_NONE;
    IDM_PASSWORD_RESET_REQUEST_DATA* pPasswordResetRequest = NULL;

    if (pJson == NULL || ppPasswordResetRequest == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = SSOMemoryAllocate(sizeof(IDM_PASSWORD_RESET_REQUEST_DATA), (void**) &pPasswordResetRequest);
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_STRING,
        NULL,
        "currentPassword",
        (void**) &(pPasswordResetRequest->currentPassword));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_STRING,
        NULL,
        "newPassword",
        (void**) &(pPasswordResetRequest->newPassword));
    BAIL_ON_ERROR(e);

    *ppPasswordResetRequest = pPasswordResetRequest;

    error:

    if (e != SSOERROR_NONE)
    {
        IdmPasswordResetRequestDataDelete(pPasswordResetRequest);
    }

    return e;
}
