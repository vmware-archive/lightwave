/*
 * Copyright © 219 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the ?~@~\License?~@~]); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an ?~@~\AS IS?~@~] BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */

#include "vmincludes.h"
#include "vmregconfig.h"
#include "vmregconfigtest.h"

typedef struct _VM_REGCONFIG_TEST_PAIR
{
    PSTR    pszKey;
    PSTR    pszValue;
} VM_REGCONFIG_TEST_PAIR, *PVM_REGCONFIG_TEST_PAIR;

PSTR    gpszVmDirFileName = VM_REGCONFIG_TEST_VMDIR_FILE;
PSTR    gpszVmAfdFileName = VM_REGCONFIG_TEST_VMAFD_FILE;

PSTR    gpszVmdirDataBuf =
"vmdir:\n"
"    description: \"VMware Directory Service\"\n"
"    testEmptyValueKey: \"\"\n"
"    user: \"lightwave\"\n"
"    group: \"lightwave\"\n"
"\n"
"    parameters:\n"
"        defautSchema: \"vmdirschema.ldif\"\n"
"        sslDisabledProtocols: \"TLSV1\\nTLSv1.1\\n\"\n"
"        enableRename: \"0\"\n"
"\n"
"        nestedparameters:\n"
"            nestedstr: \"testnestedstr\"\n"
"            nestedint: 100\n"
"\n"
"        dcAccount: lw-t5.lw.local\n"
"        Password: ;iR7{SX|1/]WiRb41Qm|\n"
"\n"
"    metadata:\n"
"        enableLdapCopy: 1\n";

VM_REGCONFIG_TEST_PAIR VmDirOrgPair[] =
{
    {"Services\\vmdir\\description", "VMware Directory Service"},
// Empty string is allowed
    {"Services\\vmdir\\testEmptyValueKey", ""},
    {"Services\\vmdir\\user", "lightwave"},
    {"Services\\vmdir\\group", "lightwave"},
    {"Services\\vmdir\\parameters\\defautSchema", "vmdirschema.ldif"},
// MultiSZ key with new line embedded
    {"Services\\vmdir\\parameters\\sslDisabledProtocols", "TLSV1\nTLSv1.1\n"},
    {"Services\\vmdir\\parameters\\enableRename", "0"},
    {"Services\\vmdir\\parameters\\nestedparameters\\nestedstr", "testnestedstr"},
    {"Services\\vmdir\\parameters\\nestedparameters\\nestedint", "100"},
    {"Services\\vmdir\\parameters\\dcAccount", "lw-t5.lw.local"},
    {"Services\\vmdir\\parameters\\Password", ";iR7{SX|1/]WiRb41Qm|"},
    {"Services\\vmdir\\metadata\\enableLdapCopy", "1"},
};


VM_REGCONFIG_TEST_PAIR VmDirSetPair[] =
{
    // add new key
    {"Services\\vmdir\\parameters\\dcAccountDN", "cn=lw-t5.lw.local,ou=domain controllers,dc=lw,dc=local"},
    {"Services\\vmdir\\parameters\\Password", "Qwe234JKL&*("},
// Double quote is value is ok
    {"Services\\vmdir\\parameters\\testValueWithDQuote", "TEST STRING VALUE WITH DOUBLE QUOTE \" here"},
    // replace existing key
    {"Services\\vmdir\\parameters\\sslDisabledProtocols", "TLSV1 yet others\nTLSv1.1 and others\n"},
};

VM_REGCONFIG_TEST_PAIR VmDirMultiSZPair[] =
{
    // add new multi sz key
    {"Services\\vmdir\\parameters\\newMultiSZValue", "SZ1\0SZ2\0"},
};

