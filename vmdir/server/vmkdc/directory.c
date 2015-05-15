#include "includes.h"

static
DWORD
_VmKdcGetKrbUPNKey(
    PSTR        pszUpnName,
    PBYTE*      ppKeyBlob,
    DWORD*      pSize
    );

VOID
VmKdcFreeDirectoryEntry(
    PVMKDC_DIRECTORY_ENTRY pDirectoryEntry)
{
    if (pDirectoryEntry)
    {
        VMKDC_SAFE_FREE_STRINGA(pDirectoryEntry->princName);
        VMKDC_SAFE_FREE_KEYSET(pDirectoryEntry->keyset);
        VMKDC_SAFE_FREE_MEMORY(pDirectoryEntry);
    }
}

DWORD
VmKdcInitializeDirectory(
    PVMKDC_GLOBALS pGlobals)
{
    DWORD dwError = 0;
    PBYTE pPrincKeyBlob = NULL;
    DWORD dwPrincKeySize = 0;
    PVMKDC_KEY master = NULL;
    PVMKDC_KEY kmEncKey = NULL;
    PVMKDC_CRYPTO pCrypto = NULL;
    PVMKDC_DATA kmKey = NULL;
    PCSTR pszRealm = NULL;
    PSTR pszMasterName = NULL;
    BOOLEAN     bInLock = FALSE;

    // wait until vmdir gVmdirKrbGlobals is initialized
    VMDIR_LOCK_MUTEX( bInLock, gVmdirKrbGlobals.pmutex);
    while ( gVmdirKrbGlobals.pszRealm == NULL       &&
            VmKdcdState() == VMKDCD_STARTUP
          )
    {
        VmDirConditionTimedWait( gVmdirKrbGlobals.pcond,
                                 gVmdirKrbGlobals.pmutex,
                                 1 * 1000); // wait 1 second
    }
    VMDIR_UNLOCK_MUTEX( bInLock, gVmdirKrbGlobals.pmutex);

    if ( VmKdcdState() == VMKDCD_STARTUP )
    {
        VMKDC_SAFE_FREE_STRINGA( pGlobals->pszDefaultRealm );
        dwError = VmKdcAllocateStringA( gVmdirKrbGlobals.pszRealm, &pGlobals->pszDefaultRealm );
        BAIL_ON_VMKDC_ERROR(dwError);

        pszRealm = pGlobals->pszDefaultRealm;

        dwError = VmKdcDecodeMasterKey(
                        gVmdirKrbGlobals.bervMasterKey.lberbv.bv_val, //pMasterKeyBlob,
                        (DWORD) gVmdirKrbGlobals.bervMasterKey.lberbv.bv_len, //dwMasterKeySize,
                        &master);
        BAIL_ON_VMKDC_ERROR(dwError);

        dwError = VmKdcAllocateStringPrintf(&pszMasterName,
                                            "K/M@%s", pszRealm);
        BAIL_ON_VMKDC_ERROR(dwError);


        dwError = _VmKdcGetKrbUPNKey(
                      pszMasterName,
                      &pPrincKeyBlob,
                      &dwPrincKeySize);
        BAIL_ON_VMKDC_ERROR(dwError);


        /*
         * The K/M master key is ASN.1 encoded and encrypted in the master key
         */
        dwError = VmKdcDecodeMasterKey(
                      pPrincKeyBlob,
                      dwPrincKeySize,
                      &kmEncKey);
        BAIL_ON_VMKDC_ERROR(dwError);

        dwError = VmKdcInitCrypto(pGlobals->pKrb5Ctx, master, &pCrypto);
        BAIL_ON_VMKDC_ERROR(dwError);

        dwError = VmKdcCryptoDecrypt(pCrypto, 0, kmEncKey->data,  &kmKey);
        BAIL_ON_VMKDC_ERROR(dwError);

        if (VMKDC_GET_LEN_DATA(master->data) != VMKDC_GET_LEN_DATA(kmKey) ||
            memcmp(VMKDC_GET_PTR_DATA(master->data),
                   VMKDC_GET_PTR_DATA(kmKey),
                   VMKDC_GET_LEN_DATA(kmKey)))
        {
            // TBD: Not quite right error
            dwError = ERROR_ALLOC_KRB5_CRYPTO_CONTEXT;
            BAIL_ON_VMKDC_ERROR(dwError);
        }

        pthread_mutex_lock(&pGlobals->mutex);
        VMKDC_SAFE_FREE_KEY(pGlobals->masterKey);
        pGlobals->masterKey = master;
        pthread_mutex_unlock(&pGlobals->mutex);
    }

error:
    VMKDC_SAFE_FREE_STRINGA(pszMasterName);
    VMKDC_SAFE_FREE_MEMORY(pPrincKeyBlob);
    VMKDC_SAFE_FREE_KEY(kmEncKey);
    VMKDC_SAFE_FREE_DATA(kmKey);
    VmKdcDestroyCrypto(pCrypto);

    VMDIR_UNLOCK_MUTEX( bInLock, gVmdirKrbGlobals.pmutex);

    return dwError;
}

