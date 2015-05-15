VOID
VmKdcFreeChecksum(
    PVMKDC_CHECKSUM pChecksum);

DWORD
VmKdcMakeChecksum(
    VMKDC_CKSUMTYPE type,
    PUCHAR contents,
    DWORD length,
    PVMKDC_CHECKSUM *ppRetChecksum);