VM_REGCONFIG_TEST_PAIR VmDirAfterSetPair[] =
{
    {"Services\\vmdir\\description", "VMware Directory Service"},
    {"Services\\vmdir\\testEmptyValueKey", ""},
    {"Services\\vmdir\\user", "lightwave"},
    {"Services\\vmdir\\group", "lightwave"},
    {"Services\\vmdir\\parameters\\defautSchema", "vmdirschema.ldif"},
    //set  {"Services\\vmdir\\parameters\\sslDisabledProtocols", "TLSV1 and  others\nTLSv1.1\n"},
    {"Services\\vmdir\\parameters\\newMultiSZValue", "SZ1\nSZ2\n"},
    {"Services\\vmdir\\parameters\\sslDisabledProtocols", "TLSV1 yet others\nTLSv1.1 and others\n"},
    {"Services\\vmdir\\parameters\\enableRename", "0"},
    {"Services\\vmdir\\parameters\\nestedparameters\\nestedstr", "testnestedstr"},
    {"Services\\vmdir\\parameters\\nestedparameters\\nestedint", "100"},
    {"Services\\vmdir\\parameters\\dcAccount", "lw-t5.lw.local"},
    //set  {"Services\\vmdir\\parameters\\Password", ";iR7{SX|1/]WiRb41Qm|"},
    {"Services\\vmdir\\parameters\\Password", "QWE234JKL&*("},
    {"Services\\vmdir\\metadata\\enableLdapCopy", "1"},

    {"Services\\vmdir\\parameters\\dcAccountDN", "cn=lw-t5.lw.local,ou=domain controllers,dc=lw,dc=local"},
    {"Services\\vmdir\\parameters\\testValueWithDQuote", "TEST STRING VALUE WITH DOUBLE QUOTE \" here"},
};

VM_REGCONFIG_TEST_PAIR VmDirDelPair[] =
{
    {"Services\\vmdir\\description", "VMware Directory Service"},
    {"Services\\vmdir\\testEmptyValueKey", ""},
    {"Services\\vmdir\\parameters\\sslDisabledProtocols", "TLSV1 yet others\nTLSv1.1 and others\n"},
    {"Services\\vmdir\\metadata\\enableLdapCopy", "1"},
    {"Services\\vmdir\\parameters\\dcAccountDN", "cn=lw-t5.lw.local,ou=domain controllers,dc=lw,dc=local"},
    {"Services\\vmdir\\parameters\\testValueWithDQuote", "TEST STRING VALUE WITH DOUBLE QUOTE \" here"},
};

VM_REGCONFIG_TEST_PAIR VmDirAfterDelPair[] =
{
    //del    {"Services\\vmdir\\description", "VMware Directory Service"},
    //del    {"Services\\vmdir\\testEmptyValueKey", ""},
    {"Services\\vmdir\\user", "lightwave"},
    {"Services\\vmdir\\group", "lightwave"},
    {"Services\\vmdir\\parameters\\defautSchema", "vmdirschema.ldif"},
    //set  {"Services\\vmdir\\parameters\\sslDisabledProtocols", "TLSV1 and  others\nTLSv1.1\n"},
    //del   {"Services\\vmdir\\parameters\\sslDisabledProtocols", "TLSV1 yet others\nTLSv1.1 and others\n"},
    {"Services\\vmdir\\parameters\\newMultiSZValue", "SZ1\nSZ2\n"},
    {"Services\\vmdir\\parameters\\enableRename", "0"},
    {"Services\\vmdir\\parameters\\nestedparameters\\nestedstr", "testnestedstr"},
    {"Services\\vmdir\\parameters\\nestedparameters\\nestedint", "100"},
    {"Services\\vmdir\\parameters\\dcAccount", "lw-t5.lw.local"},
    //set  {"Services\\vmdir\\parameters\\Password", ";iR7{SX|1/]WiRb41Qm|"},
    {"Services\\vmdir\\parameters\\Password", "QWE234JKL&*("},
    //del    {"Services\\vmdir\\metadata\\enableLdapCopy", "1"},

    //del    {"Services\\vmdir\\parameters\\dcAccountDN", "cn=lw-t5.lw.local,ou=domain controllers,dc=lw,dc=local"},
    //del    {"Services\\vmdir\\parameters\\testValueWithDQuote", "TEST STRING VALUE WITH DOUBLE QUOTE \" here"},
};

