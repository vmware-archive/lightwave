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


int ldap_syslog = 0;
int slap_debug = 0;

void printBufInHex(unsigned char *buf, int len)
{
    int i = 0;
    for (i=0; i<len; i++)
    {
        printf("%02x", buf[i]);
    }
}

/* TBD: change name to KeySet */
void printKeyEntry(char *princName, PVMKDC_KEYSET pKeyEntry)
{
    int i;

    for (i=0; i<pKeyEntry->numKeys; i++)
    {
        printf("Principal:	%s	", princName);
        printf("    key[%d]: type=%-2d kvno=%-2d len=%-3d ", 
               i, 
               pKeyEntry->encKeys[i]->keytype, 
               pKeyEntry->encKeys[i]->encdata->kvno, 
               VMKDC_GET_LEN_DATA(pKeyEntry->encKeys[i]->encdata->data));
        printBufInHex(VMKDC_GET_PTR_DATA(pKeyEntry->encKeys[i]->encdata->data), 
                      VMKDC_GET_LEN_DATA(pKeyEntry->encKeys[i]->encdata->data));
        printf("\n");
    }
}

int main(int argc, char *argv[])
{
    char *line = NULL;
    char *princName = NULL;
    FILE *infp = NULL;
    PVMKDC_KEYSET key = NULL;
    int i=0;
    int sts = 0;

    if (argc == 1)
    {
        fprintf(stderr, "usage: %s dumpfile\n", argv[0]);
        return 1;
    }

    sts =  VmKdcGetUpnKeysMitDb(
               "K/M@",
               argv[1],
               &princName,
               &key);
    if (sts == 0)
    {
        printKeyEntry(princName, key);
        free(princName);
        princName = NULL;
        VMKDC_SAFE_FREE_KEYSET(key);
    }

    infp = fopen(argv[1], "r");
    if (!infp)
    {
        fprintf(stderr, "fopen(%s) failed\n", argv[1]);
        return 1;
    }
    line = fgets_long(infp);
    while (line)
    {
        i++;
        if (princName)
        {
            free(princName);
            princName = NULL;
        }
        sts = tokenizeLine(line, &princName, &key);
        if (sts == 0 && princName && key)
        {
            printKeyEntry(princName, key);
            printf("\n");
        }
        VMKDC_SAFE_FREE_KEYSET(key);
        free(line);
        line = fgets_long(infp);
    }

    if (infp)
    {
        fclose(infp);
    }
    if (line)
    {
        free(line);
    }
    if (princName)
    {
        free(princName);
    }

    return 0;
}
