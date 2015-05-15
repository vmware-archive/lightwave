#include "includes.h"

VOID
VmKdcFreeChecksum(
    PVMKDC_CHECKSUM pChecksum)
{
    if (pChecksum)
    {
        VMKDC_SAFE_FREE_DATA(pChecksum->data);
        VMKDC_SAFE_FREE_MEMORY(pChecksum);
    }
}

DWORD
VmKdcMakeChecksum(
    VMKDC_CKSUMTYPE type,
    PUCHAR contents,
    DWORD length,
    PVMKDC_CHECKSUM *ppRetChecksum)
{
    DWORD dwError = 0;
    PVMKDC_CHECKSUM pChecksum = NULL;

    dwError = VmKdcAllocateMemory(sizeof(VMKDC_CHECKSUM), (PVOID*)&pChecksum);
    BAIL_ON_VMKDC_ERROR(dwError);

    dwError = VmKdcAllocateData(contents, length, &pChecksum->data);
    BAIL_ON_VMKDC_ERROR(dwError);

    pChecksum->type = type;

    /* TBD - calculate checksum */

    *ppRetChecksum = pChecksum;

error:
    if (dwError)
    {
        VMKDC_SAFE_FREE_CHECKSUM(pChecksum);
    }

    return dwError;
}
