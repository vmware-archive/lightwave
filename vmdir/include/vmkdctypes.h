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



/*
 * Module Name: VMKDC
 *
 * Filename: vmkdctypes.h
 *
 * Abstract:
 *
 * Common types definition
 *
 */

#ifndef __VMKDCTYPES_H__
#define __VMKDCTYPES_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _MES_header
{
    UINT8 Version;
    UINT8 Endianness;
    UINT16 CommonHeaderLength;
    UINT32 Filler1;
    UINT32 ObjectBufferLength;
    UINT32 Filler2;
    UINT32 Referent;
} MES_header, *PMES_header;

#ifdef __cplusplus
}
#endif

#endif /* __VMKDCTYPES_H__ */
