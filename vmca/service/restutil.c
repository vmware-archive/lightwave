#include "includes.h"

DWORD
base64_decode(
    const char*         pszInput,
    PSTR*               ppszOutput,
    int*                pnLength
    )
{
    DWORD dwError           = 0;
    PSTR pszOutput          = NULL;
    int nLength             = 0;
    BIO* pBio64             = NULL;
    BIO* pBioMem            = NULL;

    if(!pszInput || !ppszOutput)
    {
        dwError = 50; // TODO: fix
        BAIL_ON_VMREST_ERROR(dwError);
    }

    nLength = strlen(pszInput);

    dwError = VMCAAllocateMemory(
                    nLength + 1,
                    (void **)&pszOutput
                    );
    BAIL_ON_VMREST_ERROR(dwError);

    pBio64 = BIO_new(BIO_f_base64());
    pBioMem = BIO_new_mem_buf((char*)pszInput, nLength);
    pBioMem = BIO_push(pBio64, pBioMem);
    BIO_set_flags(pBioMem, BIO_FLAGS_BASE64_NO_NL);
    BIO_set_close(pBioMem, BIO_CLOSE);

    nLength = BIO_read(pBioMem, pszOutput, nLength);
    if(nLength <= 0)
    {
        dwError = 50; // TODO: fix
        BAIL_ON_VMREST_ERROR(dwError);
    }

    *ppszOutput = pszOutput;
    *pnLength = nLength;

cleanup:
    if(pBioMem)
    {
        BIO_free_all(pBioMem);
    }
    return dwError;

error:
    if(ppszOutput)
    {
        *ppszOutput = NULL;
    }
    if(pnLength)
    {
        *pnLength = 0;
    }
    VMCA_SAFE_FREE_MEMORY(pszOutput);
    goto cleanup;
}

uint32_t
base64_encode(
    const unsigned char* pszInput,
    const size_t nInputLength,
    char** ppszOutput
    )
{
    uint32_t dwError = 0;
    char* pszOutput = NULL;
    //int nLength = 0;
    BIO* pBio64 = NULL;
    BIO* pBioMem = NULL;
    BUF_MEM *pMemOut = NULL;

    if(!pszInput || !ppszOutput)
    {
        dwError = 50; // TODO: fix error num
        BAIL_ON_VMCA_ERROR(dwError);
    }

    pBio64 = BIO_new(BIO_f_base64());
    pBioMem = BIO_new(BIO_s_mem());
    pBioMem = BIO_push(pBio64, pBioMem);
    BIO_set_flags(pBioMem, BIO_FLAGS_BASE64_NO_NL);
    BIO_set_close(pBioMem, BIO_CLOSE);

    if(BIO_write(pBioMem, pszInput, nInputLength) <= 0)
    {
        dwError = 50; // TODO: fix error num
        BAIL_ON_VMCA_ERROR(dwError);
    }
    BIO_flush(pBioMem);
    BIO_get_mem_ptr(pBioMem, &pMemOut);

    dwError = VMCAAllocateMemory(pMemOut->length + 1, (void **)&pszOutput);
    BAIL_ON_VMCA_ERROR(dwError);

    memcpy(pszOutput, pMemOut->data, pMemOut->length);

    *ppszOutput = pszOutput;

cleanup:
    if(pBioMem)
    {
        BIO_free_all(pBioMem);
    }
    return dwError;

error:
    if(ppszOutput)
    {
        *ppszOutput = NULL;
    }
    VMCA_SAFE_FREE_MEMORY(pszOutput);
    goto cleanup;
}


DWORD
split_user_and_pass(
    const char*         pszUserPass,
    PSTR*               ppszUser,
    PSTR*               ppszPass
    )
{
    DWORD dwError           = 0;
    PSTR pszUser            = NULL;
    PSTR pszPass            = NULL;
    PSTR pszSeparator       = NULL;
    char SEPARATOR          = ':';
    int nLength             = 0;

    if(IsNullOrEmptyString(pszUserPass) || !ppszUser || !ppszPass)
    {
        dwError = 50; // TODO: fix
        BAIL_ON_VMREST_ERROR(dwError);
    }
    pszSeparator = strchr(pszUserPass, SEPARATOR);
    if(!pszSeparator)
    {
        dwError = 50; // TODO: fix
        BAIL_ON_VMREST_ERROR(dwError);
    }

    nLength = pszSeparator - pszUserPass;
    dwError = VMCAAllocateMemory(
                    nLength + 1,
                    (void **)&pszUser
                    );
    BAIL_ON_VMREST_ERROR(dwError);

    strncpy(pszUser, pszUserPass, nLength);

    nLength = strlen(pszUserPass) - (nLength + 1);
    dwError = VMCAAllocateMemory(
                    nLength + 1,
                    (void **)&pszPass
                    );
    BAIL_ON_VMREST_ERROR(dwError);

    strncpy(pszPass, pszSeparator+1, nLength);

    *ppszUser = pszUser;
    *ppszPass = pszPass;

cleanup:
    return dwError;

error:
    if(ppszUser)
    {
        *ppszUser = NULL;
    }
    if(ppszPass)
    {
        *ppszPass = NULL;
    }
    VMCA_SAFE_FREE_MEMORY(pszUser);
    VMCA_SAFE_FREE_MEMORY(pszPass);
    goto cleanup;
}

DWORD
check_user_and_pass(
    PSTR pszUser,
    PSTR pszPass,
    uint32_t *nValid
    )
{

    return 0;
}


DWORD
VMCARESTVerifyBasicAuth(
    PREST_REQUEST       pRequest,
    PREST_RESPONSE*     ppResponse
    )
{
    DWORD dwError           = 0;
    PSTR pszAuth            = NULL;
    PSTR pszUserPassBase64  = NULL;
    PSTR pszUserPass        = NULL;
    PSTR pszUser            = NULL;
    PSTR pszPass            = NULL;
    int nLength = 0;
    uint32_t nValid = 0;

    dwError = VmRESTGetHttpHeader(
                pRequest,
                "Authorization",
                &pszAuth
                );
    BAIL_ON_VMREST_ERROR(dwError);

    if (!pszAuth)
    {
        dwError = EACCES;
        BAIL_ON_VMREST_ERROR(dwError);
    }

    if(!strstr(pszAuth, VMCA_BASIC_AUTH_STRING))
    {
        dwError = EACCES;
        BAIL_ON_VMREST_ERROR(dwError);
    }

    pszUserPassBase64 = pszAuth + strlen(VMCA_BASIC_AUTH_STRING) + 1;
    if (IsNullOrEmptyString(pszUserPassBase64) )
    {
        dwError = EACCES;
        BAIL_ON_VMREST_ERROR(dwError);
    }

    dwError = base64_decode(
                pszUserPassBase64,
                &pszUserPass,
                &nLength
                );
    BAIL_ON_VMREST_ERROR(dwError);

    dwError = split_user_and_pass(
                pszUserPass,
                &pszUser,
                &pszPass
                );
    BAIL_ON_VMREST_ERROR(dwError);

    dwError = check_user_and_pass(
                pszUser,
                pszPass,
                &nValid
                );
cleanup:

    return 0;

error:

    goto cleanup;

}
