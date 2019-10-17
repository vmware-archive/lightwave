/*
 * Copyright © 2019 VMware, Inc.  All Rights Reserved.
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

typedef enum
{
    VM_YAML_PARSE_STAT_UNKNOWN,
    VM_YAML_PARSE_STAT_START_MAPPING,
    VM_YAML_PARSE_STAT_END_MAPPING,
    VM_YAML_PARSE_STAT_SCALAR
} VM_YAML_PARSE_STAT_TYPE;

typedef enum
{
    VM_KV_STAT_UNKNOWN,
    VM_KV_STAT_ADD_SUBKEY,
    VM_KV_STAT_REMOVE_SUBKEY,
    VM_KV_STAT_SCALAR
} VM_KV_STAT_TYPE;

typedef struct _VM_WRITE_KV
{
    FILE*           fh;
    PVM_STRING_LIST pSubKeyList;
    PSTR            pszBuf;
    size_t          iBufLen;
    size_t          iBufSize;
    BOOLEAN         bPrefixNL;
    PSTR            pszValueQuoteBuf;
    size_t          iQuoteBufSize;
} VM_WRITE_KV, *PVM_WRITE_KV;

typedef struct _VM_CONSTRUCT_KV
{
    VM_KV_STAT_TYPE         kvStat;

    CHAR    pszSubKey[VM_SIZE_1024];
    size_t  iSubKeySize;

    BOOLEAN bHasKey;
    PSTR    pszValue;
} VM_CONSTRUCT_KV, *PVM_CONSTRUCT_KV;
