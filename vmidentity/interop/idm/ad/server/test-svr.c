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

/*
 * Module Name:
 *
 *        test-svr.c
 *
 * Abstract:
 *                      
 *        Identity Manager - Active Directory Integration
 *
 *        Service tests
 *
 * Authors: Adam Bernstein (abernstein@vmware.com)
 *          Sriram Nambakam (snambakam@vmware.com)
 *
 */

#include "includes.h"

#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>



#define _LISTEN_PORT 4444

void
PrintUserInfo(PIDM_USER_INFO pIdmUserInformation)
{
    PSTR pszStr = NULL;
    DWORD groupCount = 0;

    if (!pIdmUserInformation)
    {
        return;
    }

    LwRtlCStringAllocateFromWC16String(
        &pszStr,
        pIdmUserInformation->pszUserName);
    printf("IDMGetUserInformationFromAuthContext: user='%s'\n", pszStr);
    free(pszStr);
    printf("Groups: %d\n", pIdmUserInformation->dwNumGroups);
    for (groupCount=0; groupCount < pIdmUserInformation->dwNumGroups; groupCount++)
    {
        LwRtlCStringAllocateFromWC16String(
            &pszStr,
            pIdmUserInformation->ppszGroupNames[groupCount]);
        printf("Group[%d] = %s\n", groupCount, pszStr);
        free(pszStr);
    }
}

void print_hexdump(unsigned char *buf, int len)
{
    int i = 0;
    int j = 0;

    for (i=0, j=0; i<len; i++)
    {
        if (i && (i % 16) == 0)
        {
            printf(" : ");
            while (j < i)
            {
                printf("%c", isprint((int) buf[j]) ? buf[j] : '.');
                j++;
            }
            printf("\n");
            j = i;
        }
        printf("%02x ", buf[i]);
    }
    printf("\n");
}


int bind_socket(int ret_sock)
{
    struct sockaddr_in addr;
    uint16_t port = _LISTEN_PORT;
    int sock = 0;
    int sts = 0;


    memset(&addr, 0, sizeof(addr));
    addr.sin_port = htons(port);
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
        return sock;
    }
    sts = bind(sock, (struct sockaddr *) &addr, sizeof(addr));
    if (sts < 0)
    {
        return sts;
    }
    return sock;
}

int send_buf(int s, unsigned char *buf, int len)
{
    unsigned char *p = NULL;
    int bytes_wrote = 0;
    int sts = 0;

    p = buf;
    while (bytes_wrote < len)
    {
        sts = send(s, p, len, 0);
        if (sts <= 0)
        {
            return sts;
        }
        bytes_wrote += sts;
        p += sts;
    }
    return bytes_wrote;
}


int recv_buf(int s, unsigned char *buf, int len)
{
    unsigned char *p = NULL;
    int bytes_read = 0;
    int sts = 0;

    p = buf;
    while (bytes_read < len)
    {
        sts = recv(s, p, len, 0);
        if (sts <= 0)
        {
            return sts;
        }
        bytes_read += sts;
        p += sts;
    }
    return bytes_read;
}

int process_connection(int s)
{
    uint32_t io_size = 0;
    unsigned char *buf = NULL;
    int sts = 0;
    DWORD dwError = 0;
    BOOL bAuthContext = FALSE;
    BOOL bAuthDone = FALSE;
    PWSTR pszPackageName = NULL;
    PIDM_AUTH_CONTEXT pAuthContext = NULL;
    PIDM_USER_INFO pIdmUserInformation = NULL;
    NTSTATUS status = 0;
    PBYTE outputBuf = NULL;
    DWORD outputBufLen = 0;

    printf("process_connection: %d\n", s);

    status = LwRtlWC16StringAllocateFromCString(
                 &pszPackageName,
                 "Negotiate");
    if (status)
    {
        return -1;
    }

    bAuthContext = IDMCreateAuthContext(
                       pszPackageName,
                       &pAuthContext);
    free(pszPackageName);
    if (bAuthContext || !pAuthContext)
    {
        printf("IDMCreateAuthContext: failed \n");
        return -1;
    }

    while (!bAuthDone)
    {
        printf("<----  Request data to server <----\n");
        sts = recv_buf(s, (unsigned char *) &io_size, sizeof(io_size));
        if (sts != sizeof(io_size))
        {
            return -1;
        }
        printf("Buffer receive length=%d\n", io_size);
        buf = (unsigned char *) calloc(io_size, sizeof(char));
        if (!buf)
        {
            return -1;
        }

        sts = recv_buf(s, buf, io_size);
        if (sts != io_size)
        {
            return -1;
        }
        print_hexdump(buf, io_size);
        printf("\n");

        outputBufLen = 0;
        dwError = IDMAuthenticate2(
                      pAuthContext,
                      (PBYTE) buf,
                      io_size,
                      &outputBuf,
                      &outputBufLen,
                      &bAuthDone);
        if (dwError)
        {
            return dwError;
        }
        printf("IDMAuthenticate2: dwError=%d bAuthDone=%d outputBufLen=%d\n",
               dwError, bAuthDone, outputBufLen);
        free(buf);
        if (outputBufLen > 0)
        {
            printf("---> Response data to client --->\n");
            printf("Buffer send length=%d\n", io_size);
            print_hexdump(outputBuf, outputBufLen);
            printf("\n");
        }
        if (outputBufLen > 0 && !bAuthDone)
        {
            io_size = outputBufLen;
            sts = send_buf(s, (unsigned char *) &io_size, sizeof(io_size));
            if (sts != sizeof(io_size))
            {
                return -1;
            }
            sts = send_buf(s, outputBuf, outputBufLen);
            if (sts != outputBufLen)
            {
                return -1;
            }
            free(outputBuf);
            outputBuf = NULL;
        }
    }

    dwError = IDMGetUserInformationFromAuthContext(
                  pAuthContext,
                  &pIdmUserInformation);
    if (dwError)
    {
        return dwError;
    }
    PrintUserInfo(pIdmUserInformation);

    IDMFreeUserInfo(pIdmUserInformation);
    IDMFreeAuthContext(pAuthContext);
    close(s);

    return 0;
}


int main(int argc, char *argv[])
{
    int sock = 0;
    int sts = 0;
    int csock = 0;

    sock = bind_socket(_LISTEN_PORT);
    printf("sock=%d\n", sock);

    sts = listen(sock, 5);
    if (sts == -1)
    {
        printf("listen() failed %s\n", strerror(errno));
        return 1;
    }

    for (;;)
    {
        csock = accept(sock, NULL, NULL);
        if (csock == -1)
        {
            printf("accept: failed %s\n", strerror(errno));
            return 1;
        }
        sts = process_connection(csock);
    }

    return 0;
}
