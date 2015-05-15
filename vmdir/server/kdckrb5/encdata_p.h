VOID
VmKdcFreeEncData(
    PVMKDC_ENCDATA pEncData);

DWORD
VmKdcMakeEncData(
    VMKDC_ENCTYPE type,
    DWORD kvno,
    PUCHAR contents,
    DWORD length,
    PVMKDC_ENCDATA *ppRetEncData);

DWORD
VmKdcEncryptEncData(
    PVMKDC_CONTEXT pContext,
    PVMKDC_KEY pKey,
    VMKDC_KEY_USAGE keyUsage,
    PVMKDC_DATA pInData,
    PVMKDC_ENCDATA *ppRetEncData);

DWORD
VmKdcDecryptEncData(
    PVMKDC_CONTEXT pContext,
    PVMKDC_KEY pKey,
    VMKDC_KEY_USAGE keyUsage,
    PVMKDC_ENCDATA pEncData,
    PVMKDC_DATA *ppRetData);

DWORD
VmKdcDecodeEncData(
    PVMKDC_DATA pData,
    PVMKDC_ENCDATA *ppRetEncData);

DWORD
VmKdcCopyEncData(
    PVMKDC_ENCDATA pEncData,
    PVMKDC_ENCDATA *ppRetEncData);

VOID
VmKdcPrintEncData(
    PVMKDC_ENCDATA pEncData);
