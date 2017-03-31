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



/* threading.c */

DWORD
VmDirInitializeMutexContent(
    PVMDIR_MUTEX pMutex
);

VOID
VmDirFreeMutexContent(
    PVMDIR_MUTEX pMutex
);

#if !defined(_WIN32) || defined(HAVE_PTHREADS_WIN32)
DWORD
VmDirInitializeConditionContent(
    PVMDIR_COND pCondition
);

VOID
VmDirFreeConditionContent(
    PVMDIR_COND pCondition
);

DWORD
VmDirInitializeSynchCounterContent(
    PVMDIR_SYNCHRONIZE_COUNTER     pSyncCounter
    );

#else
// All of this else section can eventually go away.

DWORD
VmDirInitializeSynchCounterContent(
    PVMDIR_SYNCHRONIZE_COUNTER     pSyncCounter
    );
// Win2008

DWORD
VmDirInitializeConditionContent2008(
    PVMDIR_COND_2008 pCondition
);

VOID
VmDirFreeConditionContent2008(
    PVMDIR_COND_2008 pCondition
);

DWORD
VmDirInitializeSynchCounterContent2008(
    PVMDIR_SYNCHRONIZE_COUNTER     pSyncCounter
    );

// Win2003


DWORD
VmDirInitializeConditionContent2003(
    PVMDIR_COND_2003 pCondition
);

VOID
VmDirFreeConditionContent2003(
    PVMDIR_COND_2003 pCondition
);

#endif

//IPC
DWORD
VmDirOpenServerConnectionImpl(
	PVM_DIR_CONNECTION *ppConection
	);

VOID
VmDirCloseServerConnectionImpl(
	PVM_DIR_CONNECTION pConnection
	);

VOID
VmDirShutdownServerConnectionImpl(
	PVM_DIR_CONNECTION pConnection
	);


DWORD
VmDirOpenClientConnectionImpl(
	PVM_DIR_CONNECTION *ppConnection
	);

VOID
VmDirCloseClientConnectionImpl(
	PVM_DIR_CONNECTION pConnection
	);

DWORD
VmDirAcceptConnectionImpl(
	PVM_DIR_CONNECTION pConnection,
	PVM_DIR_CONNECTION *ppConnection
	);

DWORD
VmDirReadDataImpl(
	PVM_DIR_CONNECTION pConnection,
	PBYTE *ppResponse,
	PDWORD pdwResponseSize
	);

DWORD
VmDirWriteDataImpl(
	PVM_DIR_CONNECTION pConnection,
	PBYTE pRequest,
	DWORD dwResponseSize
	);

VOID
VmDirFreeConnectionImpl(
        PVM_DIR_CONNECTION pConnection
        );

DWORD
VmDirInitializeSecurityContextImpl(
        PVM_DIR_CONNECTION pConnection,
        PVM_DIR_SECURITY_CONTEXT *ppSecurityContext
    );
VOID
VmDirFreeSecurityContextImpl(
    PVM_DIR_SECURITY_CONTEXT pSecurityContext
    );

DWORD
VmDirEncodeSecurityContextImpl (
    PVM_DIR_SECURITY_CONTEXT pSecurityContext,
    PBYTE pByteBuffer,
    DWORD dwAvailableBuffSize,
    PDWORD pdwBuffUsed
    );

DWORD
VmDirDecodeSecurityContextImpl (
    PBYTE pByteSecurityContext,
    DWORD dwBufSize,
    PVM_DIR_SECURITY_CONTEXT *ppSecurityContext
    );

DWORD
VmDirGetSecurityContextSizeImpl (
    PVM_DIR_SECURITY_CONTEXT pSecurityContext,
    PDWORD pdwSize
    );

BOOL
VmDirIsRootSecurityContextImpl (
    PVM_DIR_SECURITY_CONTEXT pSecurityContext
    );

BOOL
VmDirEqualsSecurityContextImpl (
    PVM_DIR_SECURITY_CONTEXT pSecurityContext1,
    PVM_DIR_SECURITY_CONTEXT pSecurityContext12
    );

DWORD
VmDirAllocateContextFromNameImpl (
    PCWSTR pszAccountName,
    PVM_DIR_SECURITY_CONTEXT *ppSecurityContext
    );

DWORD
VmDirAllocateNameFromContextImpl (
    PVM_DIR_SECURITY_CONTEXT pSecurityContext,
    PWSTR *ppszAccountName
    );

DWORD
VmDirCopySecurityContextImpl (
    PVM_DIR_SECURITY_CONTEXT pSecurityContextSrc,
    PVM_DIR_SECURITY_CONTEXT *ppSecurityContextDst
    );


DWORD
VmDirCreateRootSecurityContextImpl (
    PVM_DIR_SECURITY_CONTEXT *ppSecurityContext
    );

DWORD
VmDirCreateWellKnownContextImpl (
    VM_DIR_CONTEXT_TYPE contextType,
    PVM_DIR_SECURITY_CONTEXT *ppSecurityContext
    );

BOOL
VmDirContextBelongsToGroupImpl (
    PVM_DIR_SECURITY_CONTEXT pSecurityContext,
    PVM_DIR_SECURITY_CONTEXT pSecurityContextGroup
    );

DWORD
VmDirGenRandomImpl(
    PDWORD pdwRandomNumber
    );

