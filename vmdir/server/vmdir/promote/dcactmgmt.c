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



#include "includes.h"

static
DWORD
_VmDirPrepareDCParam(
    PVDIR_DC_ACT_PARAM  pDCParam,
    PCSTR   pszDomain,
    PCSTR   pszDCName
    );

static
DWORD
_VmDirCreateDCEntry(
    PVDIR_DC_ACT_PARAM  pDCParam
    );

static
DWORD
_VmDirCreateDCServiceEntry(
    PVDIR_DC_ACT_PARAM  pDCParam
    );

static
DWORD
_VmDirCreateServiceEntry(
    PVDIR_DC_ACT_PARAM  pDCParam,
    PCSTR   pszServiceName
    );

VOID
VmDirFreeDCActParamContent(
    PVDIR_DC_ACT_PARAM  pDCParam
    )
{
    if (pDCParam)
    {
        VMDIR_SAFE_FREE_MEMORY(pDCParam->pszDomainDN);
        VMDIR_SAFE_FREE_MEMORY(pDCParam->pszDCDN);
        VMDIR_SAFE_FREE_MEMORY(pDCParam->pszDCName);
        VMDIR_SAFE_FREE_MEMORY(pDCParam->pszPassword);
        VMDIR_SAFE_FREE_MEMORY(pDCParam->pszUPN);
        VMDIR_SAFE_FREE_MEMORY(pDCParam->pszMachineGUID);
        VMDIR_SAFE_FREE_MEMORY(pDCParam->pszUpperCaseDomain);
        VMDIR_SAFE_FREE_MEMORY(pDCParam->pszLowerCaseDCName);
    }
}

/*
 * Equivalent of VmDirSetupDefaultAccount in client/client.c
 */
DWORD
VmDirCreateDomainController(
    PCSTR   pszDomain,
    PCSTR   pszDCName
    )
{
    DWORD   dwError = 0;
    VDIR_DC_ACT_PARAM   dcParam = {0};
    PSTR    pszDCAdminsDN = NULL;

    dwError = _VmDirPrepareDCParam(&dcParam, pszDomain, pszDCName);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringPrintf(&pszDCAdminsDN, "%s=%s,%s=%s,%s",
        ATTR_CN,
        VMDIR_DC_GROUP_NAME,
        ATTR_CN,
        VMDIR_BUILTIN_CONTAINER_NAME,
        dcParam.pszDomainDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _VmDirCreateDCEntry(&dcParam);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirInternalAddMemberToGroup(pszDCAdminsDN, dcParam.pszDCDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirConfigSetDCAccountInfo(
        dcParam.pszDCName,
        dcParam.pszDCDN,
        dcParam.pszPassword,
        VmDirStringLenA(dcParam.pszPassword),
        dcParam.pszMachineGUID);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _VmDirCreateDCServiceEntry(&dcParam);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszDCAdminsDN);
    VmDirFreeDCActParamContent(&dcParam);

    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "error (%u)", dwError);

    goto cleanup;
}

/*
 * Equivalent of VmDirLdapSetupDCAccountOnPartner in client/client.c
 */
static
DWORD
_VmDirCreateDCEntry(
    PVDIR_DC_ACT_PARAM  pDCParam
    )
{
    DWORD   dwError = 0;
    PVDIR_SCHEMA_CTX    pSchemaCtx = NULL;

    dwError = VmDirSchemaCtxAcquire(&pSchemaCtx);
    BAIL_ON_VMDIR_ERROR( dwError );

    {
        PSTR    ppszAttributes[] =
        {
            ATTR_OBJECT_CLASS,      (PSTR)OC_COMPUTER,
            ATTR_CN,                (PSTR)pDCParam->pszDCName,
            ATTR_USER_PASSWORD,     (PSTR)pDCParam->pszPassword,
            ATTR_SAM_ACCOUNT_NAME,  (PSTR)pDCParam->pszDCName,
            ATTR_MACHINE_GUID,      (PSTR)pDCParam->pszMachineGUID,
            ATTR_PSC_VERSION,       (PSTR)VDIR_PSC_VERSION,
            ATTR_MAX_DOMAIN_FUNCTIONAL_LEVEL, (PSTR)VMDIR_MAX_DFL_STRING,
            ATTR_KRB_UPN,           (PSTR)pDCParam->pszUPN,
            NULL
        };

        dwError = VmDirSimpleEntryCreate(
                        pSchemaCtx,
                        ppszAttributes,
                        (PSTR)pDCParam->pszDCDN,
                        0);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

cleanup:
    VmDirSchemaCtxRelease(pSchemaCtx);

    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "error (%u)", dwError);

    goto cleanup;
}

