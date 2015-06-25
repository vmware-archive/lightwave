/*
 * Copyright (c) VmDnsare Inc.  All rights Reserved.
 */

#include "includes.h"

VOID
VmDnsCliFreeContext(
    PVM_DNS_CLI_CONTEXT pContext
    )
{
    if (pContext->pszZone)
    {
        VmDnsFreeStringA(pContext->pszZone);
    }
    if (pContext->pszNSHost)
    {
        VmDnsFreeStringA(pContext->pszNSHost);
    }
    if (pContext->pszNSIp)
    {
        VmDnsFreeStringA(pContext->pszNSIp);
    }
    if (pContext->pszMboxDomain)
    {
        VmDnsFreeStringA(pContext->pszMboxDomain);
    }
    if (pContext->pszForwarder)
    {
        VmDnsFreeStringA(pContext->pszForwarder);
    }
    if (pContext->pServerContext)
    {
        VmDnsCloseServer(pContext->pServerContext);
    }

    VmDnsClearRecord(&pContext->record);
    VmDnsFreeMemory(pContext);
}
