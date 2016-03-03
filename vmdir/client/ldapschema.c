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

#include "includes.h"

static
DWORD
_VmDirGetPartnerSchema(
    PVMDIR_CONNECTION          pConnection,
    PVMDIR_LDAP_SCHEMA_STRUCT* ppLdapSchema
    );

static
DWORD
_VmDirGetModStrVals(
    PVMDIR_STRING_LIST  pStrList,
    PSTR**              pppszModStr
    );

static
VOID
_VmDirLogSchemaMod(
    LDAPMod*    pMod
    );

static
DWORD
_VmDirUpgradePartnerSchema(
    PVMDIR_CONNECTION           pConnection,
    PVMDIR_LDAP_SCHEMA_MOD_STR  pModStr,
    BOOLEAN                     bDryRun
    );

/*
 * Newer version of schema is a super set of prior definition.
 * We allow following schema changes in new version:
 * 1. add new attributetypes
 * 2. add new objectclases
 * 3. add attribute to objectclasses MAY list
 *        - merge new value into existing MAY list
 * 4. add attribute to contentrule MAY list
 *        - merge new value into existing MAY list
 * 5. add objectclass to contentrule AUX list
 *        - merge new value into existing AUX list
 */
DWORD
VmDirSchemaUpgradeInternal(
    PVMDIR_CONNECTION   pConnection,
    PCSTR               pszSchemaFile,
    BOOLEAN             bDryRun,
    PSTR*               ppszErrMsg
    )
{
    DWORD   dwError = 0;
    PVMDIR_LDAP_SCHEMA_STRUCT  pFileSchema = NULL;
    PVMDIR_LDAP_SCHEMA_STRUCT  pPartnerSchema = NULL;
    VMDIR_LDAP_SCHEMA_MOD_STR  ModStr = {0};

    if (pConnection==NULL || pszSchemaFile==NULL || ppszErrMsg == NULL)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirInitModStrContent(&ModStr);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirGetSchemaFromLocalFile( pszSchemaFile, &pFileSchema);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _VmDirGetPartnerSchema(pConnection, &pPartnerSchema);
    BAIL_ON_VMDIR_ERROR(dwError);

    // merge new schema changes into existing definition.
    // Note, it does not perform full semantic verification, which is done at server side during ldap modify.
    dwError = VmDirAnalyzeSchemaUpgrade(pPartnerSchema, pFileSchema, &ModStr);
    BAIL_ON_VMDIR_ERROR(dwError);

    // upgrade partner schema if necessary.
    dwError = _VmDirUpgradePartnerSchema(pConnection, &ModStr, bDryRun);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VmDirFreeModStrContent(&ModStr);
    VmDirFreeLdapSchemaStruct(pFileSchema);
    VmDirFreeLdapSchemaStruct(pPartnerSchema);

    return dwError;
error:
    goto    cleanup;
}

static
DWORD
_VmDirGetPartnerSchema(
    PVMDIR_CONNECTION          pConnection,
    PVMDIR_LDAP_SCHEMA_STRUCT* ppLdapSchema
    )
{
    DWORD                       dwError = 0;
    PVMDIR_LDAP_SCHEMA_STRUCT   pSchemaStruct = NULL;

    dwError = VmDirGetSchemaFromPartner(pConnection->pLd, &pSchemaStruct);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppLdapSchema = pSchemaStruct;

cleanup:
    return dwError;

error:
    VmDirFreeLdapSchemaStruct(pSchemaStruct);
    goto    cleanup;
}

/*
 * ModStr is NULL terminated char**.
 * *pppszModStr does not own individual **ppzModStr.
 */
static
DWORD
_VmDirGetModStrVals(
    PVMDIR_STRING_LIST  pStrList,
    PSTR**              pppszModStr
    )
{
    DWORD   dwError = 0;
    PSTR*   ppszModStr = NULL;
    size_t  iBufSize = sizeof(*ppszModStr)*(pStrList->dwCount+1);

    dwError = VmDirAllocateMemory(iBufSize, (PVOID*)&ppszModStr);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirCopyMemory(
                ppszModStr,
                iBufSize,
                (PCVOID) pStrList->pStringList,
                sizeof(*ppszModStr)*(pStrList->dwCount));
    BAIL_ON_VMDIR_ERROR(dwError);

    *pppszModStr = ppszModStr;

cleanup:
    return dwError;

error:
    VMDIR_SAFE_FREE_MEMORY(ppszModStr);
    goto cleanup;
}

static
VOID
_VmDirLogSchemaMod(
    LDAPMod*    pMod
    )
{
    int i = 0;

    VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "Modify partner schema entry:");
    VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "OP type %d (ADD/0,DEL/1), %s", pMod->mod_op ,pMod->mod_type);
    for (i=0; pMod->mod_vals.modv_strvals[i]; i++)
    {
        VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "%s", pMod->mod_vals.modv_strvals[i]);
    }

    return;
}

/*
 * construct ldap mods
 * update partner schema entry
 */