PSTR    gpszVmAfdDataBuf =
"vmafd:\n"
"    description: VMware Afd Service\n"
"    user: lightwave\n"
"    group: lightwave\n"
"\n"
"    parameters:\n"
"        LogFile: /var/log/vmware/vmafd/vmafdd.log\n"
"        DomainState: 0\n"
"        HeartbeatInterval: 10\n";

VM_REGCONFIG_TEST_PAIR VmAfdOrgPair[] =
{
    {"Services\\vmafd\\description", "VMware Afd Service"},
    {"Services\\vmafd\\user", "lightwave"},
    {"Services\\vmafd\\group", "lightwave"},
    {"Services\\vmafd\\parameters\\LogFile", "/var/log/vmware/vmafd/vmafdd.log"},
    {"Services\\vmafd\\parameters\\DomainState", "0"},
    {"Services\\vmafd\\parameters\\HeartbeatInterval", "10"},
};

VM_REGCONFIG_TEST_PAIR VmAfdSetPair[] =
{
    {"Services\\vmafd\\parameters\\DomainState", "1"},
    {"Services\\vmafd\\parameters\\HeartbeatInterval", "10"},
    {"Services\\vmafd\\metadata\\TestAddNewKey", "has new key"},
};

VM_REGCONFIG_TEST_PAIR VmAfdAfterSetPair[] =
{
    {"Services\\vmafd\\description", "VMware Afd Service"},
    {"Services\\vmafd\\user", "lightwave"},
    {"Services\\vmafd\\group", "lightwave"},
    {"Services\\vmafd\\parameters\\LogFile", "/var/log/vmware/vmafd/vmafdd.log"},
    //set    {"Services\\vmafd\\parameters\\DomainState", "0"},
    {"Services\\vmafd\\parameters\\DomainState", "1"},
    //set    {"Services\\vmafd\\parameters\\HeartbeatInterval", "10"},
    {"Services\\vmafd\\parameters\\HeartbeatInterval", "10"},
    {"Services\\vmafd\\metadata\\TestAddNewKey", "has new key"},
};

VM_REGCONFIG_TEST_PAIR VmAfdDelPair[] =
{
    {"Services\\vmafd\\user", "lightwave"},
    {"Services\\vmafd\\group", "lightwave"},
    {"Services\\vmafd\\parameters\\HeartbeatInterval", "10"},
    {"Services\\vmafd\\metadata\\TestAddNewKey", "has new key"},
};

VM_REGCONFIG_TEST_PAIR VmAfdAfterDelPair[] =
{
    {"Services\\vmafd\\description", "VMware Afd Service"},
    //del    {"Services\\vmafd\\user", "lightwave"},
    //del    {"Services\\vmafd\\group", "lightwave"},
    {"Services\\vmafd\\parameters\\LogFile", "/var/log/vmware/vmafd/vmafdd.log"},
    //set    {"Services\\vmafd\\parameters\\DomainState", "0"},
    {"Services\\vmafd\\parameters\\DomainState", "1"},
    //set    {"Services\\vmafd\\parameters\\HeartbeatInterval", "10"},
    //del    {"Services\\vmafd\\parameters\\HeartbeatInterval", "10"},
    //del    {"Services\\vmafd\\metadata\\TestAddNewKey", "has new key"},
};


