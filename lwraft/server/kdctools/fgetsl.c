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



#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

typedef unsigned int BOOLEAN;

char *fgets_long(FILE *fp)
{
    char        buf[256];
    char        *line    = NULL;
    char        *tmpline = NULL;
    char        *cp      = NULL;
    int         len;
    BOOLEAN eof = FALSE;
    BOOLEAN longline;
    int     sts = 0;

    do {
        longline = FALSE;
        cp = fgets(buf, sizeof(buf)-2, fp);
        if (!cp || feof(fp)) {
            eof = TRUE;
        }

        if (!eof) {
            len = (int) strlen(cp);
            if (len > 0 && cp[len-1] != '\n') {
                longline = TRUE;
            }
            if (line) {
                len += (int) strlen(line);
            }
            tmpline = realloc(line, len+1);
            if (!tmpline) {
                sts = ENOMEM;
                goto clean_exit;
            }
            if (!line) {
                strcpy(tmpline, cp);
            }
            else {
                strcat(tmpline, cp);
            }
            line = tmpline;
        }
    } while (longline);

clean_exit:
    if (sts) {
        if (line) {
            free(line);
            line = NULL;
        }
    }
    return line;
}

