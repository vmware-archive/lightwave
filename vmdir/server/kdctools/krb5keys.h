DWORD
VmKdcGenerateMasterKey(
    PBYTE *ppMasterKey,
    PDWORD pMasterKeyLen);


DWORD
VmKdcStringToKeys(
    PSTR upnName,
    PSTR password,
    PBYTE *ppMasterKey,
    PDWORD pMasterKeyLen);
