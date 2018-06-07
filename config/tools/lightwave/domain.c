/*
 * Copyright © 2012-2018 VMware, Inc.  All Rights Reserved.
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
ShowUsage(
    VOID
    );

int
LightwaveDomain(
    int argc,
    char* argv[])
{
    int iRetCode = 0;

    if (!strcmp(argv[0], "promote"))
    {
        iRetCode = LightwaveDomainPromote(argc-1, &argv[1]);
    }
    else if (!strcmp(argv[0], "demote"))
    {
        iRetCode = LightwaveDomainDemote(argc-1, &argv[1]);
    }
    else if (!strcmp(argv[0], "join"))
    {
        iRetCode = LightwaveDomainJoin(argc-1, &argv[1]);
    }
    else if (!strcmp(argv[0], "leave"))
    {
        iRetCode = LightwaveDomainLeave(argc-1, &argv[1]);
    }
    else
    {
        ShowUsage();
    }

    return iRetCode;
}

static
VOID
ShowUsage(
    VOID
    )
{
    char pszUsageText[] =
        "Usage: lightwave domain COMMAND {arguments}\n"
        "\n"
        "Commands:\n"
        "    promote  promote a Lightwave domain controller\n"
        "    demote   demote a Lightwave domain controller\n"
        "    join     join a client machine to a Lightwave domain\n"
        "    leave    leave a client machine from a Lightwave domain\n"
        "\n"
        "Run 'lightwave domain COMMAND --help' for more information on a particular command'\n";

    printf("%s", pszUsageText);
}
