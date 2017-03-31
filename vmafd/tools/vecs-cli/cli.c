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

static
DWORD
VecsCliGetEntryType(
    PCSTR pszCertificate,
    PCSTR pszKey,
    CERT_ENTRY_TYPE *pEntryType
    );

static
DWORD
VecsCliReadFromFile(
     PCSTR pszFilePath,
     PSTR *ppszContent
     );

static
DWORD
VecsCliPrintKey(
    PCSTR pszKey
    );

static
PSTR
VecsCliPrintAccess(
    DWORD dwAccessMask
    );

static
DWORD
GetError
(
    VOID
);

DWORD
VecsCliCreateStoreA(
    PVMAFD_SERVER pServer,
    PCSTR pszStoreName
    )
{
    DWORD dwError = 0;

    if (IsNullOrEmptyString(pszStoreName))
    {
        fprintf(
            stderr,
            "Error : An invalid store name [%s] was specified \n",
            VMAFD_SAFE_STRING(pszStoreName));
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VecsCreateCertStoreHA(
                    pServer,
                    pszStoreName,
                    NULL,
                    NULL);
    BAIL_ON_VMAFD_ERROR(dwError);

    fprintf(stdout, "Successfully created store [%s]\n", pszStoreName);

cleanup:

    return dwError;

error:

    goto cleanup;
}

DWORD
VecsCliListStoreA(
    PVMAFD_SERVER pServer
    )
{
    DWORD dwError = 0;
    PSTR* ppszStoreNameArray = NULL;
    DWORD dwCount = 0;

    dwError = VecsEnumCertStoreHA(
                    pServer,
                    &ppszStoreNameArray,
                    &dwCount);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (!dwCount)
    {
        fprintf(stderr, "No stores found\n");
    }
    else
    {
        DWORD idx = 0;

        for (; idx < dwCount; idx++)
        {
            fprintf(stdout, "%s\n", ppszStoreNameArray[idx]);
        }
    }

cleanup:

    if (ppszStoreNameArray)
    {
        VmAfdFreeStringArrayCountA(ppszStoreNameArray, dwCount);
    }

    return dwError;

error:

    goto cleanup;
}

DWORD
VecsCliDeleteStoreA(
    PVMAFD_SERVER pServer,
    PCSTR pszStoreName,
    PCSTR pszPassword
    )
{
    DWORD dwError = 0;

    if (IsNullOrEmptyString(pszStoreName))
    {
        fprintf(
            stderr,
            "Error : An invalid store name [%s] was specified \n",
            VMAFD_SAFE_STRING(pszStoreName));
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VecsDeleteCertStoreHA(
                    pServer,
                    pszStoreName);

    if (dwError == ERROR_OBJECT_NOT_FOUND )
    {
	    fprintf(stderr, "Error : Store [%s] does not exist\n", VMAFD_SAFE_STRING(pszStoreName));
	    BAIL_ON_VMAFD_ERROR(dwError);
    }

    BAIL_ON_VMAFD_ERROR(dwError);

    fprintf(stdout, "Successfully deleted store [%s]\n", pszStoreName);

cleanup:

    return dwError;

error:

    goto cleanup;
}

DWORD
VecsCliListEntriesA(
    PVMAFD_SERVER pServer,
    PCSTR pszStoreName,
    PCSTR pszPassword,
    DWORD dwFormatAsText,
    DWORD dwAliasesOnly
    )
{
    DWORD dwError = 0;
    PVECS_STORE pStore = NULL;
    PVECS_ENUM_CONTEXT pEnumContext = NULL;
    PVECS_CERT_ENTRY_A pCertEntryArray = NULL;
    DWORD dwEntryCount = 0;
    DWORD dwEntriesInStore = 0;

    if (IsNullOrEmptyString(pszStoreName))
    {
        fprintf(
            stderr,
            "Error : An invalid store name [%s] was specified \n",
            VMAFD_SAFE_STRING(pszStoreName));
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VecsOpenCertStoreHA(
                pServer,
                pszStoreName,
                pszPassword,
                &pStore);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VecsGetEntryCount(
                                pStore,
                                &dwEntriesInStore
                               );
    BAIL_ON_VECS_CLI_ERROR (
                            dwError,
                            "Could not retrieve number of entries in store"
                           );

    fprintf(
            stdout,
            "Number of entries in store :\t%d\n",
            dwEntriesInStore
           );

    dwError = VecsBeginEnumEntries(
                    pStore,
                    1, /* max entry count */
                    ENTRY_INFO_LEVEL_2,
                    &pEnumContext);
    BAIL_ON_VMAFD_ERROR(dwError);

    do
    {
        DWORD iEntry = 0;

        if (pCertEntryArray)
        {
            VecsFreeCertEntryArrayA(pCertEntryArray, dwEntryCount);

            pCertEntryArray = NULL;
            dwEntryCount = 0;
        }

        dwError = VecsEnumEntriesA(
                    pEnumContext,
                    &pCertEntryArray,
                    &dwEntryCount);
        if (dwError == ERROR_NO_MORE_ITEMS)
        {
            dwError = 0;
        }
        BAIL_ON_VMAFD_ERROR(dwError);


        for (; iEntry < dwEntryCount; iEntry++)
        {
            PVECS_CERT_ENTRY_A pEntry = &pCertEntryArray[iEntry];

            if (dwAliasesOnly)
            {
                fprintf (stdout, "%s\n", pEntry->pszAlias);
            }
            else
            {

                fprintf(stdout, "Alias :\t%s\n", pEntry->pszAlias);

                fprintf(
                      stdout,
                      "Entry type :\t%s\n",
                      VecsMapEntryType(pEntry->entryType));

                if (pEntry->pszCertificate && dwFormatAsText)
                {
                    dwError = VecsPrintCertificate(pEntry->pszCertificate);
                    BAIL_ON_VMAFD_ERROR (dwError);
                }
                else
                {
                    fprintf(
                          stdout,
                          "Certificate :\t%s\n\n",
                          pEntry->pszCertificate ? pEntry->pszCertificate : "");
                }
            }
        }

    } while (dwEntryCount > 0);

cleanup:

    if (pCertEntryArray)
    {
        VecsFreeCertEntryArrayA(pCertEntryArray, dwEntryCount);
    }
    if (pEnumContext)
    {
        VecsEndEnumEntries(pEnumContext);
    }
    if (pStore)
    {
        VecsCloseCertStore(pStore);
    }

    return dwError;

error:

    goto cleanup;
}

DWORD
VecsCliAddEntryA(
    PVMAFD_SERVER pServer,
    PCSTR pszStoreName,
    PCSTR pszPassword,
    PCSTR pszAlias,
    PCSTR pszCertFilePath,
    PCSTR pszKeyFilePath,
    PCSTR pszKeyPassword
    )
{
    DWORD dwError = 0;
    PVECS_STORE pStore = NULL;

    PSTR pszCertificate = NULL;
    PSTR pszKey = NULL;
    PSTR pszAliasUsed = NULL;
    CERT_ENTRY_TYPE entryType = CERT_ENTRY_TYPE_UNKNOWN;

    if (IsNullOrEmptyString(pszStoreName))
    {
        fprintf (
                stderr,
                "Error: The store name [%s] is invalid \n",
                VMAFD_SAFE_STRING(pszStoreName)
                );

        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    if (pszCertFilePath)
    {
        dwError = VecsCliReadFromFile (
                          pszCertFilePath,
                          &pszCertificate
                          );
        BAIL_ON_VMAFD_ERROR (dwError);

     }

    if (pszKeyFilePath)
    {
        dwError = VecsCliReadFromFile (
                          pszKeyFilePath,
                          &pszKey
                          );
        BAIL_ON_VMAFD_ERROR (dwError);

    }

    dwError = VecsCliGetEntryType(
                    pszCertificate,
                    pszKey,
                    &entryType
                    );
    BAIL_ON_VMAFD_ERROR (dwError);

    if (entryType == CERT_ENTRY_TYPE_SECRET_KEY &&
        IsNullOrEmptyString (pszAlias)
       )
    {
        fprintf(
                stderr,
                "Error: The alias [%s] is invalid \n",
                VMAFD_SAFE_STRING(pszAlias)
               );
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    else if (IsNullOrEmptyString (pszAlias))
    {

        dwError = VecsComputeCertAliasA(
                                        pszCertificate,
                                        &pszAliasUsed
                                      );
        BAIL_ON_VECS_CLI_ERROR (dwError, "Could not generate alias from certificate");
    }

    dwError = VecsOpenCertStoreHA(
                        pServer,
                        pszStoreName,
                        pszPassword,
                        &pStore
                        );
    BAIL_ON_VECS_CLI_ERROR (dwError, "Failed to open the store.");

    dwError = VecsAddEntryA (
                      pStore,
                      entryType,
                      IsNullOrEmptyString(pszAlias)?pszAliasUsed:pszAlias,
                      pszCertificate,
                      pszKey,
                      pszKeyPassword, //PASSWORD
                      0 //AUTO_REFRESH
                      );
    BAIL_ON_VMAFD_ERROR (dwError);

    fprintf (
            stdout,
            "Entry with alias [%s] in store [%s] was created successfully \n",
            IsNullOrEmptyString(pszAlias)? pszAliasUsed : pszAlias,
            pszStoreName
            );


cleanup:
    if (pStore)
    {
        VecsCloseCertStore (pStore);
    }
    VMAFD_SAFE_FREE_MEMORY (pszCertificate);
    VMAFD_SAFE_FREE_MEMORY (pszKey);
    VMAFD_SAFE_FREE_MEMORY (pszAliasUsed);

    return dwError;

error:
    goto cleanup;
}


DWORD
VecsCliGetCertificateA(
    PVMAFD_SERVER pServer,
    PCSTR pszStoreName,
    PCSTR pszPassword,
    PCSTR pszAlias,
    PCSTR pszOutputFilePath,
    DWORD dwFormatAsText
    )
{
    DWORD dwError = 0;
    PVECS_STORE pStore = NULL;
    PVECS_CERT_ENTRY_A pCertEntry = NULL;
    FILE *stream = NULL;

    if (IsNullOrEmptyString(pszStoreName) ||
        IsNullOrEmptyString(pszAlias)
       )
    {
        fprintf (
                stderr,
                "Error: The store name [%s] or the Alias [%s] is invalid \n",
                VMAFD_SAFE_STRING(pszStoreName),
                VMAFD_SAFE_STRING(pszAlias)
                );
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    dwError = VecsOpenCertStoreHA(
                      pServer,
                      pszStoreName,
                      pszPassword,
                      &pStore
                      );
    BAIL_ON_VECS_CLI_ERROR (dwError, "Failed to open the store.");

    dwError = VecsGetEntryByAliasA(
                        pStore,
                        pszAlias,
                        ENTRY_INFO_LEVEL_2,
                        &pCertEntry
                        );
    BAIL_ON_VMAFD_ERROR (dwError);

    if (pCertEntry->entryType == CERT_ENTRY_TYPE_SECRET_KEY)
    {
        fprintf (
                  stderr,
                  "Error: No certificates were found for entry [%s] of type [%s].\n",
                  pszAlias,
                  VecsMapEntryType(CERT_ENTRY_TYPE_SECRET_KEY)
                );
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    if (!(IsNullOrEmptyString(pszOutputFilePath)))
    {
        dwError = VmAfdOpenFilePath(pszOutputFilePath, "w+", &stream, 0);
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    if (dwFormatAsText)
    {
        dwError = VecsPrintCertificate(pCertEntry->pszCertificate);
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    if (stream)
    {
        fprintf(
            stream,
            "%s\n",
            pCertEntry->pszCertificate?pCertEntry->pszCertificate:""
            );

        fclose(stream);
        stream = NULL;

        dwError = VmAfdRestrictFilePermissionToSelf(pszOutputFilePath);
        BAIL_ON_VMAFD_ERROR(dwError);
    }
    else if (!dwFormatAsText)
    {
        fprintf (
              stdout,
              "%s\n",
              pCertEntry->pszCertificate?pCertEntry->pszCertificate:""
              );
    }

cleanup:
    VecsFreeCertEntryA(pCertEntry);
    if (pStore)
    {
        VecsCloseCertStore (pStore);
    }

    return dwError;
error:
    if (stream)
    {
      fclose(stream);
    }

    goto cleanup;
}

DWORD VecsCliGetKeyA(
    PVMAFD_SERVER pServer,
    PCSTR pszStoreName,
    PCSTR pszPassword,
    PCSTR pszAlias,
    PCSTR pszOutputFilePath,
    DWORD dwFormatAsText,
    PCSTR pszKeyPassword
    )
{
    DWORD dwError = 0;
    PVECS_STORE pStore = NULL;
    PVECS_CERT_ENTRY_A pCertEntry = NULL;
    PSTR pszKey = NULL;
    FILE *stream = NULL;

    if (IsNullOrEmptyString(pszStoreName) ||
        IsNullOrEmptyString(pszAlias)
       )
    {
        fprintf (
                stderr,
                "Error: The store name [%s] or the Alias [%s] is invalid \n",
                VMAFD_SAFE_STRING(pszStoreName),
                VMAFD_SAFE_STRING(pszAlias)
                );

        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    dwError = VecsOpenCertStoreHA(
                      pServer,
                      pszStoreName,
                      pszPassword,
                      &pStore
                      );
    BAIL_ON_VECS_CLI_ERROR (dwError, "Failed to open the store.");

    dwError = VecsGetEntryByAliasA(
                      pStore,
                      pszAlias,
                      ENTRY_INFO_LEVEL_1,
                      &pCertEntry
                      );
    BAIL_ON_VMAFD_ERROR (dwError);

    if (pCertEntry->entryType == CERT_ENTRY_TYPE_TRUSTED_CERT)
    {
        fprintf (
                stderr,
                "Error: No key was found for entry [%s] of type [%s]\n",
                pszAlias,
                VecsMapEntryType (CERT_ENTRY_TYPE_TRUSTED_CERT)
                );
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    dwError = VecsGetKeyByAliasA(
                        pStore,
                        pszAlias,
                        pszKeyPassword,
                        &pszKey
                        );
    BAIL_ON_VMAFD_ERROR (dwError);

    if (!(IsNullOrEmptyString(pszOutputFilePath)))
    {
        dwError = VmAfdOpenFilePath(pszOutputFilePath, "w+", &stream, 0);
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    if (dwFormatAsText)
    {
        dwError = VecsCliPrintKey(pszKey);
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    if (stream)
    {
        fprintf(
            stream,
            "%s\n",
            pszKey?pszKey:""
            );

        fclose(stream);
        stream = NULL;

        dwError = VmAfdRestrictFilePermissionToSelf(pszOutputFilePath);
        BAIL_ON_VMAFD_ERROR(dwError);
   }
    else if (!dwFormatAsText)
    {
        fprintf (
              stdout,
              "%s\n",
              pszKey?pszKey:""
              );
    }

cleanup:
    VMAFD_SAFE_FREE_MEMORY(pszKey);

    if (pStore)
    {
        VecsCloseCertStore (pStore);
    }
    return dwError;
error:
    if (stream)
    {
      fclose(stream);
    }
    goto cleanup;

}

DWORD
VecsCliDeleteEntryA(
    PVMAFD_SERVER pServer,
    PCSTR pszStoreName,
    PCSTR pszPassword,
    PCSTR pszAlias
    )
{
    DWORD dwError = 0;
    PVECS_STORE pStore = NULL;

    if (IsNullOrEmptyString(pszStoreName) ||
        IsNullOrEmptyString(pszAlias)
       )
    {
        fprintf(
            stderr,
            "Error : An invalid store name [%s] was specified\n ",
            VMAFD_SAFE_STRING(pszStoreName));
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VecsOpenCertStoreHA(
                pServer,
                pszStoreName,
                pszPassword,
                &pStore);
    BAIL_ON_VECS_CLI_ERROR (dwError, "Failed to open the store");

    dwError = VecsDeleteEntryA(
                pStore,
                pszAlias
                );
    BAIL_ON_VMAFD_ERROR (dwError);

    fprintf (
            stdout,
            "Deleted entry with alias [%s] in store [%s] successfully\n ",
            pszAlias,
            pszStoreName
            );
cleanup:
    if (pStore)
    {
        VecsCloseCertStore(pStore);
    }

    return dwError;

error:

    goto cleanup;
}

DWORD
VecsCliSetPermissionA (
    PCSTR pszStoreName,
    PCSTR pszUserName,
    VECS_PERMISSION_MODE permMode,
    DWORD dwAccessMask
    )
{
    DWORD dwError = 0;
    PVECS_STORE pStore = NULL;

    if (IsNullOrEmptyString (pszStoreName) ||
        IsNullOrEmptyString (pszUserName) ||
        !dwAccessMask ||
        permMode == VECS_PERMISSION_MODE_UNKNOWN
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    dwError = VecsOpenCertStoreA (
                                  NULL,
                                  pszStoreName,
                                  NULL,
                                  &pStore
                                 );
    BAIL_ON_VECS_CLI_ERROR (dwError, "Failed to open the store");

    switch (permMode)
    {
        case VECS_PERMISSION_MODE_GRANT:
              dwError = VecsSetPermissionA (
                                  pStore,
                                  pszUserName,
                                  dwAccessMask
                                  );
              break;
        case VECS_PERMISSION_MODE_REVOKE:
              dwError = VecsRevokePermissionA(
                                 pStore,
                                 pszUserName,
                                 dwAccessMask
                                 );
              break;
        default:
              dwError = ERROR_INVALID_PARAMETER;
              break;
    }
    BAIL_ON_VMAFD_ERROR (dwError);

    fprintf (
            stdout,
            "Permissions for store [%s] set  successfully\n ",
            pszStoreName
            );
cleanup:
    if (pStore)
    {
        VecsCloseCertStore (pStore);
    }

    return dwError;

error:

    goto cleanup;
}

DWORD
VecsCliGetPermissionsA (
    PVMAFD_SERVER pServer,
    PCSTR pszStoreName
    )
{
    DWORD dwError = 0;
    DWORD dwIndex = 0;
    PVECS_STORE pStore = NULL;
    PSTR pszOwnerName = NULL;
    DWORD dwUserCount = 0;
    PVECS_STORE_PERMISSION_A pStorePermissions = NULL;

    if (IsNullOrEmptyString (pszStoreName))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    dwError = VecsOpenCertStoreA (
                                  NULL,
                                  pszStoreName,
                                  NULL,
                                  &pStore
                                 );
    BAIL_ON_VECS_CLI_ERROR (dwError, "Failed to open the store");

    dwError = VecsGetPermissionsA (
                            pStore,
                            &pszOwnerName,
                            &dwUserCount,
                            &pStorePermissions
                            );
    BAIL_ON_VMAFD_ERROR (dwError);


    fprintf (stdout,
             "PERMISSIONS FOR STORE: [%s]\n",
             pszStoreName
            );
    fprintf (
             stdout,
             "OWNER : %s\n",
             pszOwnerName
            );

    fprintf (
             stdout,
             "USER\t\tACCESS\n"
            );

    for (;dwIndex < dwUserCount; dwIndex++)
    {
        fprintf (stdout,
                 "%s\t%s\n",
                 pStorePermissions[dwIndex].pszUserName,
                 VecsCliPrintAccess(pStorePermissions[dwIndex].dwAccessMask)
                );
    }

cleanup:
    if (pStore)
    {
        VecsCloseCertStore (pStore);
    }
    if (pStorePermissions)
    {
        VecsFreeStorePermissionsArrayA (
                          pStorePermissions,
                          dwUserCount
                          );
    }
    VMAFD_SAFE_FREE_MEMORY (pszOwnerName);

    return dwError;
error:
    goto cleanup;
}



static
DWORD
VecsCliReadFromFile(
    PCSTR pszFilePath,
    PSTR *ppszContent
    )
{
    DWORD dwError = 0;
    PSTR pszContent = NULL;
    FILE *stream = NULL;
    DWORD dwFileSize = 0;
    DWORD dwBytesRead = 0;


    if (IsNullOrEmptyString(pszFilePath) ||
        !ppszContent
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    dwError = VmAfdOpenFilePath(pszFilePath, "r", &stream, 0);
    if (!stream)
    {
        fprintf (
                stderr,
                "Error: Failed to open file [%s] \n",
                pszFilePath
                );
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    fseek(stream, 0L, SEEK_END);
    dwFileSize = ftell(stream);
    fseek(stream, 0L, SEEK_SET);

    dwError = VmAfdAllocateMemory (
                          dwFileSize+1,
                          (PVOID *)&pszContent
                          );
    BAIL_ON_VMAFD_ERROR (dwError);

    dwBytesRead = fread(
                    pszContent,
                    sizeof(CHAR),
                    dwFileSize,
                    stream
                    );
    if (!dwBytesRead)
    {
        if (ferror(stream))
        {
            dwError = GetError();
            BAIL_ON_VMAFD_ERROR (dwError);
        }
    }

    pszContent[dwBytesRead] = '\0';

    *ppszContent = pszContent;

cleanup:
    if (stream)
    {
      fclose(stream);
    }
    return dwError;

error:
    if (pszContent)
    {
        VMAFD_SAFE_FREE_MEMORY(pszContent);
    }
    if (ppszContent)
    {
        *ppszContent = NULL;
    }

    goto cleanup;
}

static
DWORD
VecsCliGetEntryType(
    PCSTR pszCertificate,
    PCSTR pszKey,
    CERT_ENTRY_TYPE *pEntryType
    )
{
    DWORD dwError = 0;
    CERT_ENTRY_TYPE entryType = CERT_ENTRY_TYPE_UNKNOWN;

    if (!pEntryType)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    if (IsNullOrEmptyString(pszCertificate) &&
        IsNullOrEmptyString(pszKey)
       )
    {
        fprintf (
                stderr,
                "Error: A certificate or a key must be specified. \n"
                );
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    if (IsNullOrEmptyString(pszCertificate) &&
        !(IsNullOrEmptyString(pszKey))
        )
    {
        entryType = CERT_ENTRY_TYPE_SECRET_KEY;
    }
    else if (IsNullOrEmptyString(pszKey) &&
              !(IsNullOrEmptyString(pszCertificate))
            )
    {
        entryType = CERT_ENTRY_TYPE_TRUSTED_CERT;
    }
    else
    {
        entryType = CERT_ENTRY_TYPE_PRIVATE_KEY;
    }

    *pEntryType = entryType;

cleanup:
    return dwError;
error:
    if (pEntryType)
    {
        *pEntryType = CERT_ENTRY_TYPE_UNKNOWN;
    }

    goto cleanup;
}

static
DWORD
VecsCliPrintKey(
    PCSTR pszKey
    )
{
    DWORD dwError = 0;
    RSA *rsa = NULL;
    BIO *kbio = NULL;
    kbio = BIO_new_mem_buf((void*)pszKey, -1);
    rsa = PEM_read_bio_RSAPrivateKey(kbio, NULL, 0, NULL);

    if (rsa)
    {
        RSA_print_fp(stdout,rsa,0);
    }

    RSA_free(rsa);
    BIO_free_all(kbio);
    return dwError;
}

static
PSTR
VecsCliPrintAccess (
    DWORD dwAccessMask
    )
{
    if (dwAccessMask >= (READ_STORE | WRITE_STORE))
    {
        return "read,write";
    }
    else if (dwAccessMask == WRITE_STORE)
    {
        return "write";
    }
    else
    {
        return "read";
    }
}

static
DWORD
GetError(
    VOID
    )
{
#if defined _WIN32
        return GetLastError();
#else
        return LwErrnoToWin32Error(errno);
#endif
}
