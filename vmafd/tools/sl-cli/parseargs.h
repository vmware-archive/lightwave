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



DWORD
VmAfdParseBaseArgs(
        int   argc,
        char* argv[],
        PSTR* ppszNetworkAddress,
        PSTR* ppszDomain,
        PSTR* ppszUserName,
        PSTR* ppszPassword
        );

DWORD
VmAfdParseOperationArgs(
        int      argc,
        char*    argv[],
        PBOOLEAN pbEnable,
        PBOOLEAN pbIsEnabled,
        PBOOLEAN pbDisable,
        PBOOLEAN pbSetSize,
        PBOOLEAN pbGetSize,
        PBOOLEAN pbRetrieve,
        PBOOLEAN pbFlush,
        PBOOLEAN pbAggregate
        );

DWORD
VmAfdParseSetSizeArgs(
        int     argc,
        char*   argv[],
        PDWORD  pdwSize
        );

DWORD
VmAfdParseAggregateArgs(
        int      argc,
        char*    argv[],
        PBOOLEAN pbLoginDN,
        PBOOLEAN pbIP,
        PBOOLEAN pbPort,
        PBOOLEAN pbOperation,
        PBOOLEAN pbString,
        PBOOLEAN pbErrorCode,
        PBOOLEAN pbTime
        );

DWORD
VmAfdValidateBaseArgs(
        PSTR pszNetworkAddress,
        PSTR pszDomain,
        PSTR pszUserName,
        PSTR pszPassword
        );

DWORD
VmAfdValidateOperationArgs(
        BOOLEAN bEnable,
        BOOLEAN bIsEnabled,
        BOOLEAN bDisable,
        BOOLEAN bSetSize,
        BOOLEAN bGetSize,
        BOOLEAN bRetrieve,
        BOOLEAN bFlush,
        BOOLEAN bAggregate
        );

DWORD
VmAfdValidateSetSizeArgs(
        DWORD dwSize
        );

VOID
ShowUsage(
        VOID
        );
