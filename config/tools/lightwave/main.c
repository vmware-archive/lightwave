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
VOID
ShowUsage(
    VOID
    );

static
int
LightwaveCommand(
    int argc,
    char *argv[]
    );

int
main(
    int argc,
    char* argv[])
{
    int iRetCode = 0;

    if (argc < 2)
    {
        iRetCode = 1;
        ShowUsage();
    }
    else
    {
        iRetCode = LightwaveCommand(argc-1, &argv[1]);
    }

    return iRetCode;
}

static
int
LightwaveCommand(
    int argc,
    char *argv[])
{
    int iRetCode = 0;

    if (argc < 2)
    {
        ShowUsage();
        iRetCode = 1;
        goto cleanup;
    }

    if (!strcmp(argv[0], "domain"))
    {
        iRetCode = LightwaveDomain(argc-1, &argv[1]);
    }
    else if (!strcmp(argv[0], "dns"))
    {
        iRetCode = LightwaveDns(argc-1, &argv[1]);
    }
    else if (!strcmp(argv[0], "ca"))
    {
        iRetCode = LightwaveCa(argc-1, &argv[1]);
    }
    else
    {
        ShowUsage();
        iRetCode = 1;
    }

cleanup:
    return iRetCode;
}

static
VOID
ShowUsage(
    VOID
    )
{
    char pszUsageText[] =
        "Usage: lightwave COMMAND {arguments}\n"
        "\n"
        "Commands:\n"
        "    domain   promote, demote, join, or leave a Lightwave domain\n"
        "    dns      delete\n"
        "    ca       get-cert"
        "\n"
        "Run 'lightwave COMMAND --help' for more information on a particular command'\n";

    printf("%s", pszUsageText);
}
