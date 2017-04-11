/*
 * Copyright (C) 2016 VMware, Inc. All rights reserved.
 *
 * Module   : ddnspackets.c
 *
 * Abstract :
 *
 */
#include "includes.h"

static
DWORD
VmDdnsConstructName(
    PSTR pInputString,
    PSTR* ppOutString,
    DWORD* pStringLength
    );

static
DWORD
VmDdnsMakeUpdateHeader(
    PVMAFD_MESSAGE_BUFFER pBuffer,
    DWORD hederId,
    DWORD upCount
    );

static
DWORD
VmDdnsMakeZone(
    PSTR pszZone,
    PVMAFD_MESSAGE_BUFFER pBuffer
    );

static
DWORD
VmDdnsMakeUpdateRR(
    WORD pszRecordType,
    PSTR pszName,
    DWORD labelLength,
    PVMAFD_MESSAGE_BUFFER pBuffer,
    VMDNS_IP4_ADDRESS* ipV4Address,
    VMDNS_IP6_ADDRESS* ipV6Address
    );

static
DWORD
VmDdnsMakeDeleteRR(
    WORD pszRecordType,
    PSTR pszName,
    DWORD labelLength,
    PVMAFD_MESSAGE_BUFFER pBuffer
    );

DWORD
VmDdnsUpdateMakePacket(
    PSTR pszZone,
    PSTR pszHostname,
    PSTR pszName,
    PSTR* ppDnsPacket,
    DWORD* ppacketSize,
    DWORD headerId,
    DWORD dwFlag
    )
{
    PSTR pLabelName = NULL;
    PSTR fqdn = NULL;
    DWORD dwError = 0;
    DWORD labelLength = 0;
    DWORD packetSize = 0;
    DWORD upCount = 2;
    VMDNS_IP4_ADDRESS* pV4Address = NULL;
    VMDNS_IP6_ADDRESS* pV6Address = NULL;
    PVMAFD_MESSAGE_BUFFER pDnsBuffer = NULL;
    PSTR pDnsPacket = NULL;

    if(!pszZone || !pszName || !pszHostname || !ppDnsPacket || !ppacketSize)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdAllocateStringPrintf(
                          &fqdn,
                          "%s.%s",
                          pszName,
                          pszZone
                          );
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmDdnsConstructName(
                        fqdn,
                        &pLabelName,
                        &labelLength
                        );
    BAIL_ON_VMAFD_ERROR(dwError);

    //Add updated records
    if(dwFlag == VMDDNS_UPDATE_PACKET)
    {
        dwError = VmDdnsGetSourceIp(
                              &pV4Address,
                              &pV6Address
                              );
        BAIL_ON_VMAFD_ERROR(dwError);
        upCount++;
    }

    dwError = VmAfdAllocateBufferStream(
                        0,
                        &pDnsBuffer
                        );
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmDdnsMakeUpdateHeader(
                          pDnsBuffer,
                          headerId,
                          upCount
                          );
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmDdnsMakeZone(
                    pszZone,
                    pDnsBuffer
                    );
    BAIL_ON_VMAFD_ERROR(dwError);

    //Delete all A records
    dwError = VmDdnsMakeDeleteRR(
                      VMDDNS_TYPE_A,
                      pLabelName,
                      labelLength,
                      pDnsBuffer
                      );
    BAIL_ON_VMAFD_ERROR(dwError);

    //Delete all AAAA records
    dwError = VmDdnsMakeDeleteRR(
                        VMDDNS_TYPE_AAAA,
                        pLabelName,
                        labelLength,
                        pDnsBuffer
                        );
    BAIL_ON_VMAFD_ERROR(dwError);

    if(dwFlag == VMDDNS_UPDATE_PACKET)
    {
        //Add A records
        if(pV4Address)
        {
            dwError = VmDdnsMakeUpdateRR(
                              VMDDNS_TYPE_A,
                              pLabelName,
                              labelLength,
                              pDnsBuffer,
                              pV4Address,
                              NULL
                              );
            BAIL_ON_VMAFD_ERROR(dwError);
        }
        else
        {
            dwError = VmDdnsMakeUpdateRR(
                              VMDDNS_TYPE_AAAA,
                              pLabelName,
                              labelLength,
                              pDnsBuffer,
                              NULL,
                              pV6Address
                              );
            BAIL_ON_VMAFD_ERROR(dwError);
        }
    }

    dwError = VmAfdCopyBufferFromBufferStream(
                    pDnsBuffer,
                    NULL,
                    &packetSize
                    );
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateMemory(
                    packetSize,
                    (PVOID)&pDnsPacket
                    );
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdCopyBufferFromBufferStream(
                    pDnsBuffer,
                    pDnsPacket,
                    &packetSize
                    );
    BAIL_ON_VMAFD_ERROR(dwError);

    *ppDnsPacket = pDnsPacket;
    *ppacketSize = packetSize;

cleanup:

    VMAFD_SAFE_FREE_MEMORY(pV4Address);
    VMAFD_SAFE_FREE_MEMORY(pV6Address);
    VmAfdFreeBufferStream(pDnsBuffer);
    VMAFD_SAFE_FREE_STRINGA(pLabelName);
    VMAFD_SAFE_FREE_STRINGA(fqdn);
    return dwError;

error:
    if(pDnsPacket)
    {
        VMAFD_SAFE_FREE_STRINGA(pDnsPacket);
    }
    if(ppDnsPacket)
    {
        *ppDnsPacket = NULL;
    }
    goto cleanup;
}

