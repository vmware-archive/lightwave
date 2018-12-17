/*
 * Copyright © 2012-2018 VMware, Inc.  All Rights Reserved.
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

static
DWORD
VmDnsReadPropertyDataFromBuffer(
    PVMDNS_MESSAGE_BUFFER pVmDnsBuffer,
    VMDNS_PROPERTY_ID PropertyId,
    PVMDNS_PROPERTY_DATA pDnsPropertyData,
    UINT8 *puiPropertyDataLen
    )
{
    DWORD dwError = 0;
    UINT8 uiPropertyDataLen = 0;

    if (PropertyId != VMDNS_PROPERTY_ID_ZONE_ID)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    VmDnsReadCharFromBuffer(pVmDnsBuffer, (PCHAR)&pDnsPropertyData->ZoneId);
    BAIL_ON_VMDNS_ERROR(dwError);

    uiPropertyDataLen = 1;

    *puiPropertyDataLen = uiPropertyDataLen;

cleanup:

    return dwError;

error:

    goto cleanup;
}

static
DWORD
VmDnsWritePropertyToBuffer(
    PVMDNS_PROPERTY pDnsProperty,
    PVMDNS_MESSAGE_BUFFER pVmDnsBuffer
    )
{
    DWORD dwError = 0;

    if (!pVmDnsBuffer ||
        !pDnsProperty ||
        pDnsProperty->Id != VMDNS_PROPERTY_ID_ZONE_ID)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    /* DataLength (1 byte) */

    dwError = VmDnsWriteCharToBuffer(pDnsProperty->DataLength, pVmDnsBuffer);
    BAIL_ON_VMDNS_ERROR(dwError);

    /* NameLength (1 byte) */

    dwError = VmDnsWriteCharToBuffer(0x01, pVmDnsBuffer);
    BAIL_ON_VMDNS_ERROR(dwError);

    /* Flag (1 byte) */

    dwError = VmDnsWriteCharToBuffer(0x00, pVmDnsBuffer);
    BAIL_ON_VMDNS_ERROR(dwError);

    /* Version (1 byte) */

    dwError = VmDnsWriteCharToBuffer(0x01, pVmDnsBuffer);
    BAIL_ON_VMDNS_ERROR(dwError);

    /* Id (1 byte) */

    dwError = VmDnsWriteCharToBuffer(pDnsProperty->Id, pVmDnsBuffer);
    BAIL_ON_VMDNS_ERROR(dwError);

    /* Data */

    dwError = VmDnsWriteCharToBuffer(pDnsProperty->Data.ZoneId, pVmDnsBuffer);
    BAIL_ON_VMDNS_ERROR(dwError);

    /* Name (1 byte) */

    dwError = VmDnsWriteCharToBuffer(0x00, pVmDnsBuffer);
    BAIL_ON_VMDNS_ERROR(dwError);

cleanup:

    return dwError;

error:

    goto cleanup;
}