VOID
VmKdcTerminateDirectory(
    PVMKDC_GLOBALS pGlobals)
{
    pthread_mutex_lock(&pGlobals->mutex);
    if (pGlobals->pDirectory)
    {
        // TBD: Free memory here?
        pGlobals->pDirectory = NULL;
    }
    VMKDC_SAFE_FREE_KEY(pGlobals->masterKey);
    pthread_mutex_unlock(&pGlobals->mutex);
}

DWORD
VmKdcSearchDirectory(
    PVMKDC_CONTEXT pContext,
    PVMKDC_PRINCIPAL pPrincipal,
    PVMKDC_DIRECTORY_ENTRY *ppRetDirectoryEntry)
{
    DWORD dwError = 0;
    PSTR pszPrincName = NULL;
    PVMKDC_KEYSET princKeySet = NULL;
    PVMKDC_DIRECTORY_ENTRY pDirectoryEntry = NULL;
    PVMKDC_DATA princAsn1KeyData = NULL;
    PBYTE pPrincAsn1KeyBlob = NULL;
    DWORD dwPrincAsn1KeySize = 0;

    BAIL_ON_VMKDC_INVALID_POINTER(pContext, dwError);
    BAIL_ON_VMKDC_INVALID_POINTER(pPrincipal, dwError);
    BAIL_ON_VMKDC_INVALID_POINTER(ppRetDirectoryEntry, dwError);

    /*
     * When vmdir is unavailable, the master key will be NULL.
     */
    if (!pContext->pRequest->masterKey)
    {
        dwError = VMKDC_RPC_SERVER_NOTAVAIL;
        BAIL_ON_VMKDC_ERROR(dwError);
    }

    dwError = VmKdcUnparsePrincipalName(pPrincipal, &pszPrincName);
    BAIL_ON_VMKDC_ERROR(dwError);


    dwError = _VmKdcGetKrbUPNKey(pszPrincName, &pPrincAsn1KeyBlob, &dwPrincAsn1KeySize);
    BAIL_ON_VMKDC_ERROR(dwError);


    dwError = VmKdcAllocateData(
                  pPrincAsn1KeyBlob,
                  dwPrincAsn1KeySize,
                  &princAsn1KeyData);
    BAIL_ON_VMKDC_ERROR(dwError);

    dwError = VmKdcDecodeKeySet(princAsn1KeyData , &princKeySet);
    BAIL_ON_VMKDC_ERROR(dwError);

    dwError = VmKdcDecryptKeySet(pContext,
                                 pContext->pRequest->masterKey,
                                 princKeySet);
    BAIL_ON_VMKDC_ERROR(dwError);

    dwError = VmKdcAllocateMemory(sizeof(VMKDC_DIRECTORY_ENTRY),
                                  (PVOID*)&pDirectoryEntry);
    BAIL_ON_VMKDC_ERROR(dwError);

    pDirectoryEntry->princName = pszPrincName;
    pDirectoryEntry->keyset = princKeySet;
    *ppRetDirectoryEntry = pDirectoryEntry;

error:
    VMKDC_SAFE_FREE_DATA(princAsn1KeyData);
    VMKDC_SAFE_FREE_MEMORY(pPrincAsn1KeyBlob);
    if (dwError)
    {
        VMKDC_SAFE_FREE_KEYSET(princKeySet);
        VMKDC_SAFE_FREE_STRINGA(pszPrincName);
        VMKDC_SAFE_FREE_MEMORY(pDirectoryEntry);
    }
    return dwError;
}