static
DWORD
VmDdnsConstructName(
    PSTR pInputString,
    PSTR* ppOutString,
    DWORD* pStringLength
    )
{
    PSTR pszTempString = NULL;
    PSTR pszTempStringTwo = NULL;
    DWORD labelLength = 0;
    DWORD inputPos = 0;
    DWORD outLength = 0;
    DWORD dwError = 0;

    if(!pInputString || !ppOutString || !pStringLength)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdAllocateMemory(
                  512,
                  (PVOID *)&pszTempString
                  );
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateMemory(
                  64,
                  (PVOID *)&pszTempStringTwo
                  );
    BAIL_ON_VMAFD_ERROR(dwError);

    do
    {
        while(pInputString[inputPos] != '.' && pInputString[inputPos])
        {
            pszTempStringTwo[labelLength] = pInputString[inputPos];
            labelLength++;
            inputPos++;
        }
        //copy the length
        pszTempString[outLength] = (BYTE)labelLength;
        outLength++;
        if(pInputString[inputPos] == '.')
        {
            inputPos++;
        }

        dwError = VmAfdCopyMemory(
                        (pszTempString + outLength),
                        outLength,
                        pszTempStringTwo,
                        (size_t)labelLength
                        );
        BAIL_ON_VMAFD_ERROR(dwError);

        outLength += labelLength;
        labelLength = 0;
    } while(pInputString[inputPos]);

    *ppOutString = pszTempString;
    *pStringLength = outLength;

cleanup:

    VMAFD_SAFE_FREE_STRINGA(pszTempStringTwo);
    return dwError;

error:

    if(ppOutString)
    {
        VMAFD_SAFE_FREE_STRINGA(*ppOutString);
        ppOutString = NULL;
    }
    goto cleanup;
}

static
DWORD
VmDdnsMakeUpdateHeader(
    PVMAFD_MESSAGE_BUFFER pBuffer,
    DWORD headerId,
    DWORD upCount
    )
{
    DWORD dwError = 0;

    if(!pBuffer)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdWriteUINT16ToBuffer(
                        headerId,
                        pBuffer
                        );
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdWriteUINT16ToBuffer(
                        VMDDNS_UPDATE_CODES_WORD,
                        pBuffer
                        );
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdWriteUINT16ToBuffer(
                        0x0001,
                        pBuffer
                        );
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdWriteUINT16ToBuffer(
                        0,
                        pBuffer
                        );
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdWriteUINT16ToBuffer(
                        upCount,
                        pBuffer
                        );
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdWriteUINT16ToBuffer(
                        0,
                        pBuffer
                        );
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    return dwError;

error:

    goto cleanup;
}

