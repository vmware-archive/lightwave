/*
 * Copyright (c) VMware Inc.  All rights Reserved.
 */

/*
 * Module Name: VMware Registry
 *
 * Filename: regdb.h
 *
 * Abstract:
 *
 * Registry Database
 *
 */

#ifndef _REGDB_H__
#define _REGDB_H__

#ifdef __cplusplus
extern "C" {
#endif

DWORD
VmAfdRegGetString(
    PCSTR    pszSubKey,      /* IN     */
    PCSTR    pszValueName,   /* IN     */
    PWSTR*   ppwszValue      /*    OUT */
  );

DWORD
VmAfdRegSetString(
    PCSTR    pszSubKeyParam, /* IN     */
    PCSTR    pszValueName,   /* IN     */
    PCWSTR   pwszValue       /* IN     */
  );

DWORD
VmAfdRegGetInteger(
    PCSTR    pszValueName,   /* IN     */
    PDWORD   pdwValue        /*    OUT */
  );

DWORD
VmAfdRegSetInteger(
    PCSTR    pszValueName,   /* IN     */
    DWORD    dwValue         /* IN     */
  );

DWORD
VmAfdRegDeleteValue(
    PCSTR    pszSubKey,      /* IN     */
    PCSTR    pszValueName   /* IN     */
  );

#ifdef __cplusplus
}
#endif

#endif /* _REGDB_H__ */