DWORD
VmKdcFindKeyByEType(
    PVMKDC_DIRECTORY_ENTRY pDirectoryEntry,
    VMKDC_ENCTYPE etype,
    PVMKDC_KEY *ppRetKey)
{
    DWORD dwError = 0;
    PVMKDC_KEY pKey = NULL;
    DWORD k = 0;

    if (!pDirectoryEntry)
    {
        dwError = ERROR_NO_DATA_AVAILABLE;
        BAIL_ON_VMKDC_ERROR(dwError);
    }

    dwError = 0;
    for (k=0; k<pDirectoryEntry->keyset->numKeys; k++)
    {
        if (etype == pDirectoryEntry->keyset->keys[k]->type)
        {
            pKey = pDirectoryEntry->keyset->keys[k];
            goto found_key;
        }
    }
    dwError = ERROR_NO_KEY_ETYPE;
    BAIL_ON_VMKDC_ERROR(dwError);

found_key:

    if (pKey)
    {
        *ppRetKey = pKey;
    }

error:
    return dwError;
}

static
DWORD
_VmKdcGetKrbUPNKey(
    PSTR        pszUpnName,
    PBYTE*      ppKeyBlob,
    DWORD*      pSize
    )
{
    DWORD               dwError = 0;
    PVDIR_ATTRIBUTE     pKrbUPNKey = NULL;
    PBYTE               pRetUPNKey = NULL;
    VDIR_ENTRY_ARRAY    entryArray = {0};

    if (IsNullOrEmptyString(pszUpnName)
        || !ppKeyBlob
        || !pSize
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMKDC_ERROR(dwError);
    }

    dwError = VmDirSimpleEqualFilterInternalSearch(
                    "",
                    LDAP_SCOPE_SUBTREE,
                    ATTR_KRB_UPN,
                    pszUpnName,
                    &entryArray);
    BAIL_ON_VMKDC_ERROR(dwError);

    if (entryArray.iSize == 1)
    {
        pKrbUPNKey = VmDirFindAttrByName(&(entryArray.pEntry[0]), ATTR_KRB_PRINCIPAL_KEY);

        if (!pKrbUPNKey)
        {
            dwError = ERROR_NO_PRINC;
            BAIL_ON_VMKDC_ERROR(dwError);
        }

        dwError = VmDirAllocateAndCopyMemory(
                        pKrbUPNKey->vals[0].lberbv.bv_val,
                        pKrbUPNKey->vals[0].lberbv.bv_len,
                        (PVOID*)& pRetUPNKey
                        );
        BAIL_ON_VMKDC_ERROR(dwError);

        *ppKeyBlob = pRetUPNKey;
        *pSize     = (DWORD) pKrbUPNKey->vals[0].lberbv.bv_len;
        pRetUPNKey = NULL;
    }
    else
    {
        dwError = ERROR_NO_PRINC;
        BAIL_ON_VMKDC_ERROR(dwError);
    }

cleanup:

    VmDirFreeEntryArrayContent(&entryArray);

    return dwError;

error:

    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "_VmKdcGetKrbUPNKey: failed (%u)(%s)", dwError, VDIR_SAFE_STRING(pszUpnName));

    VMKDC_SAFE_FREE_MEMORY(pRetUPNKey);

    // keep error code space to KDC specific
    dwError = ERROR_NO_PRINC;

    goto cleanup;

}
