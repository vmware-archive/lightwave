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



#pragma once

DWORD
VmAfdOpenServerConnectionImpl(
	PVM_AFD_CONNECTION *ppConection
	);

VOID
VmAfdCloseServerConnectionImpl(
	PVM_AFD_CONNECTION pConnection
	);

DWORD
VmAfdOpenClientConnectionImpl(
	PVM_AFD_CONNECTION *ppConnection
	);

VOID
VmAfdCloseClientConnectionImpl(
	PVM_AFD_CONNECTION pConnection
	);

DWORD
VmAfdAcceptConnectionImpl(
	PVM_AFD_CONNECTION pConnection,
	PVM_AFD_CONNECTION *ppConnection
	);

DWORD
VmAfdReadDataImpl(
	PVM_AFD_CONNECTION pConnection,
	PBYTE *ppResponse,
	PDWORD pdwResponseSize
	);

DWORD
VmAfdWriteDataImpl(
	PVM_AFD_CONNECTION pConnection,
	PBYTE pRequest,
	DWORD dwResponseSize
	);

VOID
VmAfdFreeConnectionImpl(
        PVM_AFD_CONNECTION pConnection
        );

DWORD
VmAfdInitializeConnectionContextImpl(
    PVM_AFD_CONNECTION pConnection,
    PVM_AFD_CONNECTION_CONTEXT *ppConnectionContext
    );

VOID
VmAfdFreeConnectionContextImpl(
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext
    );

DWORD
VmAfdInitializeSecurityContextImpl(
        PVM_AFD_CONNECTION pConnection,
        PVM_AFD_SECURITY_CONTEXT *ppSecurityContext
    );
VOID
VmAfdFreeSecurityContextImpl(
    PVM_AFD_SECURITY_CONTEXT pSecurityContext
    );

DWORD
VmAfdEncodeSecurityContextImpl (
    PVM_AFD_SECURITY_CONTEXT pSecurityContext,
    PBYTE pByteBuffer,
    DWORD dwAvailableBuffSize,
    PDWORD pdwBuffUsed
    );

DWORD
VmAfdDecodeSecurityContextImpl (
    PBYTE pByteSecurityContext,
    DWORD dwBufSize,
    PVM_AFD_SECURITY_CONTEXT *ppSecurityContext
    );

DWORD
VmAfdGetSecurityContextSizeImpl (
    PVM_AFD_SECURITY_CONTEXT pSecurityContext,
    PDWORD pdwSize
    );

BOOL
VmAfdIsRootSecurityContextImpl (
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext
    );

BOOL
VmAfdEqualsSecurityContextImpl (
    PVM_AFD_SECURITY_CONTEXT pSecurityContext1,
    PVM_AFD_SECURITY_CONTEXT pSecurityContext12
    );

DWORD
VmAfdAllocateContextFromNameImpl (
    PCWSTR pszAccountName,
    PVM_AFD_SECURITY_CONTEXT *ppSecurityContext
    );

DWORD
VmAfdAllocateNameFromContextImpl (
    PVM_AFD_SECURITY_CONTEXT pSecurityContext,
    PWSTR *ppszAccountName
    );

DWORD
VmAfdCopySecurityContextImpl (
    PVM_AFD_SECURITY_CONTEXT pSecurityContextSrc,
    PVM_AFD_SECURITY_CONTEXT *ppSecurityContextDst
    );


DWORD
VmAfdCreateAnonymousConnectionContextImpl (
    PVM_AFD_CONNECTION_CONTEXT *ppSecurityContext
    );

DWORD
VmAfdCreateWellKnownContextImpl (
    VM_AFD_CONTEXT_TYPE contextType,
    PVM_AFD_SECURITY_CONTEXT *ppSecurityContext
    );

BOOL
VmAfdContextBelongsToGroupImpl (
    PVM_AFD_SECURITY_CONTEXT pSecurityContext,
    PVM_AFD_SECURITY_CONTEXT pSecurityContextGroup
    );

DWORD
VmAfdGenRandomImpl(
    PDWORD pdwRandomNumber
    );
