/*
 * Copyright © 2012-2017 VMware, Inc.  All Rights Reserved.
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
 *
 * Author: Aishu Raghavan
 */



#include "includes.h"

static
DWORD
_VmDirSrvCreateOUContainer(
    PVDIR_SCHEMA_CTX pSchemaCtx,
    PCSTR            pszContainerDN,
    PCSTR            pszContainerName
    );

static
DWORD
_VmDirSrvLDAPAdd(
    PVDIR_CONNECTION pConnection,
    PCSTR pszBaseDN,
    LDAPMod* pLDAPMod[]
    );

static
DWORD
_VmDirSrvLDAPModify(
    PVDIR_CONNECTION pConnection,
    PCSTR pszBaseDN,
    LDAPMod* pLDAPMod
    );

static
DWORD
_VmDirSrvLDAPSearch(
    PVDIR_CONNECTION pConnection,
    PCSTR pszBaseDN,
    int searcScope,
    PCSTR pszFilter,
    PCSTR ppszAttrs[],
    DWORD dwAttrCount,
    PVDIR_ENTRY_ARRAY* ppEntryArray
    );

static
DWORD
_VmDirSrvLDAPModReplaceAttribute(
    PVDIR_CONNECTION pConnection,
    PCSTR pszBaseDN,
    PCSTR pszAttribute,
    PCSTR pszAttrValue
    );

static
DWORD
_VmDirSrvLDAPQueryAttrValues(
    PVDIR_ENTRY         pEntry,
    PCSTR               pszAttribute,
    PVMDIR_STRING_LIST  *ppAttributeList
    );


#ifndef LIGHTWAVE_BUILD
static
DWORD
_VmDirSrvLDAPSetupAccountMembership(
    PVDIR_CONNECTION   pConnection,
    PCSTR   pszDomainDN,
    PCSTR   pszBuiltinGroupName,
    PCSTR   pszAccountDN
    );
#endif

static
DWORD
_VmDirSrvConvertUPNToDN(
     PVDIR_CONNECTION pConnection,
     PCSTR      pszUPN,
     PSTR*      ppszOutDN
     );

static
DWORD
_VmDirSrvGetComputerAccountDN(
    PVDIR_CONNECTION pConnection,
    PCSTR pszDomain,
    PCSTR pszMachineName,
    PSTR* ppszAccountDN
    );

static
DWORD
_VmDirSrvGetComputerGuidInternal(
    PVDIR_CONNECTION pConnection,
    PCSTR pszDomain,
    PCSTR pszMachineName,
    PSTR* ppszGUID
    );

#if 0
static
DWORD
VmDirSrvLDAPDelete(
    );
#endif

DWORD
VmDirSrvGetConnectionObj(
    PCSTR  pszUPN,
    PVDIR_CONNECTION* ppConnection
    )
{
    DWORD dwError = 0;
    PVDIR_CONNECTION pConnection = NULL;
    PSTR pszBindDN = NULL;
    PVDIR_OPERATION pRpcSrvBindOp = NULL;

    if (IsNullOrEmptyString(pszUPN) || !ppConnection)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirUPNToDN(pszUPN, &pszBindDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateConnection(&pConnection);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirExternalOperationCreate(
                                       NULL,
                                       -1,
                                       LDAP_REQ_BIND,
                                       pConnection,
                                       &pRpcSrvBindOp
                                       );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirStringToBervalContent(pszBindDN, &pRpcSrvBindOp->reqDn);
    BAIL_ON_VMDIR_ERROR(dwError);

    pRpcSrvBindOp->request.bindReq.method = LDAP_AUTH_NONE;

    dwError = VmDirInternalBindEntry(pRpcSrvBindOp);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppConnection = pConnection;

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszBindDN);
    if (pRpcSrvBindOp)
    {
      VmDirFreeOperation(pRpcSrvBindOp);
    }
    return dwError;

error:
    if (ppConnection)
    {
        *ppConnection = NULL;
    }
    VmDirDeleteConnection(&pConnection);
    goto cleanup;
}

