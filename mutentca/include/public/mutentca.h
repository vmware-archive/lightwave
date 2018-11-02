/*
 * Copyright © 2018 VMware, Inc.  All Rights Reserved.
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

#ifndef __MUTENTCA_H__
#define __MUTENTCA_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifndef VMW_PSTR_DEFINED
#define VMW_PSTR_DEFINED 1
typedef char* PSTR;
#endif /* VMW_PSTR_DEFINED */

#ifndef VMW_PCSTR_DEFINED
#define VMW_PCSTR_DEFINED 1
typedef const char* PCSTR;
#endif /* VMW_PCSTR_DEFINED */

#ifndef VMW_VOID_DEFINED
#define VMW_VOID_DEFINED 1
typedef void VOID, *PVOID;
#endif /* VMW_VOID_DEFINED */

#ifndef VMW_DWORD_DEFINED
#define VMW_DWORD_DEFINED 1
typedef uint32_t DWORD, *PDWORD;
#endif /* VMW_DWORD_DEFINED */

#ifndef LWCA_UINT8_DEFINED
#define LWCA_UINT8_DEFINED 1
typedef uint8_t  UINT8;
#endif /* LWCA_UINT8_DEFINED */

#ifndef VMW_PBYTE_DEFINED
#define VMW_PBYTE_DEFINED 1
typedef unsigned char* PBYTE;
#endif /* VMW_PBYTE_DEFINED */

#ifndef LWCA_BOOLEAN_DEFINED
#define LWCA_BOOLEAN_DEFINED 1
typedef UINT8 BOOLEAN, *PBOOLEAN;
#endif /* LWCA_BOOLEAN_DEFINED */

#ifndef LWCA_CERTIFICATE_DEFINED
#define LWCA_CERTIFICATE_DEFINED 1
typedef PSTR PLWCA_CERTIFICATE;
#endif /* LWCA_CERTIFICATE_DEFINED */

/*
 * String array and number of elements
 */
typedef struct _LWCA_STRING_ARRAY
{
    PSTR    *ppData;
    DWORD   dwCount;
} LWCA_STRING_ARRAY, *PLWCA_STRING_ARRAY;

/*
 * Request context containing information about API requestor
 */
typedef struct _LWCA_REQ_CONTEXT
{
    PSTR                    pszBindUPN;
    PSTR                    pszBindUPNTenant;
    PLWCA_STRING_ARRAY      pBindUPNGroups;
    PSTR                    pszCAId;
    PSTR                    pszRequestId;
} LWCA_REQ_CONTEXT, *PLWCA_REQ_CONTEXT;

/*
 * Defines Certificate array
 */
typedef struct _LWCA_CERTIFICATE_ARRAY
{
    PLWCA_CERTIFICATE *ppCertificates;
    DWORD             dwCount;
} LWCA_CERTIFICATE_ARRAY, *PLWCA_CERTIFICATE_ARRAY;

/*
 * Defines key
 */
typedef struct _LWCA_KEY
{
    PBYTE pData;
    DWORD dwLength;
} LWCA_KEY, *PLWCA_KEY;

/*
 * Represents CA status
 */
typedef enum _LWCA_CA_STATUS
{
    LWCA_CA_STATUS_INACTIVE = 0,
    LWCA_CA_STATUS_ACTIVE
} LWCA_CA_STATUS,*PLWCA_CA_STATUS;

/*
 * Represents Certificate status
 */
typedef enum _LWCA_CERT_STATUS
{
    LWCA_CERT_STATUS_INACTIVE = 0,
    LWCA_CERT_STATUS_ACTIVE
} LWCA_CERT_STATUS,*PLWCA_CERT_STATUS;

#ifdef __cplusplus
}
#endif

#endif // __MUTENTCA_H__