static
DWORD
_VmDirPrepareDCParam(
    PVDIR_DC_ACT_PARAM  pDCParam,
    PCSTR   pszDomain,
    PCSTR   pszDCName
    )
{
    DWORD   dwError = 0;

    dwError = VmDirDomainNameToDN(pszDomain, &pDCParam->pszDomainDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringA(pszDCName, &pDCParam->pszDCName);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocASCIIUpperToLower(pszDCName, &pDCParam->pszLowerCaseDCName);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringPrintf(&pDCParam->pszDCDN, "%s=%s,%s=%s,%s",
        ATTR_CN,
        pDCParam->pszLowerCaseDCName,
        ATTR_OU,
        VMDIR_DOMAIN_CONTROLLERS_RDN_VAL,
        pDCParam->pszDomainDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirGenerateRandomInternalPassword(pszDomain, &pDCParam->pszPassword);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirGenerateGUID(&pDCParam->pszMachineGUID);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocASCIILowerToUpper(pszDomain, &pDCParam->pszUpperCaseDomain);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringPrintf(&pDCParam->pszUPN, "%s@%s",
        pDCParam->pszLowerCaseDCName,
        pDCParam->pszUpperCaseDomain);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "error (%u)", dwError);

    goto cleanup;
}

/*
 * Equivalent of VmDirLdapSetupServiceAccount in client/ldaputil.c
 */
static
DWORD
_VmDirCreateServiceEntry(
    PVDIR_DC_ACT_PARAM  pDCParam,
    PCSTR   pszServiceName
    )
{
    DWORD   dwError = 0;
    PSTR    pszLocalServiceFullName = NULL;
    PSTR    pszLocalServiceUPN = NULL;
    PSTR    pszLocalServiceDN = NULL;
    PSTR    pszLocalPassword = NULL;
    PVDIR_SCHEMA_CTX    pSchemaCtx = NULL;

    dwError = VmDirSchemaCtxAcquire(&pSchemaCtx);
    BAIL_ON_VMDIR_ERROR( dwError );

    dwError = VmDirAllocateStringPrintf(&pszLocalServiceFullName, "%s/%s",
        pszServiceName,
        pDCParam->pszDCName);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringPrintf(&pszLocalServiceUPN, "%s/%s@%s",
        pszServiceName,
        pDCParam->pszLowerCaseDCName,
        pDCParam->pszUpperCaseDomain);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringPrintf(&pszLocalServiceDN, "%s=%s,%s=%s,%s",
        ATTR_CN,
        pszLocalServiceUPN,
        ATTR_CN,
        VMDIR_MSAS_RDN_VAL,
        pDCParam->pszDomainDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirGenerateRandomInternalPassword(
        pDCParam->pszUpperCaseDomain,
        &pszLocalPassword);
    BAIL_ON_VMDIR_ERROR(dwError);

    {
        PSTR    ppszAttributes[] =
        {
            ATTR_OBJECT_CLASS,      (PSTR)OC_MANAGED_SERVICE_ACCOUNT,
            ATTR_CN,                (PSTR)pszLocalServiceFullName,
            ATTR_SAM_ACCOUNT_NAME,  (PSTR)pszLocalServiceFullName,
            ATTR_KRB_UPN,           (PSTR)pszLocalServiceUPN,
            ATTR_USER_PASSWORD,     (PSTR)pszLocalPassword,
            NULL
        };

        dwError = VmDirSimpleEntryCreate(
                        pSchemaCtx,
                        ppszAttributes,
                        (PSTR)pszLocalServiceDN,
                        0);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszLocalServiceFullName);
    VMDIR_SAFE_FREE_MEMORY(pszLocalServiceUPN);
    VMDIR_SAFE_FREE_MEMORY(pszLocalServiceDN);
    VMDIR_SAFE_FREE_MEMORY(pszLocalPassword);

    VmDirSchemaCtxRelease(pSchemaCtx);

    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "error (%u)", dwError);

    goto cleanup;
}

static
DWORD
_VmDirCreateDCServiceEntry(
    PVDIR_DC_ACT_PARAM  pDCParam
    )
{
    DWORD   dwError = 0;
    int     iCnt = 0;
    PCSTR   pszServiceTable[] = VMDIR_DEFAULT_SERVICE_PRINCIPAL_INITIALIZER;

    for (iCnt = 0; iCnt < sizeof(pszServiceTable)/sizeof(pszServiceTable[0]); iCnt++)
    {
        dwError = _VmDirCreateServiceEntry(pDCParam, pszServiceTable[iCnt]);

        if (dwError == VMDIR_ERROR_BACKEND_ENTRY_EXISTS)
        {
            dwError = LDAP_SUCCESS;
        }
        BAIL_ON_VMDIR_ERROR(dwError);
    }

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "error (%u)", dwError);

    goto cleanup;
}