static
DWORD
VmDdnsMakeZone(
    PSTR pszZone,
    PVMAFD_MESSAGE_BUFFER pBuffer
    )
{
    DWORD dwError = 0;
    DWORD stringLength = 0;
    PSTR psLabelZone = NULL;

    if( !pszZone  || !pBuffer)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmDdnsConstructName(
                      pszZone,
                      &psLabelZone,
                      &stringLength
                      );
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdWriteStringToBuffer(
                        psLabelZone,
                        stringLength,
                        pBuffer
                        );
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdWriteUINT16ToBuffer(
                        VMDDNS_TYPE_SOA,
                        pBuffer
                        );
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdWriteUINT16ToBuffer(
                        VMDDNS_CLASS_INTERNET,
                        pBuffer
                        );
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    VMAFD_SAFE_FREE_STRINGA(psLabelZone);
    return dwError;

error:

    goto cleanup;
}

static
DWORD
VmDdnsMakeUpdateRR(
    WORD pszRecordType,
    PSTR pszName,
    DWORD labelLength,
    PVMAFD_MESSAGE_BUFFER pBuffer,
    VMDNS_IP4_ADDRESS* ipV4Address,
    VMDNS_IP6_ADDRESS* ipV6Address
    )
{
    DWORD dwError = 0;

    if( !pszName || !pszRecordType || !(ipV6Address || ipV4Address)  || !pBuffer)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdWriteStringToBuffer(
                        pszName,
                        labelLength,
                        pBuffer
                        );
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdWriteUINT16ToBuffer(
                        pszRecordType,
                        pBuffer
                        );
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdWriteUINT16ToBuffer(
                        VMDDNS_CLASS_INTERNET,
                        pBuffer
                        );
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdWriteUINT32ToBuffer(
                        VMDDNS_DEFAULT_TTL,
                        pBuffer
                        );
    BAIL_ON_VMAFD_ERROR(dwError);

    if(pszRecordType == VMDDNS_TYPE_A)
    {
        dwError = VmAfdWriteUINT16ToBuffer(
                            VMDNS_IP4_ADDRESS_SIZE,
                            pBuffer
                            );
        BAIL_ON_VMAFD_ERROR(dwError);

        dwError = VmAfdWriteUINT32ToBuffer(
                            *ipV4Address,
                            pBuffer
                            );
        BAIL_ON_VMAFD_ERROR(dwError);
    }
    else
    {
        dwError = VmAfdWriteUINT16ToBuffer(
                            VMDNS_IP6_ADDRESS_SIZE,
                            pBuffer
                            );
        BAIL_ON_VMAFD_ERROR(dwError);

        dwError = VmAfdWriteStringToBuffer(
                            (PSTR)ipV6Address,
                            16,
                            pBuffer
                            );
        BAIL_ON_VMAFD_ERROR(dwError);
    }

cleanup:

    return dwError;

error:

    goto cleanup;
}

static
DWORD
VmDdnsMakeDeleteRR(
    WORD pszRecordType,
    PSTR pszName,
    DWORD labelLength,
    PVMAFD_MESSAGE_BUFFER pBuffer
    )
{
    DWORD dwError = 0;

    if(!pszName || !pBuffer)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdWriteStringToBuffer(
                        pszName,
                        labelLength,
                        pBuffer
                        );
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdWriteUINT16ToBuffer(
                        pszRecordType,
                        pBuffer
                        );
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdWriteUINT16ToBuffer(
                        VMDDNS_CLASS_ANY,
                        pBuffer
                        );
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdWriteUINT32ToBuffer(
                        0,
                        pBuffer
                        );
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdWriteUINT16ToBuffer(
                        0,
                        pBuffer
                        );
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    return dwError;

error:

    goto cleanup;
}
