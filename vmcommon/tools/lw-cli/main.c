/*
 * Copyright � 2019  VMware, Inc.  All Rights Reserved.
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
LW_CLICommand(
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
        iRetCode = LW_CLICommand(argc-1, &argv[1]);
    }

    return iRetCode;
}

static
int
LW_CLICommand(
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

    if (!strcmp(argv[0], "regcfg"))
    {
        iRetCode = CLIRegCfg(argc-1, &argv[1]);
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
        "Usage: lw-cli COMMAND {arguments}\n"
        "\n"
        "Commands:\n"
        "    regcfg   \n"
        "\n"
        "Run 'lw-cli COMMAND --help' for more information on a particular command'\n";

    printf("%s", pszUsageText);
}