DWORD
VmDirSrvCreateComputerOUContainer(
    PVDIR_CONNECTION pConnection,
    PCSTR pszDomainName,
    PCSTR pszOUContainer
    )
{
    DWORD               dwError             = 0;
    PSTR                pszDomainDN         = NULL;
    PSTR                pszOuDN             = NULL;
    PVDIR_SCHEMA_CTX    pSchemaCtx          = NULL;
    PSTR                pszOUPrefix         = ATTR_OU "=";
    PVMDIR_STRING_LIST  pOUContainerList    = NULL;
    PVMDIR_STRING_LIST  pOUValuesList       = NULL;
    PSTR                pszTmp              = NULL;
    int                 idx                 = 0;

    if (!pszDomainName || !pszOUContainer)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if ( VmDirStringCompareA(pszOUContainer, VMDIR_COMPUTERS_RDN_VAL, FALSE) == 0)
    {
        goto cleanup;  // default container
    }

    dwError = VmDirDomainNameToDN(pszDomainName, &pszDomainDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSchemaCtxAcquire(&pSchemaCtx);
    BAIL_ON_VMDIR_ERROR(dwError);

    // create user specified OU container under default OU=computers container.
    if (VmDirStringNCompareA(pszOUContainer, pszOUPrefix,
                             VmDirStringLenA(pszOUPrefix), FALSE) != 0)
    {
        dwError = VmDirAllocateStringPrintf(
                    &pszOuDN,
                    "%s=%s,%s=%s,%s",
                    ATTR_OU,
                    pszOUContainer,
                    ATTR_OU,
                    VMDIR_COMPUTERS_RDN_VAL,
                    pszDomainDN);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = _VmDirSrvCreateOUContainer(
                                    pSchemaCtx,
                                    pszOuDN,
                                    pszOUContainer);
        if (dwError == VMDIR_ERROR_ENTRY_ALREADY_EXIST ||
            dwError == VMDIR_ERROR_BACKEND_ENTRY_EXISTS)
        {
            dwError = ERROR_SUCCESS;
        }
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    else
    {
        dwError = VmDirDNToRDNList(
                    pszOUContainer,
                    FALSE,
                    &pOUContainerList);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirDNToRDNList(
                    pszOUContainer,
                    TRUE,
                    &pOUValuesList);
        BAIL_ON_VMDIR_ERROR(dwError);

        for ( idx = pOUContainerList->dwCount - 1 ; idx >= 0 ; --idx )
        {
            if (idx == pOUContainerList->dwCount - 1)
            {
                dwError = VmDirAllocateStringPrintf(
                            &pszOuDN,
                            "%s,%s=%s,%s",
                            pOUContainerList->pStringList[idx],
                            ATTR_OU,
                            VMDIR_COMPUTERS_RDN_VAL,
                            pszDomainDN);
                BAIL_ON_VMDIR_ERROR(dwError);
            }
            else
            {
                pszTmp = pszOuDN;

                dwError = VmDirAllocateStringPrintf(
                            &pszOuDN,
                            "%s,%s",
                            pOUContainerList->pStringList[idx],
                            pszTmp);
                BAIL_ON_VMDIR_ERROR(dwError);

                VMDIR_SAFE_FREE_STRINGA(pszTmp);
            }

            dwError = _VmDirSrvCreateOUContainer(
                                        pSchemaCtx,
                                        pszOuDN,
                                        pOUValuesList->pStringList[idx]);
            if (dwError == VMDIR_ERROR_ENTRY_ALREADY_EXIST ||
                dwError == VMDIR_ERROR_BACKEND_ENTRY_EXISTS)
            {
                dwError = ERROR_SUCCESS;
            }
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }

cleanup:
    VmDirSchemaCtxRelease(pSchemaCtx);
    VMDIR_SAFE_FREE_MEMORY(pszDomainDN);
    VMDIR_SAFE_FREE_MEMORY(pszOuDN);
    VmDirStringListFree(pOUContainerList);
    VmDirStringListFree(pOUValuesList);

    return dwError;

error:

    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s. Error(%u)", __FUNCTION__, dwError);
    goto cleanup;
}

DWORD
VmDirSrvSetupComputerAccount(
    PVDIR_CONNECTION pConnection,
    PCSTR pszDomainName,
    PCSTR pszComputerOU,
    PCSTR pszMachineAccountName,
    PVMDIR_MACHINE_INFO_A* ppMachineInfo
    )
{
    DWORD       dwError = 0;
    PSTR        pszComputerDN = NULL;
    PSTR        pszDomainDN = NULL;
    PSTR        pByteAccountPasswd = NULL;
    DWORD       dwAccountPasswdSize = 0;
    PSTR        pszUPN = NULL;
    PSTR        pszUpperCaseDomainName = NULL;
    PSTR        pszLowerCaseComputerHostName = NULL;
    PSTR        pszSiteGUID = NULL;
    PSTR        pszMachineGUID = NULL;
    DWORD       dwRetries = 0;
    BOOLEAN     bAcctExists = FALSE;
    PSTR        pszExtendedOU = (PSTR)pszComputerOU;
    PSTR        pszOUPrefix = ATTR_OU "=";
    PVMDIR_MACHINE_INFO_A pMachineInfo = NULL;

    char* modv_oc[] = {OC_COMPUTER, NULL};
    char* modv_cn[] = {(PSTR)pszMachineAccountName, NULL};
    char* modv_sam[] = {(PSTR)pszMachineAccountName, NULL};
    char* modv_site[] = {(PSTR)NULL, NULL};
    char* modv_machine[] = {(PSTR)NULL, NULL};
    char* modv_upn[] = {(PSTR)NULL, NULL};
    char* modv_passwd[] = {(PSTR)NULL, NULL};

    BerValue    bvPasswd = {0};

    LDAPMod modObjectClass = {0};
    LDAPMod modCn = {0};
    LDAPMod modPwd = {0};
    LDAPMod modSamAccountName = {0};
    LDAPMod modUserPrincipalName = {0};
    LDAPMod modSiteGUID = {0};
    LDAPMod modMachineGUID = {0};

    LDAPMod* pDCMods[] =
    {
            &modObjectClass,
            &modCn,
            &modPwd,
            &modSamAccountName,
            &modUserPrincipalName,
            &modSiteGUID,
            &modMachineGUID,
            NULL
    };

    if (!pConnection ||
        IsNullOrEmptyString(pszDomainName) ||
        IsNullOrEmptyString(pszMachineAccountName))
    {
        dwError =  VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    modObjectClass.mod_op = LDAP_MOD_ADD;
    modObjectClass.mod_type = ATTR_OBJECT_CLASS;
    modObjectClass.mod_values = modv_oc;

    modCn.mod_op = LDAP_MOD_ADD;
    modCn.mod_type = ATTR_CN;
    modCn.mod_values = modv_cn;

    modPwd.mod_op = LDAP_MOD_ADD | LDAP_MOD_BVALUES;
    modPwd.mod_type = ATTR_USER_PASSWORD;
    modPwd.mod_values = modv_passwd;

    modSamAccountName.mod_op = LDAP_MOD_ADD;
    modSamAccountName.mod_type = ATTR_SAM_ACCOUNT_NAME;
    modSamAccountName.mod_values = modv_sam;

    modSiteGUID.mod_op = LDAP_MOD_ADD;
    modSiteGUID.mod_type = ATTR_SITE_GUID;
    modSiteGUID.mod_values = modv_site;

    modMachineGUID.mod_op = LDAP_MOD_ADD;
    modMachineGUID.mod_type = ATTR_MACHINE_GUID;
    modMachineGUID.mod_values = modv_machine;

    dwError = VmDirAllocASCIILowerToUpper( pszDomainName, &pszUpperCaseDomainName );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocASCIIUpperToLower(
                    pszMachineAccountName,
                    &pszLowerCaseComputerHostName );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringPrintf(
                    &pszUPN,
                    "%s@%s",
                    pszLowerCaseComputerHostName,
                    pszUpperCaseDomainName );
    BAIL_ON_VMDIR_ERROR(dwError);

    modv_upn[0] = pszUPN;

    modUserPrincipalName.mod_op = LDAP_MOD_ADD;
    modUserPrincipalName.mod_type = ATTR_KRB_UPN;
    modUserPrincipalName.mod_values = modv_upn;

    dwError = VmDirDomainNameToDN(pszDomainName, &pszDomainDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (VmDirStringCompareA(pszComputerOU, VMDIR_COMPUTERS_RDN_VAL, FALSE) != 0)
    {
        dwError = VmDirAllocateStringPrintf(
                        &pszExtendedOU,
                        "%s,%s=%s",
                        pszComputerOU,
                        ATTR_OU,
                        VMDIR_COMPUTERS_RDN_VAL);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (VmDirStringNCompareA(pszExtendedOU, pszOUPrefix,
                             VmDirStringLenA(pszOUPrefix), FALSE) != 0)
    {
        dwError = VmDirAllocateStringPrintf(
                        &pszComputerDN,
                        "%s=%s,%s=%s,%s",
                        ATTR_CN,
                        pszLowerCaseComputerHostName,
                        ATTR_OU,
                        pszExtendedOU,
                        pszDomainDN);
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    else
    {
        dwError = VmDirAllocateStringPrintf(
                        &pszComputerDN,
                        "%s=%s,%s,%s",
                        ATTR_CN,
                        pszLowerCaseComputerHostName,
                        pszExtendedOU,
                        pszDomainDN);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateMemory(
                               VMDIR_GUID_STR_LEN,
                               (PVOID*)&pszSiteGUID
                               );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirGetLocalSiteGuid(pszSiteGUID);
    BAIL_ON_VMDIR_ERROR(dwError);

    modv_site[0] = pszSiteGUID;

    dwError = VmDirGenerateGUID(&pszMachineGUID);
    BAIL_ON_VMDIR_ERROR(dwError);

    modv_machine[0] = pszMachineGUID;

    for (dwRetries=0; dwRetries < VMDIR_MAX_PASSWORD_RETRIES; dwRetries++)
    {
        dwError = VmDirGenerateRandomPasswordByDefaultPolicy(
                                         &pByteAccountPasswd
                                         );
        BAIL_ON_VMDIR_ERROR(dwError);

        modv_passwd[0] = pByteAccountPasswd;

        dwAccountPasswdSize = (int)VmDirStringLenA((PSTR)pByteAccountPasswd);

        bvPasswd.bv_val = pByteAccountPasswd;
        bvPasswd.bv_len = dwAccountPasswdSize;

        if (!bAcctExists)
        {
            // add ComputerAccount
            dwError = _VmDirSrvLDAPAdd(pConnection, pszComputerDN, pDCMods);
            if (dwError == LDAP_ALREADY_EXISTS)
            {
                bAcctExists = TRUE;
            }
        }

        if (bAcctExists)
        {
            // reset ComputerAccount password. NOTE pByteDCAccountPasswd is null terminated.
            dwError = _VmDirSrvLDAPModReplaceAttribute( pConnection, pszComputerDN, ATTR_USER_PASSWORD, pByteAccountPasswd );
        }

        if (dwError == LDAP_SUCCESS || dwError != LDAP_CONSTRAINT_VIOLATION)
        {
            break; // done or other unexpected error
        }

        // pasword LDAP_CONSTRAINT_VIOLATION retry again.
        VMDIR_SAFE_FREE_MEMORY(pByteAccountPasswd);
        dwAccountPasswdSize = 0;
    }
    BAIL_ON_VMDIR_ERROR(dwError);

    VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "Computer account (%s) created (recycle %s)", pszComputerDN, bAcctExists ? "T":"F");

#ifndef LIGHTWAVE_BUILD
    // add Computer Account into DCClients group
    dwError = _VmDirSrvLDAPSetupAccountMembership(
                                      pConnection,
                                      pszDomainDN,
                                      VMDIR_DCCLIENT_GROUP_NAME,
                                      pszComputerDN
                                      );
    BAIL_ON_VMDIR_ERROR(dwError);
#endif

    dwError = VmDirAllocateMemory(
                            sizeof(VMDIR_MACHINE_INFO_A),
                            (PVOID*)&pMachineInfo);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringA(
                              pszComputerDN,
                              &pMachineInfo->pszComputerDN
                              );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringA(
                              pszMachineGUID,
                              &pMachineInfo->pszMachineGUID
                              );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringA(
                              pByteAccountPasswd,
                              &pMachineInfo->pszPassword
                              );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringA(
                              gVmdirServerGlobals.pszSiteName,
                              &pMachineInfo->pszSiteName
                              );
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppMachineInfo = pMachineInfo;

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszComputerDN);
    VMDIR_SAFE_FREE_MEMORY(pszDomainDN);
    VMDIR_SAFE_FREE_MEMORY(pByteAccountPasswd);
    VMDIR_SAFE_FREE_MEMORY(pszUPN);
    VMDIR_SAFE_FREE_MEMORY(pszUpperCaseDomainName);
    VMDIR_SAFE_FREE_MEMORY(pszLowerCaseComputerHostName);
    VMDIR_SAFE_FREE_MEMORY(pszSiteGUID);
    VMDIR_SAFE_FREE_MEMORY(pszMachineGUID);

    if (pszExtendedOU != pszComputerOU)
    {
        VMDIR_SAFE_FREE_MEMORY(pszExtendedOU);
    }

    return dwError;

error:

    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s: (%s) failed with error (%u)",
                    __FUNCTION__, VDIR_SAFE_STRING(pszComputerDN), dwError);
    if (ppMachineInfo)
    {
        *ppMachineInfo = NULL;
    }
    if (pMachineInfo)
    {
        VmDirFreeMachineInfoA(pMachineInfo);
    }

    goto cleanup;
}

DWORD
VmDirSrvSetupServiceAccount(
    PVDIR_CONNECTION pConnection,
    PCSTR            pszDomainName,
    PCSTR            pszServiceName,
    PCSTR            pszDCHostName         // Self host name
    )
{
    DWORD       dwError = 0;
    PSTR        pszMSADN = NULL;
    PSTR        pszDomainDN = NULL;
    PBYTE       pByteMSAPasswd = NULL;
    DWORD       dwMSAPasswdSize = 0;
    PSTR        pszUPN = NULL;
    PSTR        pszName = NULL;
    PSTR        pszUpperCaseDomainName = NULL;
    PSTR        pszLowerCaseDCHostName = NULL;
    BOOLEAN     bAcctExists = FALSE;
    PSTR        pszSRPUPN = NULL;

    char* modv_oc[] = {OC_COMPUTER,NULL};
    char* modv_cn[] = {(PSTR)NULL, NULL};
    char* modv_sam[] = {(PSTR)NULL, NULL};
    char* modv_upn[] = {(PSTR)NULL, NULL};
    char* modv_passwd[] = {(PSTR)NULL, NULL};
    BerValue    bvPasswd = {0};

    LDAPMod modObjectClass = {0};
    LDAPMod modCn = {0};
    LDAPMod modPwd = {0};
    LDAPMod modSamAccountName = {0};
    LDAPMod modUserPrincipalName = {0};

    LDAPMod* pDCMods[] =
    {
            &modObjectClass,
            &modCn,
            &modPwd,
            &modSamAccountName,
            &modUserPrincipalName,
            NULL
    };

    if (!pConnection ||
        IsNullOrEmptyString(pszDomainName) ||
        IsNullOrEmptyString(pszServiceName) ||
        IsNullOrEmptyString(pszDCHostName)
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    modObjectClass.mod_op = LDAP_MOD_ADD;
    modObjectClass.mod_type = ATTR_OBJECT_CLASS;
    modObjectClass.mod_values = modv_oc;

    dwError = VmDirAllocateStringPrintf( &pszName, "%s/%s", pszServiceName, pszDCHostName );
    BAIL_ON_VMDIR_ERROR(dwError);

    modv_cn[0] = modv_sam[0] = pszName;

    modCn.mod_op = LDAP_MOD_ADD;
    modCn.mod_type = ATTR_CN;
    modCn.mod_values = modv_cn;

    modPwd.mod_op = LDAP_MOD_ADD | LDAP_MOD_BVALUES;
    modPwd.mod_type = ATTR_USER_PASSWORD;
    modPwd.mod_values = modv_passwd;

    modSamAccountName.mod_op = LDAP_MOD_ADD;
    modSamAccountName.mod_type = ATTR_SAM_ACCOUNT_NAME;
    modSamAccountName.mod_values = modv_sam;

    dwError = VmDirAllocASCIILowerToUpper( pszDomainName, &pszUpperCaseDomainName );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocASCIIUpperToLower( pszDCHostName, &pszLowerCaseDCHostName);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringPrintf( &pszUPN, "%s/%s@%s", pszServiceName, pszLowerCaseDCHostName, pszUpperCaseDomainName );
    BAIL_ON_VMDIR_ERROR(dwError);

    modv_upn[0] = pszUPN;

    modUserPrincipalName.mod_op = LDAP_MOD_ADD;
    modUserPrincipalName.mod_type = ATTR_KRB_UPN;
    modUserPrincipalName.mod_values = modv_upn;

    dwError = VmDirDomainNameToDN(pszDomainName, &pszDomainDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringPrintf( &pszMSADN, "%s=%s,%s=%s,%s", ATTR_CN, pszUPN,
                                             ATTR_CN, VMDIR_MSAS_RDN_VAL, pszDomainDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    while( TRUE )
    {
        dwError = VmDirGenerateRandomPasswordByDefaultPolicy(
                                        (PSTR*)&pByteMSAPasswd
                                        );
        BAIL_ON_VMDIR_ERROR(dwError);

        modv_passwd[0] = pByteMSAPasswd;

        dwMSAPasswdSize = (int)VmDirStringLenA((PSTR)pByteMSAPasswd);

        bvPasswd.bv_val = pByteMSAPasswd;
        bvPasswd.bv_len = dwMSAPasswdSize;

        // and the ldap_add_ext_s is a synchronous call
        dwError = _VmDirSrvLDAPAdd(pConnection, pszMSADN, pDCMods);
        if ( dwError == LDAP_SUCCESS )
        {
            break;
        }
        else if ( dwError == LDAP_ALREADY_EXISTS )
        {
            bAcctExists = TRUE;

            // reset ServiceAccount password. NOTE pByteDCAccountPasswd is null terminted.
            dwError = _VmDirSrvLDAPModReplaceAttribute( pConnection, pszMSADN, ATTR_USER_PASSWORD, pByteMSAPasswd );
            if (dwError == LDAP_CONSTRAINT_VIOLATION)
            {
                VMDIR_SAFE_FREE_MEMORY(pByteMSAPasswd);
                dwMSAPasswdSize = 0;
                continue;
            }
            BAIL_ON_VMDIR_ERROR(dwError);
            break;
        }
        else if (dwError == LDAP_CONSTRAINT_VIOLATION)
        {
            VMDIR_SAFE_FREE_MEMORY(pByteMSAPasswd);
            dwMSAPasswdSize = 0;
            continue;
        }
        else
        {
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }

    VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "Service account (%s) created (recycle %s)", pszMSADN, bAcctExists ? "T":"F");

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszMSADN);
    VMDIR_SAFE_FREE_MEMORY(pszDomainDN);
    VMDIR_SAFE_FREE_MEMORY(pByteMSAPasswd);
    VMDIR_SAFE_FREE_MEMORY(pszUPN);
    VMDIR_SAFE_FREE_MEMORY(pszName);
    VMDIR_SAFE_FREE_MEMORY(pszUpperCaseDomainName);
    VMDIR_SAFE_FREE_MEMORY(pszLowerCaseDCHostName);
    VMDIR_SAFE_FREE_MEMORY(pszSRPUPN);

    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s: (%s) failed with error (%u)",
                    __FUNCTION__, VDIR_SAFE_STRING(pszMSADN), dwError);
    goto cleanup;
}

DWORD
VmDirSrvGetKeyTabInfoClient(
    PVDIR_CONNECTION pConnection,
    PCSTR            pszDomainName,
    PCSTR            pszHostName,
    PVMDIR_KRB_INFO* ppKrbInfo
    )
{
    DWORD                   dwError = 0;
    PSTR                    pszUpperCaseDomainName = NULL;
    PSTR                    pszMachineAccountUPN = NULL;
    PSTR                    pszServiceAccountUPN = NULL;
    PCSTR                   pszClientServiceTable[] = VMDIR_CLIENT_SERVICE_PRINCIPAL_INITIALIZER;
    PCSTR                  *ppszServiceTable = NULL;
    int                     iServiceTableLen = 0;
    PSTR                    pszLowerCaseHostName = NULL;
    PVMDIR_KRB_INFO         pKrbInfo = NULL;
    DWORD                   dwKrbInfoCount = 0;
    DWORD                   dwIndex = 0;
    DWORD                   dwSrvTabIndex = 0;

    if (!pConnection ||
        IsNullOrEmptyString(pszDomainName) ||
        IsNullOrEmptyString(pszHostName) ||
        !ppKrbInfo
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }


    iServiceTableLen = VMDIR_ARRAY_SIZE(pszClientServiceTable);;
    dwKrbInfoCount = iServiceTableLen+1;

    dwError = VmDirAllocateMemory(
                              sizeof(VMDIR_KRB_INFO),
                              (PVOID*)& pKrbInfo
                              );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateMemory(
                             sizeof(VMDIR_KRB_BLOB)*dwKrbInfoCount,
                             (PVOID*)&pKrbInfo->pKrbBlobs
                             );
    BAIL_ON_VMDIR_ERROR(dwError);

    pKrbInfo->dwCount = dwKrbInfoCount;

    dwError = VmDirAllocASCIILowerToUpper(
                                pszDomainName,
                                &pszUpperCaseDomainName
                                );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocASCIIUpperToLower(
                                pszHostName,
                                &pszLowerCaseHostName
                                );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringPrintf( &pszMachineAccountUPN, "%s@%s", pszLowerCaseHostName, pszUpperCaseDomainName );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirGetKeyTabRecBlob(pszMachineAccountUPN,
                                    &pKrbInfo->pKrbBlobs[0].krbBlob,
                                    &pKrbInfo->pKrbBlobs[0].dwCount
                                    );
    BAIL_ON_VMDIR_ERROR( dwError );

    ppszServiceTable = pszClientServiceTable;

    // Get UPN keys for the service accounts and write to keytab file
    for (dwSrvTabIndex = 0, dwIndex = 1;
         (dwSrvTabIndex < iServiceTableLen) && dwIndex < dwKrbInfoCount;
         ++dwSrvTabIndex, ++ dwIndex
        )
    {
        VMDIR_SAFE_FREE_MEMORY(pszServiceAccountUPN);

        dwError = VmDirAllocateStringPrintf(
                                  &pszServiceAccountUPN,
                                  "%s/%s@%s",
                                  ppszServiceTable[dwSrvTabIndex],
                                  pszLowerCaseHostName,
                                  pszUpperCaseDomainName
                                  );
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirGetKeyTabRecBlob(pszServiceAccountUPN,
                                        &pKrbInfo->pKrbBlobs[dwIndex].krbBlob,
                                        &pKrbInfo->pKrbBlobs[dwIndex].dwCount
                                       );
        BAIL_ON_VMDIR_ERROR( dwError );

    }

    *ppKrbInfo = pKrbInfo;

cleanup:

    VMDIR_SAFE_FREE_MEMORY(pszMachineAccountUPN);
    VMDIR_SAFE_FREE_MEMORY(pszServiceAccountUPN);
    VMDIR_SAFE_FREE_MEMORY(pszUpperCaseDomainName);
    VMDIR_SAFE_FREE_MEMORY(pszLowerCaseHostName);
    return dwError;
error:

    if (ppKrbInfo)
    {
        *ppKrbInfo = NULL;
    }
    if (pKrbInfo)
    {
        VmDirFreeKrbInfo(pKrbInfo);
    }
    goto cleanup;
}


DWORD
VmDirSrvGetComputerAccountInfo(
    PVDIR_CONNECTION pConnection,
    PCSTR            pszDomainName,
    PCSTR            pszComputerHostName,
    PSTR*            ppszComputerDN,
    PSTR*            ppszMachineGUID,
    PSTR*            ppszSiteName
    )
{
    DWORD dwError = 0;
    PSTR pszComputerDN = NULL;
    PSTR pszMachineGUID = NULL;
    PSTR pszSiteName = NULL;

    if (!pConnection || !ppszComputerDN || !ppszMachineGUID || !ppszSiteName)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    dwError = _VmDirSrvGetComputerAccountDN(
                    pConnection,
                    pszDomainName,
                    pszComputerHostName,
                    &pszComputerDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _VmDirSrvGetComputerGuidInternal(
                    pConnection,
                    pszDomainName,
                    pszComputerHostName,
                    &pszMachineGUID);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringA(
                            gVmdirServerGlobals.pszSiteName,
                            &pszSiteName
                            );
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppszComputerDN = pszComputerDN;
    *ppszMachineGUID = pszMachineGUID;
    *ppszSiteName = pszSiteName;

cleanup:

    return dwError;
error:

    if (ppszComputerDN)
    {
        *ppszComputerDN = NULL;
    }
    if (ppszMachineGUID)
    {
        *ppszMachineGUID = NULL;
    }
    if (ppszSiteName)
    {
        *ppszSiteName = NULL;
    }
    VMDIR_SAFE_FREE_STRINGA(pszSiteName);
    VMDIR_SAFE_FREE_STRINGA(pszMachineGUID);
    VMDIR_SAFE_FREE_STRINGA(pszComputerDN);
    goto cleanup;
}

DWORD
VmDirSrvAllocateRpcMachineInfoWFromA(
    PVMDIR_MACHINE_INFO_A pMachineInfo,
    PVMDIR_MACHINE_INFO_W *ppRpcMachineInfo
    )
{
    DWORD dwError = 0;
    PVMDIR_MACHINE_INFO_W pRpcMachineInfoW = NULL;

    if (!pMachineInfo || !ppRpcMachineInfo)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirRpcAllocateMemory(
                              sizeof(VMDIR_MACHINE_INFO_W),
                              (PVOID*)&pRpcMachineInfoW
                              );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirRpcAllocateStringWFromA(
                                  pMachineInfo->pszComputerDN,
                                  &pRpcMachineInfoW->pwszComputerDN
                                  );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirRpcAllocateStringWFromA(
                                  pMachineInfo->pszMachineGUID,
                                  &pRpcMachineInfoW->pwszMachineGUID
                                  );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirRpcAllocateStringWFromA(
                                  pMachineInfo->pszPassword,
                                  &pRpcMachineInfoW->pwszPassword
                                  );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirRpcAllocateStringWFromA(
                                  pMachineInfo->pszSiteName,
                                  &pRpcMachineInfoW->pwszSiteName
                                  );
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppRpcMachineInfo = pRpcMachineInfoW;

cleanup:

    return dwError;
error:

    if (ppRpcMachineInfo)
    {
        *ppRpcMachineInfo = NULL;
    }
    if(pRpcMachineInfoW)
    {
        VmDirSrvRpcFreeMachineInfoW(pRpcMachineInfoW);
    }
    goto cleanup;
}


DWORD
VmDirSrvAllocateRpcMachineInfoAFromW(
    PVMDIR_MACHINE_INFO_W pMachineInfo,
    PVMDIR_MACHINE_INFO_A *ppRpcMachineInfo
    )
{
    DWORD dwError = 0;
    PVMDIR_MACHINE_INFO_A pRpcMachineInfoA = NULL;

    if (!pMachineInfo || !ppRpcMachineInfo)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirRpcAllocateMemory(
                              sizeof(VMDIR_MACHINE_INFO_A),
                              (PVOID*)&pRpcMachineInfoA
                              );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirRpcAllocateStringAFromW(
                                  pMachineInfo->pwszComputerDN,
                                  &pRpcMachineInfoA->pszComputerDN
                                  );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirRpcAllocateStringAFromW(
                                  pMachineInfo->pwszMachineGUID,
                                  &pRpcMachineInfoA->pszMachineGUID
                                  );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirRpcAllocateStringAFromW(
                                  pMachineInfo->pwszPassword,
                                  &pRpcMachineInfoA->pszPassword
                                  );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirRpcAllocateStringAFromW(
                                  pMachineInfo->pwszSiteName,
                                  &pRpcMachineInfoA->pszSiteName
                                  );
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppRpcMachineInfo = pRpcMachineInfoA;

cleanup:

    return dwError;
error:

    if (ppRpcMachineInfo)
    {
        *ppRpcMachineInfo = NULL;
    }
    if(pRpcMachineInfoA)
    {
        VmDirSrvRpcFreeMachineInfoA(pRpcMachineInfoA);
    }
    goto cleanup;
}

DWORD
VmDirSrvAllocateRpcKrbInfo(
    PVMDIR_KRB_INFO  pKrbInfoIn,
    PVMDIR_KRB_INFO* ppRpcKrbInfo
    )
{
    DWORD dwError = 0;
    PVMDIR_KRB_INFO pRpcKrbInfo = NULL;
    DWORD dwIndex = 0;

    if (!pKrbInfoIn || !ppRpcKrbInfo)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirRpcAllocateMemory(
                              sizeof(VMDIR_KRB_INFO),
                              (PVOID*)&pRpcKrbInfo
                              );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirRpcAllocateMemory(
                              sizeof(VMDIR_KRB_BLOB)*pKrbInfoIn->dwCount,
                              (PVOID*)&pRpcKrbInfo->pKrbBlobs
                              );
    BAIL_ON_VMDIR_ERROR(dwError);

    for (; dwIndex < pKrbInfoIn->dwCount; ++dwIndex)
    {
        dwError = VmDirRpcAllocateMemory(
                                  pKrbInfoIn->pKrbBlobs[dwIndex].dwCount,
                                  (PVOID*)&pRpcKrbInfo->pKrbBlobs[dwIndex].krbBlob
                                  );
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirCopyMemory(
                              pRpcKrbInfo->pKrbBlobs[dwIndex].krbBlob,
                              pKrbInfoIn->pKrbBlobs[dwIndex].dwCount,
                              pKrbInfoIn->pKrbBlobs[dwIndex].krbBlob,
                              pKrbInfoIn->pKrbBlobs[dwIndex].dwCount
                              );
        BAIL_ON_VMDIR_ERROR(dwError);

        pRpcKrbInfo->pKrbBlobs[dwIndex].dwCount = pKrbInfoIn->pKrbBlobs[dwIndex].dwCount;
    }

    pRpcKrbInfo->dwCount = dwIndex;

    *ppRpcKrbInfo = pRpcKrbInfo;

cleanup:

    return dwError;
error:

    if (ppRpcKrbInfo)
    {
        *ppRpcKrbInfo = NULL;
    }
    if (pRpcKrbInfo)
    {
        VmDirSrvRpcFreeKrbInfo(pRpcKrbInfo);
    }
    goto cleanup;
}

static
DWORD
_VmDirSrvCreateOUContainer(
    PVDIR_SCHEMA_CTX pSchemaCtx,
    PCSTR            pszContainerDN,
    PCSTR            pszContainerName
    )
{
    DWORD dwError = 0;
    PSTR ppszAttributes[] =
    {
            ATTR_OBJECT_CLASS,              OC_TOP,
            ATTR_OBJECT_CLASS,              OC_ORGANIZATIONAL_UNIT,
            ATTR_OU,                        (PSTR)pszContainerName,
            NULL
    };

    dwError = VmDirSimpleEntryCreate( pSchemaCtx, ppszAttributes, (PSTR)pszContainerDN, 0);
    BAIL_ON_VMDIR_ERROR(dwError);

error:

    return dwError;
}

static
DWORD
_VmDirSrvLDAPAdd(
    PVDIR_CONNECTION pConnection,
    PCSTR pszBaseDN,
    LDAPMod* LDAPModArray[]
    )
{
    DWORD dwError = 0;
    PVDIR_OPERATION pAddOp = NULL;
    DWORD dwIndex = 0;
    PVDIR_ENTRY pEntry = NULL;
    LDAPMod* pCurrentMod = NULL;

    if (!pConnection || IsNullOrEmptyString(pszBaseDN))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirExternalOperationCreate(
                 NULL, -1, LDAP_REQ_ADD, pConnection, &pAddOp);
    BAIL_ON_VMDIR_ERROR(dwError);

    pAddOp->reqDn.lberbv_val = (PSTR)pszBaseDN;
    pAddOp->reqDn.lberbv_len = VmDirStringLenA(pszBaseDN);

    pEntry = pAddOp->request.addReq.pEntry;
    pEntry->allocType = ENTRY_STORAGE_FORMAT_NORMAL;
    dwError = VmDirSchemaCtxCloneIfDifferent(
                                  pAddOp->pSchemaCtx,
                                  &pEntry->pSchemaCtx
                                  );
    BAIL_ON_VMDIR_ERROR(dwError);
    pEntry->dn.lberbv_val = pAddOp->reqDn.lberbv_val;
    pEntry->dn.lberbv_len = pAddOp->reqDn.lberbv_len;

    pCurrentMod = LDAPModArray[0];

    while (pCurrentMod)
    {
        dwError = VmDirEntryAddSingleValueAttribute(
                                              pEntry,
                                              pCurrentMod->mod_type,
                                              pCurrentMod->mod_values[0],
                                              strlen(pCurrentMod->mod_values[0])
                                              );
        BAIL_ON_VMDIR_ERROR(dwError);

        pCurrentMod = LDAPModArray[++dwIndex];
    }

    dwError = VmDirMLAdd(pAddOp);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:

    if (pAddOp)
    {
        VmDirFreeOperation(pAddOp);
    }
    return dwError;
error:

    goto cleanup;
}

static
DWORD
_VmDirSrvLDAPModify(
    PVDIR_CONNECTION pConnection,
    PCSTR pszBaseDN,
    LDAPMod* pLDAPMod
    )
{
    DWORD dwError = 0;
    PVDIR_OPERATION pModifyOp = NULL;
    if (!pConnection || IsNullOrEmptyString(pszBaseDN))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirExternalOperationCreate(
                NULL, -1, LDAP_REQ_MODIFY, pConnection, & pModifyOp);
    BAIL_ON_VMDIR_ERROR(dwError);

    pModifyOp->pBEIF = VmDirBackendSelect(NULL);
    pModifyOp->reqDn.lberbv_val = (PSTR)pszBaseDN;
    pModifyOp->reqDn.lberbv_len = VmDirStringLenA(pszBaseDN);

    pModifyOp->request.modifyReq.dn.lberbv_val = pModifyOp->reqDn.lberbv_val;
    pModifyOp->request.modifyReq.dn.lberbv_len = pModifyOp->reqDn.lberbv_len;

    dwError = VmDirAppendAMod(
                            pModifyOp,
                            pLDAPMod->mod_op,
                            pLDAPMod->mod_type,
                            strlen(pLDAPMod->mod_type),
                            pLDAPMod->mod_values[0],
                            strlen(pLDAPMod->mod_values[0])
                            );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirMLModify(pModifyOp);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:

    if (pModifyOp)
    {
        VmDirFreeOperation(pModifyOp);
    }
    return dwError;
error:

    goto cleanup;
}

static
DWORD
_VmDirSrvLDAPSearch(
    PVDIR_CONNECTION pConnection,
    PCSTR pszBaseDN,
    int searchScope,
    PCSTR pszFilter,
    PCSTR ppszAttributes[],
    DWORD dwAttrCount,
    PVDIR_ENTRY_ARRAY* ppEntryArray
    )
{
    DWORD dwError = 0;
    PVDIR_OPERATION pSearchOp = NULL;
    PVDIR_FILTER pFilter = NULL;
    PVDIR_BERVALUE pbvAttrs = NULL;
    DWORD dwIndex = 0;
    PVDIR_ENTRY_ARRAY pEntryArray = NULL;

    if (!pConnection || !ppEntryArray)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirExternalOperationCreate(
                                        NULL,
                                        -1,
                                        LDAP_REQ_SEARCH,
                                        pConnection,
                                        &pSearchOp
                                        );
    BAIL_ON_VMDIR_ERROR(dwError);

    if (IsNullOrEmptyString(pszFilter))
    {
        dwError = StrFilterToFilter(
                            "(objectclass=*)",
                            &pFilter
                            );
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    else
    {
        dwError = StrFilterToFilter(pszFilter, &pFilter);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (dwAttrCount)
    {
        dwError = VmDirAllocateMemory(
                                  sizeof(VDIR_BERVALUE)*(dwAttrCount+1),
                                  (PVOID*)&pbvAttrs
                                  );
        BAIL_ON_VMDIR_ERROR(dwError);

        for (; dwIndex < dwAttrCount; ++dwIndex)
        {
            dwError = VmDirStringToBervalContent(
                                            ppszAttributes[dwIndex],
                                            &pbvAttrs[dwIndex]
                                            );
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }

    dwError = VmDirStringToBervalContent(
                                      pszBaseDN,
                                      &pSearchOp->reqDn
                                      );
    BAIL_ON_VMDIR_ERROR(dwError);

    pSearchOp->request.searchReq.scope = searchScope;
    pSearchOp->request.searchReq.filter = pFilter;
    pFilter = NULL;
    pSearchOp->request.searchReq.attrs = pbvAttrs;
    pbvAttrs = NULL;
    pSearchOp->request.searchReq.bStoreRsltInMem = TRUE;

    dwError = VmDirMLSearch(pSearchOp);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateMemory(
                              sizeof(VDIR_ENTRY_ARRAY),
                              (PVOID*)&pEntryArray
                              );
    BAIL_ON_VMDIR_ERROR(dwError);

    pEntryArray->iSize = pSearchOp->internalSearchEntryArray.iSize;
    pEntryArray->pEntry = pSearchOp->internalSearchEntryArray.pEntry;
    pSearchOp->internalSearchEntryArray.iSize = 0;
    pSearchOp->internalSearchEntryArray.pEntry = NULL;

    *ppEntryArray = pEntryArray;

cleanup:

    if (pSearchOp)
    {
        VmDirFreeOperation(pSearchOp);
    }
    if (pFilter)
    {
        DeleteFilter(pFilter);
    }
    VMDIR_SAFE_FREE_MEMORY(pbvAttrs);
    return dwError;
error:

    if (ppEntryArray)
    {
       *ppEntryArray = NULL;
    }
    if (pEntryArray)
    {
        VmDirFreeEntryArray(pEntryArray);
    }
    goto cleanup;
}

static
DWORD
_VmDirSrvLDAPModReplaceAttribute(
    PVDIR_CONNECTION pConnection,
    PCSTR pszBaseDN,
    PCSTR pszAttribute,
    PCSTR pszAttrValue
    )
{
    DWORD       dwError = 0;
    LDAPMod     mod = {0};
    PSTR        vals[2] = {(PSTR)pszAttrValue, NULL};

    mod.mod_op = LDAP_MOD_REPLACE;
    mod.mod_type = (PSTR)pszAttribute;
    mod.mod_vals.modv_strvals = vals;

    dwError = _VmDirSrvLDAPModify(
                            pConnection,
                            pszBaseDN,
                            &mod
                            );
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:

    return dwError;
error:

    goto cleanup;
}

static
DWORD
_VmDirSrvLDAPQueryAttrValues(
    PVDIR_ENTRY         pEntry,
    PCSTR               pszAttribute,
    PVMDIR_STRING_LIST  *ppAttributeList
    )
{
    DWORD dwError = 0;
    PVMDIR_STRING_LIST pAttributeList = NULL;
    BOOLEAN bReturn = FALSE;
    BOOLEAN bAsterisk = FALSE;
    BOOLEAN bPlusSign = FALSE;
    PVDIR_ATTRIBUTE pAttr = NULL;
    PVDIR_ATTRIBUTE pAttrs[3] = {0};
    DWORD dwIIndex = 0;
    DWORD dwJIndex = 0;
    PSTR pszValue = NULL;

    if (!pEntry || !ppAttributeList)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirStringListInitialize(&pAttributeList, 0);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (!VmDirStringCompareA("*", pszAttribute, TRUE))
    {
        bAsterisk = TRUE;
    }
    else if (!VmDirStringCompareA("+", pszAttribute, TRUE))
    {
        bPlusSign = TRUE;
    }

    pAttrs[0] = pEntry->attrs;
    pAttrs[1] = pEntry->pComputedAttrs;

    for (; pAttrs[dwIIndex]; ++dwIIndex)
    {
        for (pAttr = pAttrs[dwIIndex]; pAttr; pAttr = pAttr->next)
        {
            bReturn = FALSE;

            if ((bAsterisk || IsNullOrEmptyString(pszAttribute)) &&
                pAttr->pATDesc->usage == VDIR_LDAP_DIRECTORY_OPERATION_ATTRIBUTE
               )
            {
                bReturn = TRUE;
            }
            else if (bPlusSign &&
                    pAttr->pATDesc->usage ==
                            VDIR_LDAP_DIRECTORY_OPERATION_ATTRIBUTE)
            {
                bReturn = TRUE;
            }
            else if (!IsNullOrEmptyString(pszAttribute))
            {
                if (VmDirStringCompareA(
                            pAttr->type.lberbv.bv_val,
                            pszAttribute,
                            FALSE) == 0)
                {
                    bReturn = TRUE;
                    break;
                }
            }
        }
        if (bReturn)
        {
            for (dwJIndex = 0; dwJIndex < pAttr->numVals; ++dwJIndex)
            {
                {
                    dwError = VmDirAllocateStringA(
                                          pAttr->vals[dwJIndex].lberbv_val,
                                          &pszValue
                                          );
                    BAIL_ON_VMDIR_ERROR(dwError);

                }

                dwError = VmDirStringListAdd(pAttributeList, pszValue);
                BAIL_ON_VMDIR_ERROR(dwError);
                pszValue = NULL;
            }
        }
    }

    *ppAttributeList = pAttributeList;

cleanup:

    VMDIR_SAFE_FREE_MEMORY(pszValue);
    return dwError;
error:

    if (ppAttributeList)
    {
        *ppAttributeList = NULL;
    }
    if (pAttributeList)
    {
        VmDirStringListFree(pAttributeList);
    }
    goto cleanup;
}


#ifndef LIGHTWAVE_BUILD
static
DWORD
_VmDirSrvLDAPSetupAccountMembership(
    PVDIR_CONNECTION   pConnection,
    PCSTR   pszDomainDN,
    PCSTR   pszBuiltinGroupName,
    PCSTR   pszAccountDN
    )
{
    DWORD       dwError = 0;
    LDAPMod     mod = {0};
    PSTR        vals[2] = {(PSTR)pszAccountDN, NULL};
    PSTR        pszGroupDN = NULL;

    // set DomainControllerGroupDN
    dwError = VmDirAllocateStringPrintf( &pszGroupDN,
                                             "cn=%s,cn=%s,%s",
                                             pszBuiltinGroupName,
                                             VMDIR_BUILTIN_CONTAINER_NAME,
                                             pszDomainDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    mod.mod_op = LDAP_MOD_ADD;
    mod.mod_type = (PSTR)ATTR_MEMBER;
    mod.mod_vals.modv_strvals = vals;

    dwError = _VmDirSrvLDAPModify(
                            pConnection,
                            pszGroupDN,
                            &mod
                            );
    if ( dwError == LDAP_TYPE_OR_VALUE_EXISTS )
    {
        dwError = 0;    // already a member of group
    }
    BAIL_ON_VMDIR_ERROR(dwError);


cleanup:

    VMDIR_SAFE_FREE_STRINGA(pszGroupDN);
    return dwError;
error:

    goto cleanup;
}
#endif

static
DWORD
_VmDirSrvConvertUPNToDN(
     PVDIR_CONNECTION pConnection,
     PCSTR      pszUPN,
     PSTR*      ppszOutDN
     )
{
    DWORD       dwError = 0;
    PSTR            pszFilter = NULL;
    PSTR            pszOutDN = NULL;
    int             iCount = 0;
    PVDIR_ENTRY_ARRAY pEntryArray = NULL;

    if ( !pConnection || !pszUPN || !ppszOutDN )
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    dwError = VmDirAllocateStringPrintf(
                    &pszFilter, "%s=%s",
                    ATTR_KRB_UPN,
                    pszUPN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _VmDirSrvLDAPSearch(
                                pConnection,
                                "",
                                LDAP_SCOPE_SUBTREE,
                                pszFilter,
                                NULL,
                                0,
                                &pEntryArray
                                );
    BAIL_ON_VMDIR_ERROR(dwError);

    iCount = pEntryArray->iSize;

    //GetDN
    // should have either 0 or 1 result
    if (iCount > 1 )
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_STATE);
    }
    else if (iCount == 0)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_ENTRY_NOT_FOUND);
    }

    dwError = VmDirAllocateStringA(
                              pEntryArray->pEntry[0].dn.lberbv.bv_val,
                              &pszOutDN
                              );
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppszOutDN = pszOutDN;
cleanup:

    VMDIR_SAFE_FREE_MEMORY(pszFilter);
    if (pEntryArray)
    {
        VmDirFreeEntryArray(pEntryArray);
    }
    return dwError;
error:

    if (ppszOutDN)
    {
        *ppszOutDN = NULL;
    }
    VMDIR_SAFE_FREE_STRINGA(pszOutDN);
    goto cleanup;
}

static
DWORD
_VmDirSrvGetComputerAccountDN(
    PVDIR_CONNECTION pConnection,
    PCSTR pszDomain,
    PCSTR pszMachineName,
    PSTR* ppszAccountDN
    )
{
    DWORD dwError = 0;
    PSTR  pszAccountDN = NULL;
    PSTR  pszUPN = NULL;

    if (!pConnection ||
        IsNullOrEmptyString(pszDomain) ||
        IsNullOrEmptyString(pszMachineName) ||
        !ppszAccountDN)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateStringPrintf(
                    &pszUPN,
                    "%s@%s",
                    pszMachineName,
                    pszDomain);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _VmDirSrvConvertUPNToDN(
                    pConnection,
                    pszUPN,
                    &pszAccountDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppszAccountDN = pszAccountDN;

cleanup:

    VMDIR_SAFE_FREE_MEMORY(pszUPN);
    return dwError;
error:

    if (ppszAccountDN)
    {
        *ppszAccountDN = NULL;
    }
    VMDIR_SAFE_FREE_MEMORY(pszAccountDN);
    goto cleanup;
}

static
DWORD
_VmDirSrvGetComputerGuidInternal(
    PVDIR_CONNECTION pConnection,
    PCSTR pszDomain,
    PCSTR pszMachineName,
    PSTR* ppszGUID
    )
{
    DWORD  dwError = 0;
    PSTR   pszFilter = "(objectclass=computer)";
    PSTR   pszAccountDN = NULL;
    PSTR   pszAttrMachineGUID = ATTR_VMW_MACHINE_GUID;
    PCSTR  ppszAttrs[] = { (PCSTR)pszAttrMachineGUID, NULL };
//    VDIR_ENTRY_ARRAY entryArray = {0};
    PVDIR_ENTRY_ARRAY pEntryArray = NULL;
    VDIR_ENTRY pEntry = {0};
    PSTR  pszGUID = NULL;
    PVMDIR_STRING_LIST pAttributeList = NULL;
    int iCount = 0;

    dwError = _VmDirSrvGetComputerAccountDN(
                                  pConnection,
                                  pszDomain,
                                  pszMachineName,
                                  &pszAccountDN
                                  );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _VmDirSrvLDAPSearch(
                    pConnection,
                    pszAccountDN,
                    LDAP_SCOPE_BASE,
                    pszFilter,
                    ppszAttrs,
                    1,
                    &pEntryArray
                    );
    BAIL_ON_VMDIR_ERROR(dwError);

    iCount = pEntryArray->iSize;
    //searching by name should yield just one
    if (iCount != 1)
    {
        dwError = ERROR_INVALID_STATE;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    pEntry = pEntryArray->pEntry[0];

    dwError = _VmDirSrvLDAPQueryAttrValues(
                                      &pEntry,
                                      pszAttrMachineGUID,
                                      &pAttributeList
                                      );
    BAIL_ON_VMDIR_ERROR(dwError);

    if (!pAttributeList || pAttributeList->dwCount != 1)
    {
        dwError = ERROR_INVALID_STATE;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateStringPrintf(
                        &pszGUID,
                        "%s",
                        pAttributeList->pStringList[0]
                        );
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppszGUID = pszGUID;

cleanup:

    if (pEntryArray)
    {
        VmDirFreeEntryArray(pEntryArray);
    }
    if (pAttributeList)
    {
        VmDirStringListFree(pAttributeList);
    }
    VMDIR_SAFE_FREE_MEMORY(pszAccountDN);
    return dwError;
error:

    if (ppszGUID)
    {
        *ppszGUID = NULL;
    }
    VMDIR_SAFE_FREE_MEMORY(pszGUID);
    goto cleanup;
}

