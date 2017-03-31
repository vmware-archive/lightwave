/*
 * Copyright (C) 2014 VMware, Inc. All rights reserved.
 *
 * Module   : authdb.h
 *
 * Abstract :
 *
 */
#ifndef _AUTHDB_H__
#define _AUTHDB_H__

#ifdef __cplusplus
extern "C" {
#endif


DWORD
VecsDbSetSecurityDescriptor (
            DWORD dwStoreID,
            PVMAFD_SECURITY_DESCRIPTOR pSecurityDescriptor
            );

DWORD
VecsDbGetSecurityDescriptor (
            DWORD dwStoreID,
            PVMAFD_SECURITY_DESCRIPTOR *ppSecurityDescriptor
            );

DWORD
VecsDbEnumFilteredStores (
            PBYTE pSecurityContextBlob,
            DWORD dwSizeOfContext,
            PWSTR **ppwszStoreNames,
            PDWORD pdwCount
            );

#ifdef __cplusplus
}
#endif

#endif