DWORD
_TestCreateAndAddFile(
    VOID
    )
{
    DWORD   dwError = 0;
    FILE*   fh = NULL;


    fh = fopen(gpszVmDirFileName, "w");
    if (!fh)
    {
        printf("FAIL: Error in open vmdir data file for test: %s, Error Code = %d\n", __FUNCTION__, dwError);
        BAIL_WITH_VM_COMMON_ERROR(dwError,VM_COMMON_ERROR_FILE_IO);
    }

    if (fwrite(gpszVmdirDataBuf, 1, VmStringLenA(gpszVmdirDataBuf), fh) != VmStringLenA(gpszVmdirDataBuf))
    {
        printf("FAIL: Error in create vmdir data file for test: %s, Error Code = %d\n", __FUNCTION__, dwError);
        BAIL_WITH_VM_COMMON_ERROR(dwError,VM_COMMON_ERROR_FILE_IO);
    }
    fclose(fh);

    dwError = VmRegConfigAddFile(gpszVmDirFileName, FALSE);
    if (dwError)
    {
        printf("FAIL: Error in vmdir VmRegConfigAddFile for test: %s, Error Code = %d\n", __FUNCTION__, dwError);
        BAIL_ON_VM_COMMON_ERROR(dwError)
    }

    fh = NULL;
    fh = fopen(gpszVmAfdFileName, "w");
    if (!fh)
    {
        printf("FAIL: Error in open vmafd data file for test: %s, Error Code = %d\n", __FUNCTION__, dwError);
        BAIL_WITH_VM_COMMON_ERROR(dwError,VM_COMMON_ERROR_FILE_IO);
    }

    if (fwrite(gpszVmAfdDataBuf, 1, VmStringLenA(gpszVmAfdDataBuf), fh) != VmStringLenA(gpszVmAfdDataBuf))
    {
        printf("FAIL: Error in create vmafd data file for test: %s, Error Code = %d\n", __FUNCTION__, dwError);
        BAIL_WITH_VM_COMMON_ERROR(dwError,VM_COMMON_ERROR_FILE_IO);
    }
    fclose(fh);

    dwError = VmRegConfigAddFile(gpszVmAfdFileName, FALSE);
    if (dwError)
    {
        printf("FAIL: Error in vmafd VmRegConfigAddFile for test: %s, Error Code = %d\n", __FUNCTION__, dwError);
        BAIL_ON_VM_COMMON_ERROR(dwError)
    }

error:
    return dwError;
}

DWORD
_TestReadKey(
    PVM_REGCONFIG_TEST_PAIR pPair,
    DWORD                   dwSize
    )
{
    DWORD   dwError = 0;
    DWORD   dwCnt = 0;
    CHAR    pszValueBuf[VM_SIZE_1024] = {0};
    size_t  iValueBufSize = 0;

    for (dwCnt=0; dwCnt < dwSize; dwCnt++)
    {
        memset(pszValueBuf, 0, VM_SIZE_1024);
        iValueBufSize = VM_SIZE_1024;
        dwError = VmRegConfigGetKeyA(pPair[dwCnt].pszKey, pszValueBuf, &iValueBufSize);
        if (dwError)
        {
            printf("FAIL: get key (%s) not match (%s)(%s) for test: %s, Error Code = %d\n",
                    pPair[dwCnt].pszKey, pPair[dwCnt].pszValue, pszValueBuf, __FUNCTION__, dwError);
            BAIL_WITH_VM_COMMON_ERROR(dwError, VM_COMMON_ERROR_REGCONFIG_BAD_KEY);
        }
        BAIL_ON_VM_COMMON_ERROR(dwError);

        if (VmStringCompareA(pszValueBuf, pPair[dwCnt].pszValue, FALSE) != 0)
        {
            printf("FAIL: key (%s) not match (%s)(%s) for test: %s, Error Code = %d\n",
                    pPair[dwCnt].pszKey, pPair[dwCnt].pszValue, pszValueBuf, __FUNCTION__, dwError);
            BAIL_WITH_VM_COMMON_ERROR(dwError, VM_COMMON_ERROR_REGCONFIG_BAD_KEY);
        }
    }

error:
    return dwError;
}

