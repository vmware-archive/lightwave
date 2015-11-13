#include "includes.h"

VOID
_Test_VmdirAllocateStringA_NullSourceString(
    VOID
    )
{
    DWORD dwError = 0;
    PSTR pszString = (PSTR)0xDEADBEEF;

    dwError = VmDirAllocateStringA(NULL, &pszString);
    ASSERT(dwError == 0);
    ASSERT(pszString == NULL);
}

VOID
_Test_VmdirAllocateStringA_EmptySourceString(
    VOID
    )
{
    DWORD dwError = 0;
    PSTR pszString = NULL;

    dwError = VmDirAllocateStringA("", &pszString);
    ASSERT(dwError == 0);
    ASSERT(*pszString == '\0');
}

VOID
_Test_VmdirAllocateStringA_NullDestinationString(
    VOID
    )
{
    DWORD dwError = 0;

    dwError = VmDirAllocateStringA("test", NULL);
    ASSERT(dwError == 0);
}

VOID
_Test_VmdirAllocateStringA_CallShouldSucceed(
    VOID
    )
{
    DWORD dwError = 0;
    PSTR pszString = NULL;

    dwError = VmDirAllocateStringA("Hello, world!", &pszString);
    ASSERT(dwError == 0);
    ASSERT(strcmp(pszString, "Hello, world!") == 0);
}


VOID
TestVmDirAllocateStringA(
    VOID
    )
{
    printf("Testing VmDirAllocateStringA ...\n");
    _Test_VmdirAllocateStringA_NullSourceString();
    _Test_VmdirAllocateStringA_EmptySourceString();
    _Test_VmdirAllocateStringA_NullDestinationString();
    _Test_VmdirAllocateStringA_CallShouldSucceed();
}
