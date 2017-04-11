/*
 * Copyright © 2012-2016 VMware, Inc.  All Rights Reserved.
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
#include "includes.h"


VOID
_VmDirTestBreakIntoDebugger(
    VOID)
{
#ifdef _WIN32
    DbgBreakPoint();
#else
    raise(SIGTRAP);
#endif
}

VOID
_VmDirTestAssertionWorker(
    PVMDIR_TEST_STATE pState
    )
{
    if (pState->bBreakIntoDebugger)
    {
        _VmDirTestBreakIntoDebugger();
    }

    if (!pState->bKeepGoing)
    {
        (*pState->pfnCleanupCallback)(pState);
        exit(1);
    }
}

VOID
VmDirTestReportAssertionFailure(
    PCSTR pszExpression,
    PCSTR pszCustomMsg,
    PCSTR pszFile,
    PCSTR pszFunction,
    DWORD dwLineNumber,
    PVMDIR_TEST_STATE pState
    )
{
    if (!IsNullOrEmptyString(pszCustomMsg))
    {
        printf("ERROR: %s\n", pszCustomMsg);
    }

    printf("%s:%d: %s: Assertion `%s` failed.\n", pszFile, dwLineNumber, pszFunction, pszExpression);
    _VmDirTestAssertionWorker(pState);
}

VOID
VmDirTestReportAssertionFailureDwordOperands(
    PCSTR pszSideA,
    PCSTR pszSideB,
    DWORD dwValueA,
    DWORD dwValueB,
    BOOLEAN bEquality,
    PCSTR pszFile,
    PCSTR pszFunction,
    DWORD dwLineNumber,
    PVMDIR_TEST_STATE pState
    )
{
    printf("%s:%d: %s:\n", pszFile, dwLineNumber, pszFunction);
    printf("Assertion Failure: %s %s %s\n", pszSideA, bEquality ? "==" : "!=", pszSideB);
    printf("Actual values: %d / %d\n", dwValueA, dwValueB);

    _VmDirTestAssertionWorker(pState);
}

VOID
VmDirTestReportAssertionFailurePtrOperands(
    PCSTR pszSideA,
    PCSTR pszSideB,
    PVOID pValueA,
    PVOID pValueB,
    BOOLEAN bEquality,
    PCSTR pszFile,
    PCSTR pszFunction,
    DWORD dwLineNumber,
    PVMDIR_TEST_STATE pState
    )
{
    printf("%s:%d: %s:\n", pszFile, dwLineNumber, pszFunction);
    printf("Assertion Failure: %s %s %s\n", pszSideA, bEquality ? "==" : "!=", pszSideB);
    printf("Actual values: %p / %p\n", pValueA, pValueB);

    _VmDirTestAssertionWorker(pState);
}

VOID
VmDirTestReportAssertionFailureStringOperands(
    PCSTR pszSideA,
    PCSTR pszSideB,
    PCSTR pszValueA,
    PCSTR pszValueB,
    BOOLEAN bEquality,
    PCSTR pszFile,
    PCSTR pszFunction,
    DWORD dwLineNumber,
    PVMDIR_TEST_STATE pState
    )
{
    printf("%s:%d: %s:\n", pszFile, dwLineNumber, pszFunction);
    printf("Assertion Failure: %s %s %s\n", pszSideA, bEquality ? "==" : "!=", pszSideB);
    printf("Actual values: %s / %s\n", pszValueA, pszValueB);

    _VmDirTestAssertionWorker(pState);
}
