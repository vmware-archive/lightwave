/*
 * Copyright © 2012-2015 VMware, Inc.  All Rights Reserved.
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
