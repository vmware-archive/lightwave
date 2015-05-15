VOID
VmKdcFreePaData(
    PVMKDC_PADATA pPaData);

VOID
VmKdcFreeMethodData(
    PVMKDC_METHOD_DATA pMethodData);

DWORD
VmKdcAllocateMethodData(
    DWORD length,
    PVMKDC_METHOD_DATA *ppRetMethodData);

DWORD
VmKdcEncodeMethodData(
    PVMKDC_METHOD_DATA pMethodData,
    PVMKDC_DATA *ppRetData);

DWORD
VmKdcMakePaData(
    VMKDC_PADATA_TYPE type,
    DWORD length,
    PUCHAR contents,
    PVMKDC_PADATA *ppRetPaData);

DWORD
VmKdcFindPaData(
    VMKDC_PADATA_TYPE type,
    PVMKDC_METHOD_DATA pMethodData,
    PVMKDC_PADATA *ppRetPaData);

VOID
VmKdcPrintPaData(
    PVMKDC_PADATA pPaData);

VOID
VmKdcPrintMethodData(
    PVMKDC_METHOD_DATA pMethodData);
