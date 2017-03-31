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
VmDirParseBaseArgs(
        int   argc,
        char* argv[],
        PSTR* ppszNetworkAddress,
        PSTR* ppszDomain,
        PSTR* ppszUserName,
        PSTR* ppszPassword
        );

DWORD
VmDirParseOperationArgs(
        int      argc,
        char*    argv[],
        PBOOLEAN pbNodeData,
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
VmDirParseSetSizeArgs(
        int     argc,
        char*   argv[],
        PDWORD  pdwSize
        );

DWORD
VmDirParseAggregateArgs(
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
VmDirValidateBaseArgs(
        PSTR pszNetworkAddress,
        PSTR pszDomain,
        PSTR pszUserName,
        PSTR pszPassword
        );

DWORD
VmDirValidateOperationArgs(
        BOOLEAN bNodeData,
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
VmDirValidateSetSizeArgs(
        DWORD dwSize
        );

VOID
ShowUsage(
        VOID
        );