static
DWORD
_VmDirUpgradePartnerSchema(
    PVMDIR_CONNECTION           pConnection,
    PVMDIR_LDAP_SCHEMA_MOD_STR  pModStr,
    BOOLEAN                     bDryRun
    )
{
    DWORD       dwError = 0;
    int         iCnt = 0;
    LDAPMod*    mods[7] = {0};
    LDAPMod     modatadd = {0};
    LDAPMod     modocadd = {0};
    LDAPMod     modcradd = {0};
    LDAPMod     modatdel = {0};
    LDAPMod     modocdel = {0};
    LDAPMod     modcrdel = {0};
    PSTR*       ppATModDelStr = NULL;
    PSTR*       ppATModAddStr = NULL;
    PSTR*       ppOCModAddStr = NULL;
    PSTR*       ppOCModDelStr = NULL;
    PSTR*       ppCRModAddStr = NULL;
    PSTR*       ppCRModDelStr = NULL;

    if (pModStr->pDelATStrList->dwCount > 0)
    {
        dwError = _VmDirGetModStrVals(
                    pModStr->pDelATStrList,
                    &ppATModDelStr);
        BAIL_ON_VMDIR_ERROR(dwError);

        modatdel.mod_op = LDAP_MOD_DELETE;
        modatdel.mod_type = (PSTR)ATTR_ATTRIBUTETYPES;
        modatdel.mod_vals.modv_strvals = ppATModDelStr;
        _VmDirLogSchemaMod(&modatdel);

        mods[iCnt] = &modatdel;
        iCnt++;
    }

    if (pModStr->pAddATStrList->dwCount > 0)
    {
        dwError = _VmDirGetModStrVals(
                    pModStr->pAddATStrList,
                    &ppATModAddStr);
        BAIL_ON_VMDIR_ERROR(dwError);

        modatadd.mod_op = LDAP_MOD_ADD;
        modatadd.mod_type = (PSTR)ATTR_ATTRIBUTETYPES;
        modatadd.mod_vals.modv_strvals = ppATModAddStr;
        _VmDirLogSchemaMod(&modatadd);

        mods[iCnt] = &modatadd;
        iCnt++;
    }

    if (pModStr->pDelOCStrList->dwCount > 0)
    {
        dwError = _VmDirGetModStrVals(
                    pModStr->pDelOCStrList,
                    &ppOCModDelStr);
        BAIL_ON_VMDIR_ERROR(dwError);

        modocdel.mod_op = LDAP_MOD_DELETE;
        modocdel.mod_type = (PSTR)ATTR_OBJECTCLASSES;
        modocdel.mod_vals.modv_strvals = ppOCModDelStr;
        _VmDirLogSchemaMod(&modocdel);

        mods[iCnt] = &modocdel;
        iCnt++;
    }

    if (pModStr->pAddOCStrList->dwCount > 0)
    {
        dwError = _VmDirGetModStrVals(
                    pModStr->pAddOCStrList,
                    &ppOCModAddStr);
        BAIL_ON_VMDIR_ERROR(dwError);

        modocadd.mod_op = LDAP_MOD_ADD;
        modocadd.mod_type = (PSTR)ATTR_OBJECTCLASSES;
        modocadd.mod_vals.modv_strvals = ppOCModAddStr;
        _VmDirLogSchemaMod(&modocadd);

        mods[iCnt] = &modocadd;
        iCnt++;
    }

    if (pModStr->pDelCRStrList->dwCount > 0)
    {
        dwError = _VmDirGetModStrVals(
                    pModStr->pDelCRStrList,
                    &ppCRModDelStr);
        BAIL_ON_VMDIR_ERROR(dwError);

        modcrdel.mod_op = LDAP_MOD_DELETE;
        modcrdel.mod_type = (PSTR)ATTR_DITCONTENTRULES;
        modcrdel.mod_vals.modv_strvals = ppCRModDelStr;
        _VmDirLogSchemaMod(&modcrdel);

        mods[iCnt] = &modcrdel;
        iCnt++;
    }

    if (pModStr->pAddCRStrList->dwCount > 0)
    {
        dwError = _VmDirGetModStrVals(
                    pModStr->pAddCRStrList,
                    &ppCRModAddStr);
        BAIL_ON_VMDIR_ERROR(dwError);

        modcradd.mod_op = LDAP_MOD_ADD;
        modcradd.mod_type = (PSTR)ATTR_DITCONTENTRULES;
        modcradd.mod_vals.modv_strvals = ppCRModAddStr;
        _VmDirLogSchemaMod(&modcradd);

        mods[iCnt] = &modcradd;
        iCnt++;
    }

    if (iCnt>0 && bDryRun==FALSE)
    {
        dwError = ldap_modify_ext_s(
                    pConnection->pLd,
                    SUB_SCHEMA_SUB_ENTRY_DN,
                    mods,
                    NULL,
                    NULL);
        BAIL_ON_VMDIR_ERROR(dwError);

        VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "[%s][%d] Update partner schema.",__FUNCTION__,__LINE__);
    }
    else
    {
        VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "[%s][%d] Update partner schema pass through.  Mod change (%d), Dryrun(%d)",
                       __FUNCTION__,__LINE__, iCnt, bDryRun);
    }

cleanup:
    VMDIR_SAFE_FREE_MEMORY(ppATModAddStr);
    VMDIR_SAFE_FREE_MEMORY(ppOCModAddStr);
    VMDIR_SAFE_FREE_MEMORY(ppCRModAddStr);
    VMDIR_SAFE_FREE_MEMORY(ppATModDelStr);
    VMDIR_SAFE_FREE_MEMORY(ppOCModDelStr);
    VMDIR_SAFE_FREE_MEMORY(ppCRModDelStr);

    return dwError;
error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "[%s][%d] failed error=%d",__FUNCTION__,__LINE__,dwError);
    goto cleanup;
}