DWORD
_TestReadMultiSZKey(
    PVM_REGCONFIG_TEST_PAIR pPair,
    DWORD                   dwSize
    )
{
    DWORD   dwError = 0;
    DWORD   dwCnt = 0;
    CHAR    pszValueBuf[VM_SIZE_1024] = {0};
    size_t  iValueBufSize = 0;

    for (dwCnt=0; dwCnt < dwSize; dwCnt++)
    {
        memset(pszValueBuf, 0, VM_SIZE_1024);
        iValueBufSize = VM_SIZE_1024;
        dwError = VmRegConfigGetMultiSZKeyA(pPair[dwCnt].pszKey, pszValueBuf, &iValueBufSize);
        if (dwError)
        {
            printf("FAIL: get key (%s) not match (%s)(%s) for test: %s, Error Code = %d\n",
                    pPair[dwCnt].pszKey, pPair[dwCnt].pszValue, pszValueBuf, __FUNCTION__, dwError);
            BAIL_WITH_VM_COMMON_ERROR(dwError, VM_COMMON_ERROR_REGCONFIG_BAD_KEY);
        }
        BAIL_ON_VM_COMMON_ERROR(dwError);

        if (memcmp(pszValueBuf, pPair[dwCnt].pszValue, iValueBufSize) != 0)
        {
            printf("FAIL: key (%s) not match (%s)(%s) for test: %s, Error Code = %d\n",
                    pPair[dwCnt].pszKey, pPair[dwCnt].pszValue, pszValueBuf, __FUNCTION__, dwError);
            BAIL_WITH_VM_COMMON_ERROR(dwError, VM_COMMON_ERROR_REGCONFIG_BAD_KEY);
        }
    }

error:
    return dwError;
}

DWORD
_TestSetMultiSZKey(
    PVM_REGCONFIG_TEST_PAIR pPair,
    DWORD                   dwSize
    )
{
    DWORD   dwError = 0;
    DWORD   dwCnt = 0;

    for (dwCnt=0; dwCnt < dwSize; dwCnt++)
    {
	DWORD	dwLen = 0;

	for (dwLen = 0;
	     pPair[dwCnt].pszValue[dwLen] != '\0' ||
	     pPair[dwCnt].pszValue[dwLen+1] != '\0' ;
	     dwLen++) {}

        dwError = VmRegConfigSetMultiSZKeyA(
                pPair[dwCnt].pszKey,
                pPair[dwCnt].pszValue,
                dwLen+1);
        BAIL_ON_VM_COMMON_ERROR(dwError);
    }

error:
    return dwError;
}

DWORD
_TestSetKey(
    VOID
    )
{
    DWORD   dwError = 0;
    DWORD   dwCnt = 0;

    for (dwCnt=0; dwCnt < sizeof(VmDirSetPair)/sizeof(VmDirSetPair[0]); dwCnt++)
    {
        dwError = VmRegConfigSetKeyA(
                VmDirSetPair[dwCnt].pszKey,
                VmDirSetPair[dwCnt].pszValue,
                VmStringLenA(VmDirSetPair[dwCnt].pszValue));
        BAIL_ON_VM_COMMON_ERROR(dwError);
    }

    dwError = _TestSetMultiSZKey(
            &VmDirMultiSZPair[0],
            sizeof(VmDirMultiSZPair)/sizeof(VmDirMultiSZPair[0]));
    BAIL_ON_VM_COMMON_ERROR(dwError);

    dwError = _TestReadMultiSZKey(
            &VmDirMultiSZPair[0],
            sizeof(VmDirMultiSZPair)/sizeof(VmDirMultiSZPair[0]));
    BAIL_ON_VM_COMMON_ERROR(dwError);

    dwError = _TestReadKey(
            &VmDirAfterSetPair[0],
            sizeof(VmDirAfterSetPair)/sizeof(VmDirAfterSetPair[0]));
    BAIL_ON_VM_COMMON_ERROR(dwError);

    for (dwCnt=0; dwCnt < sizeof(VmAfdSetPair)/sizeof(VmAfdSetPair[0]); dwCnt++)
    {
        dwError = VmRegConfigSetKeyA(
                VmAfdSetPair[dwCnt].pszKey,
                VmAfdSetPair[dwCnt].pszValue,
                VmStringLenA(VmAfdSetPair[dwCnt].pszValue));
        BAIL_ON_VM_COMMON_ERROR(dwError);
    }

    dwError = _TestReadKey(
            &VmAfdAfterSetPair[0],
            sizeof(VmAfdAfterSetPair)/sizeof(VmAfdAfterSetPair[0]));
    BAIL_ON_VM_COMMON_ERROR(dwError);

cleanup:
    return dwError;

error:
    goto cleanup;
}

