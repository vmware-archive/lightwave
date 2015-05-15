#ifndef _PARSEKT_H
#define _PARSEKT_H

typedef struct _VMKDC_KEYTAB_HANDLE
{
    int ktType; // 1=file, 2=memory,...
    FILE *ktfp;
    int ktOffset; // offset from start of container where kt data begins
    int ktMode; //1 = "r", 2 = "rw", 3="a"
} VMKDC_KEYTAB_HANDLE, *PVMKDC_KEYTAB_HANDLE;

typedef struct _VMKDC_MIT_KEYTAB_FILE
{
    int entrySize;
    int princType;
    char *realm;
    char **nameComponents;
    int nameComponentsLen;
    int timeStamp;
#if 1
    int kvno;
    int keyType;
#endif
    PVMKDC_KEY key;
} VMKDC_MIT_KEYTAB_FILE, *PVMKDC_MIT_KEYTAB_FILE;


DWORD
VmKdcParseKeyTabOpen(
    PSTR ktName,
    PSTR ktOpenMode, // "r", "rw"
    PVMKDC_KEYTAB_HANDLE *ppKeyTab);

VOID
VmKdcParseKeyTabClose(
    PVMKDC_KEYTAB_HANDLE pKeyTab);

DWORD
VmKdcParseKeyTabRead(
    PVMKDC_KEYTAB_HANDLE pKt,
    PVMKDC_MIT_KEYTAB_FILE *ppRetData);

DWORD
VmKdcParseKeyTabWrite(
    PVMKDC_KEYTAB_HANDLE pKt,
    PVMKDC_MIT_KEYTAB_FILE pKtEntry);

VOID
VmKdcParseKeyTabFreeEntry(
    PVMKDC_MIT_KEYTAB_FILE pKtEntry);
    
#endif
