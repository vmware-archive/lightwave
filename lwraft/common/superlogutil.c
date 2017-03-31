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
VOID
_FreeSearchInformation(
        PVMDIR_SUPERLOG_ENTRY_LDAPOPERATION pRpcEntry)
{
    if (pRpcEntry)
    {
        VmDirRpcClientFreeMemory(pRpcEntry->opInfo.searchInfo.pwszAttributes);
        pRpcEntry->opInfo.searchInfo.pwszAttributes = NULL;
        VmDirRpcClientFreeMemory(pRpcEntry->opInfo.searchInfo.pwszBaseDN);
        pRpcEntry->opInfo.searchInfo.pwszBaseDN = NULL;
        VmDirRpcClientFreeMemory(pRpcEntry->opInfo.searchInfo.pwszScope);
        pRpcEntry->opInfo.searchInfo.pwszScope = NULL;
        VmDirRpcClientFreeMemory(pRpcEntry->opInfo.searchInfo.pwszIndexResults);
        pRpcEntry->opInfo.searchInfo.pwszIndexResults = NULL;
    }
}

VOID
VmDirRpcFreeSuperLogEntryLdapOperationArray(
        PVMDIR_SUPERLOG_ENTRY_LDAPOPERATION_ARRAY pRpcEntries
        )
{
    unsigned int i;
    if (pRpcEntries)
    {
        for (i = 0; i < pRpcEntries->dwCount; i++)
        {
            VmDirRpcClientFreeMemory(pRpcEntries->entries[i].pwszLoginDN);
            pRpcEntries->entries[i].pwszLoginDN = NULL;
            VmDirRpcClientFreeMemory(pRpcEntries->entries[i].pwszClientIP);
            pRpcEntries->entries[i].pwszClientIP = NULL;
            VmDirRpcClientFreeMemory(pRpcEntries->entries[i].pwszServerIP);
            pRpcEntries->entries[i].pwszServerIP = NULL;
            VmDirRpcClientFreeMemory(pRpcEntries->entries[i].pwszOperation);
            pRpcEntries->entries[i].pwszOperation = NULL;
            VmDirRpcClientFreeMemory(pRpcEntries->entries[i].pwszString);
            pRpcEntries->entries[i].pwszString = NULL;
            switch (pRpcEntries->entries[i].opType)
            {
            case LDAP_REQ_SEARCH:
                _FreeSearchInformation(&pRpcEntries->entries[i]);
                break;
            default:
                break;
            }
        }
        VmDirRpcClientFreeMemory(pRpcEntries->entries);
        pRpcEntries->entries = NULL;
        VmDirRpcClientFreeMemory(pRpcEntries);
        pRpcEntries = NULL;
    }
}