DWORD
_TestDelKey(
    VOID
    )
{
    DWORD   dwError = 0;
    DWORD   dwCnt = 0;

    for (dwCnt=0; dwCnt < sizeof(VmDirDelPair)/sizeof(VmDirDelPair[0]); dwCnt++)
    {
        dwError = VmRegConfigDeleteKeyA(VmDirDelPair[dwCnt].pszKey);
        BAIL_ON_VM_COMMON_ERROR(dwError);
    }

    dwError = _TestReadKey(
            &VmDirAfterDelPair[0],
            sizeof(VmDirAfterDelPair)/sizeof(VmDirAfterDelPair[0]));
    BAIL_ON_VM_COMMON_ERROR(dwError);

    for (dwCnt=0; dwCnt < sizeof(VmAfdDelPair)/sizeof(VmAfdDelPair[0]); dwCnt++)
    {
        dwError = VmRegConfigDeleteKeyA(VmAfdDelPair[dwCnt].pszKey);
        BAIL_ON_VM_COMMON_ERROR(dwError);
    }

    dwError = _TestReadKey(
            &VmAfdAfterDelPair[0],
            sizeof(VmAfdAfterDelPair)/sizeof(VmAfdAfterDelPair[0]));
    BAIL_ON_VM_COMMON_ERROR(dwError);

cleanup:
    return dwError;

error:
    goto cleanup;
}

DWORD
_TestReloadKey(
    VOID
    )
{
    DWORD   dwError = 0;

    dwError = _TestReadKey(
            &VmDirAfterDelPair[0],
            sizeof(VmDirAfterDelPair)/sizeof(VmDirAfterDelPair[0]));
    BAIL_ON_VM_COMMON_ERROR(dwError);

    dwError = _TestReadKey(
            &VmAfdAfterDelPair[0],
            sizeof(VmAfdAfterDelPair)/sizeof(VmAfdAfterDelPair[0]));
    BAIL_ON_VM_COMMON_ERROR(dwError);

cleanup:
    return dwError;

error:
    goto cleanup;
}

DWORD
_TestMisc(
    PVM_REGCONFIG_CONTEXT   pContext
    )
{
    // add bad key  '\\' ':'
    // add bad value start with '|'
    // add key where sub key not exist yet
    // delete key cause subtree empty ?
    // delete key where sub key exists

    return 0;
}


DWORD
VmRegConfigTest(
    VOID
    )
{
    DWORD   dwError = 0;

    dwError = VmRegConfigInit();
    if (dwError)
    {
        printf("FAIL: Error in VmRegConfigInit for test: %s, Error Code = %d\n", __FUNCTION__, dwError);
        goto error;
    }

    dwError = _TestCreateAndAddFile();
    if (dwError)
    {
        printf("FAIL: Error in _TestCreateAndAddFile for test: %s, Error Code = %d\n", __FUNCTION__, dwError);
        goto error;
    }

    dwError = _TestReadKey(&VmDirOrgPair[0], sizeof(VmDirOrgPair)/sizeof(VmDirOrgPair[0])) +
              _TestReadKey( &VmAfdOrgPair[0], sizeof(VmAfdOrgPair)/sizeof(VmAfdOrgPair[0]));
    if (dwError)
    {
        printf("FAIL: Error in _TestReadKey for test: %s, Error Code = %d\n", __FUNCTION__, dwError);
        goto error;
    }

    dwError = _TestSetKey();
    if (dwError)
    {
        printf("FAIL: Error in _TestWriteKey for test: %s, Error Code = %d\n", __FUNCTION__, dwError);
        goto error;
    }

    dwError = _TestDelKey();
    if (dwError)
    {
        printf("FAIL: Error in _TestDelKey for test: %s, Error Code = %d\n", __FUNCTION__, dwError);
        goto error;
    }

    dwError = _TestReloadKey();
    if (dwError)
    {
        printf("FAIL: Error in _TestReloadKey for test: %s, Error Code = %d\n", __FUNCTION__, dwError);
        goto error;
    }

    printf("PASS: %s\n", __FUNCTION__);

cleanup:
    unlink(gpszVmDirFileName);
    unlink(gpszVmAfdFileName);
    VmRegConfigFree();

    return dwError;

error:
    dwError = 1;
    goto cleanup;
}
