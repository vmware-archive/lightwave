/*
 * Copyright (C) 2014 VMware, Inc. All rights reserved.
 *
 * Module   : prototypes.h
 *
 * Abstract :
 *
 */

// cli.c
DWORD
CdcCliEnableDefaultHA(
    PVMAFD_SERVER pServer
    );

DWORD
CdcCliEnableLegacyHA(
    PVMAFD_SERVER pServer
    );

DWORD
CdcCliGetStateofClientAffinity(
    PVMAFD_SERVER pServer
    );

DWORD
CdcCliGetDCName(
    PVMAFD_SERVER pServer
    );

DWORD
CdcCliPurgeDCCache(
    PVMAFD_SERVER pServer
    );

DWORD
CdcCliDcCacheList(
    PVMAFD_SERVER pServer
    );

DWORD
CdcCliDcCacheRefresh(
    PVMAFD_SERVER pServer
    );

DWORD
CdcCliGetDCStatus(
    PVMAFD_SERVER pServer,
    PCSTR          pszPSC
    );
