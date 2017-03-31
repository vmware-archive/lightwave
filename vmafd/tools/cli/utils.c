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

VOID
VmAfdCliFreeContext(
    PVM_AFD_CLI_CONTEXT pContext
    )
{
    if (pContext->pszServerName)
    {
        VmAfdFreeStringA(pContext->pszServerName);
    }
    if (pContext->pszUserName)
    {
        VmAfdFreeStringA(pContext->pszUserName);
    }
    if (pContext->pszPassword)
    {
        VmAfdFreeStringA(pContext->pszPassword);
    }
    if (pContext->pszMachineName)
    {
        VmAfdFreeStringA(pContext->pszMachineName);
    }
    if (pContext->pszDomainName)
    {
        VmAfdFreeStringA(pContext->pszDomainName);
    }
    if (pContext->pszOrgUnit)
    {
        VmAfdFreeStringA(pContext->pszOrgUnit);
    }
    if (pContext->pszPartnerName)
    {
        VmAfdFreeStringA(pContext->pszPartnerName);
    }
    if (pContext->pszSiteName)
    {
        VmAfdFreeStringA(pContext->pszSiteName);
    }
    if (pContext->pszMachineId)
    {
        VmAfdFreeStringA(pContext->pszMachineId);
    }
    if (pContext->pszSiteGUID)
    {
        VmAfdFreeStringA(pContext->pszSiteGUID);
    }
     if (pContext->pszPNID)
    {
        VmAfdFreeStringA(pContext->pszPNID);
    }
   VmAfdFreeMemory(pContext);
}