DWORD
VmDnsSerializeDnsProperty(
    PVMDNS_PROPERTY pDnsProperty,
    PBYTE* ppBytes,
    DWORD* pdwSize
    )
{
    DWORD dwError = 0;
    PVMDNS_MESSAGE_BUFFER pVmDnsBuffer = NULL;
    PBYTE pBytes = NULL;
    DWORD dwSize =  0;

    BAIL_ON_VMDNS_INVALID_POINTER(pDnsProperty, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(ppBytes, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(pdwSize, dwError);

    dwError = VmDnsAllocateBufferStream(
                        0,
                        &pVmDnsBuffer);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsWritePropertyToBuffer(
                        pDnsProperty,
                        pVmDnsBuffer);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsCopyBufferFromBufferStream(
                        pVmDnsBuffer,
                        NULL,
                        &dwSize);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsAllocateMemory(
                        dwSize,
                        (PVOID *)&pBytes);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsCopyBufferFromBufferStream(
                        pVmDnsBuffer,
                        pBytes,
                        &dwSize);
    BAIL_ON_VMDNS_ERROR(dwError);

    *ppBytes = pBytes;
    *pdwSize = dwSize;

cleanup:

    if (pVmDnsBuffer)
    {
        VmDnsFreeBufferStream(pVmDnsBuffer);
    }

    return dwError;

error:

    if (ppBytes)
    {
        *ppBytes = NULL;
    }
    if (pdwSize)
    {
        *pdwSize = 0;
    }
    VMDNS_SAFE_FREE_MEMORY(pBytes);

    goto cleanup;
}

static
DWORD
VmDnsReadPropertyFromBuffer(
    PVMDNS_MESSAGE_BUFFER pVmDnsBuffer,
    PVMDNS_PROPERTY *ppDnsProperty
    )
{
    DWORD dwError = 0;
    PVMDNS_PROPERTY pDnsProperty = NULL;
    UINT8 uiNameLength = 0;
    UINT8 uiFlag = 0;
    UINT8 uiVersion = 0;
    UINT8 uiName = 0;
    UINT8 uiDataLength = 0;

    BAIL_ON_VMDNS_INVALID_POINTER(pVmDnsBuffer, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(ppDnsProperty, dwError);

    dwError = VmDnsAllocateMemory(
                        sizeof(VMDNS_PROPERTY),
                        (PVOID *)&pDnsProperty);
    BAIL_ON_VMDNS_ERROR(dwError);

    /* DataLength (1 byte) */

    dwError = VmDnsReadCharFromBuffer(pVmDnsBuffer, (PCHAR)&uiDataLength);
    BAIL_ON_VMDNS_ERROR(dwError);

    /* NameLength (1 byte) */

    dwError = VmDnsReadCharFromBuffer(pVmDnsBuffer, (PCHAR)&uiNameLength);
    BAIL_ON_VMDNS_ERROR(dwError);

    if (uiNameLength != 0x01)
    {
        dwError = ERROR_INVALID_DATA;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    /* Flag (1 byte) */

    dwError = VmDnsReadCharFromBuffer(pVmDnsBuffer, (PCHAR)&uiFlag);
    BAIL_ON_VMDNS_ERROR(dwError);

    if (uiFlag != 0x00)
    {
        dwError = ERROR_INVALID_DATA;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    /* Version (1 byte) */

    dwError = VmDnsReadCharFromBuffer(pVmDnsBuffer, (PCHAR)&uiVersion);
    BAIL_ON_VMDNS_ERROR(dwError);

    if (uiVersion != 0x01)
    {
        dwError = ERROR_INVALID_DATA;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    /* Id (1 byte) */

    dwError = VmDnsReadCharFromBuffer(pVmDnsBuffer, (PCHAR)&pDnsProperty->Id);
    BAIL_ON_VMDNS_ERROR(dwError);

    /* Data */

    dwError = VmDnsReadPropertyDataFromBuffer(
                    pVmDnsBuffer,
                    pDnsProperty->Id,
                    &pDnsProperty->Data,
                    &pDnsProperty->DataLength);
    BAIL_ON_VMDNS_ERROR(dwError);

    /* Name (1 byte) */

    dwError = VmDnsReadCharFromBuffer(pVmDnsBuffer, (PCHAR)&uiName);
    BAIL_ON_VMDNS_ERROR(dwError);

    *ppDnsProperty = pDnsProperty;

cleanup:

    return dwError;

error:

    if (ppDnsProperty)
    {
        *ppDnsProperty = NULL;
    }
    VMDNS_FREE_PROPERTY(pDnsProperty);
    goto cleanup;
}

DWORD
VmDnsDeserializeDnsProperty(
    PBYTE pBytes,
    DWORD dwSize,
    PVMDNS_PROPERTY *ppDnsProperty
    )
{
    DWORD dwError = 0;
    PVMDNS_PROPERTY pDnsProperty = NULL;
    PVMDNS_MESSAGE_BUFFER pVmDnsBuffer = NULL;

    if (!pBytes || !dwSize || !ppDnsProperty)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsAllocateBufferStreamWithBuffer(
                    pBytes,
                    dwSize,
                    0,
                    FALSE,
                    &pVmDnsBuffer);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsReadPropertyFromBuffer(
                    pVmDnsBuffer,
                    &pDnsProperty);
    BAIL_ON_VMDNS_ERROR(dwError);

    *ppDnsProperty = pDnsProperty;

cleanup:

    VmDnsFreeBufferStream(pVmDnsBuffer);
    return dwError;

error:

    if (ppDnsProperty)
    {
        *ppDnsProperty = NULL;
    }
    VMDNS_FREE_PROPERTY(pDnsProperty);
    goto cleanup;
}

BOOL
VmDnsIsSupportedPropertyId(
    VMDNS_PROPERTY_ID dwPropertyId
    )
{
    return dwPropertyId == VMDNS_PROPERTY_ID_ZONE_ID;
}

DWORD
VmDnsCreateZoneIdProperty(
    VMDNS_ZONE_ID zoneId,
    PVMDNS_PROPERTY* ppProperty)
{
    DWORD dwError = 0;
    PVMDNS_PROPERTY pProperty = NULL;

    BAIL_ON_VMDNS_INVALID_POINTER(ppProperty, dwError);

    dwError = VmDnsAllocateMemory(
                        sizeof(VMDNS_PROPERTY),
                        (PVOID *)&pProperty);
    BAIL_ON_VMDNS_ERROR(dwError);

    pProperty->DataLength = sizeof(VMDNS_ZONE_ID);
    pProperty->Id = VMDNS_PROPERTY_ID_ZONE_ID;
    pProperty->Data.ZoneId = zoneId;

    *ppProperty = pProperty;

cleanup:

    return dwError;

error:

    VMDNS_FREE_PROPERTY(pProperty);
    goto cleanup;
}
