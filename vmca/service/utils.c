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



/*
 * Module Name: utils.c
 *
 * Filename: utils.c
 *
 * Abstract:
 *
 * Utility functions
 *
 */

#include "includes.h"

VMCA_DB_CERTIFICATE_STATUS
VMCAMapToDBStatus(CERTIFICATE_STATUS st)
{
	switch(st)
	{
	case CERTIFICATE_ACTIVE : return VMCA_DB_CERTIFICATE_STATUS_ACTIVE;
	case CERTIFICATE_REVOKED : return VMCA_DB_CERTIFICATE_STATUS_REVOKED;
	case CERTIFIFCATE_EXPIRED : return VMCA_DB_CERTIFICATE_STATUS_EXPIRED;
	case CERTIFICATE_ALL : return VMCA_DB_CERTIFICATE_STATUS_ALL;
	}
     return VMCA_DB_CERTIFICATE_STATUS_ALL;
}

DWORD
VMCAHeartbeatInit(
    PVMAFD_HB_HANDLE *ppHandle
    )
{
    DWORD dwError = 0;
    PVMAFD_HB_HANDLE pHandle = NULL;

    if (!ppHandle)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    dwError = VmAfdStartHeartbeatA(
                                VMCA_EVENT_SOURCE,
                                2014,
                                &pHandle
                                );
    BAIL_ON_VMCA_ERROR(dwError);

    *ppHandle = pHandle;

cleanup:

    return dwError;
error:

    if (ppHandle)
    {
        *ppHandle = NULL;
    }
    if (pHandle)
    {
        VmAfdStopHeartbeat(pHandle);
    }

    goto cleanup;
}

VOID
VMCAStopHeartbeat(
    PVMAFD_HB_HANDLE pHandle
    )
{
    if (pHandle)
    {
        VmAfdStopHeartbeat(pHandle);
    }
}

