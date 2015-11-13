/*
 * Copyright (C) 2014 VMware, Inc. All rights reserved.
 *
 * Module   : testdl.c
 *
 * Abstract :
 *
 */
#include <errno.h>
#include <dlfcn.h>
#include <stdio.h>
#include "includes.h"

int
main(void)
{
	DWORD dwError = 0;
    PSTR certString = NULL;
    PSTR aliasString = NULL;
    PVMAFD_CERT_ARRAY ppCerts = NULL;
    int x = 0;

    dwError = VmAfdEnumCertificates(
        "localhost",
        CERTIFICATE_STORE_TYPE_PRIVATE,
        0,
        100,
        &ppCerts);
    printf("error Code : %d dwCount= %d\n", dwError,(int) ppCerts->dwCount);

    size_t certlen = 0; //strlen(certString);

    //int y = ppCerts->dwCount;
    for ( x = 0; x < ppCerts->dwCount ; x++)
    {
        VMAFD_CERT_CONTAINER Cert = ppCerts->certificates[x];

        printf("Counter : %d \t dwCount = %d \n ",
        	x, (int) ppCerts->dwCount);
        sleep(1);
        if (Cert.pCert != NULL ) {
            VmAfdAllocateStringAFromW(Cert.pCert,
            &certString);
            certlen = strlen(certString);
        }

        if (Cert.pAlias != NULL ) {
                VmAfdAllocateStringAFromW(Cert.pAlias,
                &aliasString);
        }


        printf("Len %d,  Cert -> %s \t", (int)certlen, certString);
        printf("Alias -> %s \n", aliasString);
        VmAfdFreeStringA(certString);
        VmAfdFreeStringA(aliasString);
        certString = NULL;
        aliasString = NULL;

    }
    return 0;

}
