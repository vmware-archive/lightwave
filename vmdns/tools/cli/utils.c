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
    if (pContext->pszServer)
    {
        VmDnsFreeStringA(pContext->pszServer);
    }
    if (pContext->pServerContext)
    {
        VmDnsCloseServer(pContext->pServerContext);
    }

    VmDnsClearRecord(&pContext->record);
    VmDnsFreeMemory(pContext);
}

#ifdef _WIN32
VOID
VmDnsReadString(
    PCSTR szPrompt,
    PSTR szString,
    int len,
    BOOLEAN bHideString
    )
{
    DWORD oldMode = 0;
    HANDLE hConIn = INVALID_HANDLE_VALUE;
    PSTR pszNl = NULL;

    if (bHideString)
    {
        hConIn = GetStdHandle(STD_INPUT_HANDLE);
        if (hConIn != INVALID_HANDLE_VALUE)
        {
            if (GetConsoleMode(hConIn, &oldMode))
            {
                SetConsoleMode(hConIn, oldMode & ~ENABLE_ECHO_INPUT);
            }
        }
    }

    if (szPrompt)
    {
        fputs(szPrompt, stderr);
        fflush(stderr);
    }

    if (fgets(szString, len, stdin) == NULL)
    {
        szString[0] = '\0';
    }
    else
    {
        pszNl = VmDnsStringChrA(szString, '\n');
        if (pszNl)
        {
            *pszNl = '\0';
        }
    }

    if (bHideString)
    {
        fputs("\n", stderr);
        SetConsoleMode(hConIn, oldMode);
    }
    fflush(stderr);
}
#else
VOID
VmDnsReadString(
    PCSTR szPrompt,
    PSTR szString,
    int len,
    BOOLEAN bHideString
    )
{
    sigset_t sig, osig;
    struct termios ts, ots;
    PSTR pszNl = NULL;

    if (bHideString)
    {
        sigemptyset(&sig);
        sigaddset(&sig, SIGINT);
        sigaddset(&sig, SIGTSTP);
        sigprocmask(SIG_BLOCK, &sig, &osig);

        tcgetattr(fileno(stdin), &ts);
        ots = ts;
        ts.c_lflag &= ~(ECHO);
        tcsetattr(fileno(stdin), TCSANOW, &ts);
    }

    if (szPrompt)
    {
        fputs(szPrompt, stderr);
        fflush(stderr);
    }

    if (fgets(szString, len, stdin) == NULL)
    {
        szString[0] = '\0';
    }
    else
    {
        pszNl = VmDnsStringChrA(szString, '\n');
        if (pszNl)
        {
            *pszNl = '\0';
        }
    }

    if (bHideString)
    {
        fputs("\n", stderr);

        tcsetattr(fileno(stdin), TCSANOW, &ots);
        sigprocmask(SIG_SETMASK, &osig, NULL);
    }
    fflush(stderr);
}
#endif
