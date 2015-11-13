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
 * Module Name: Directory middle layer
 *
 * Filename: srputil.c
 *
 * Abstract:
 *
 * SASL SRP secret key creation.
 * This implementation closely matches with SASL srp.c srp_setpass function to create and store
 *   { utf8(mda) mpi(v) os(salt) }  (base64 encoded) SRP secret in Lotus.
 *
 */

#include "includes.h"

// Only support SHA-1 for now to match with SASL srp.c implementation.
//   (actually, we should read vmdrd.conf sasl config file first and fall back to SHA-1)
static PCSTR             gSASLMDAName[] = { "SHA-1", "sha1" };

// This needs to match with srp.c generate_N_and_g function.
static const char*  g2048N = "AC6BDB41324A9A9BF166DE5E1389582FAF72B6651987EE07FC3192943DB56050A37329"
                             "CBB4A099ED8193E0757767A13DD52312AB4B03310DCD7F48A9DA04FD50E8083969EDB7"
                             "67B0CF6095179A163AB3661A05FBD5FAAAE82918A9962F0B93B855F97993EC975EEAA8"
                             "0D740ADBF4FF747359D041D5C33EA71D281E446B14773BCA97B43A23FB801676BD207A"
                             "436C6481F1D2B9078717461A5B9D32E688F87748544523B524B0D57D5EA77A2775D2EC"
                             "FA032CFBDBF52FB3786160279004E57AE6AF874E7303CE53299CCC041C7BC308D82A56"
                             "98F3A8D0C38271AE35F8E9DBFBB694B5C803D89F7AE435DE236D525F54759B65E372FC"
                             "D68EF20FA7111F9E4AFF73";
static const char*  g2048g = "2";

static
DWORD
_VmDirSRPMakeVandSalt(
    PCSTR           pszUPN,
    PVDIR_BERVALUE  pBervPass,
    PVDIR_BERVALUE  pBervV,
    PVDIR_BERVALUE  pBervSalt
    );

static
DWORD
_VmDirSRPMakeSecret(
    PCSTR           pszMDAName,
    PVDIR_BERVALUE  pBervVBlob,
    PVDIR_BERVALUE  pBervSaltBlob,
    PVDIR_BERVALUE  pBervSecretBlob
    );

static
DWORD
_VmDirSRPCalculateX(
    const EVP_MD*   pMD,
    PCSTR           pszUser,
    PVDIR_BERVALUE  pBervPass,
    PVDIR_BERVALUE  pBervSalt,
    BIGNUM*         pX
    );

static
DWORD
_VmDirSRPCalculateV(
    BIGNUM*         pN,
    BIGNUM*         pg,
    PCSTR           pszUPN,
    PVDIR_BERVALUE  pBervPass,
    PVDIR_BERVALUE  pBervSalt,
    BIGNUM*         pv
    );


static
DWORD
_VmDirSRPMakeVandSalt(
    PCSTR           pszUPN,
    PVDIR_BERVALUE  pBervPass,
    PVDIR_BERVALUE  pBervV,
    PVDIR_BERVALUE  pBervSalt
    );

static
DWORD
_VmdirSRPValidateVandSalt(
    PCSTR pUpn,
    PCSTR pPwd,
    PBYTE pSalt,
    DWORD dwSaltLen,
    PBYTE pV,
    DWORD dwVLen
    );

DWORD
VmDirSRPCreateSecret(
    PVDIR_BERVALUE   pUPN,
    PVDIR_BERVALUE   pClearTextPasswd,
    PVDIR_BERVALUE   pSecretResult
    )
{
    DWORD           dwError = 0;
    PSTR            pszLowerCaseUPN = NULL;
    VDIR_BERVALUE   bervSecretBlob = VDIR_BERVALUE_INIT;
    VDIR_BERVALUE   bervVBlob = VDIR_BERVALUE_INIT;
    VDIR_BERVALUE   bervSaltBlob = VDIR_BERVALUE_INIT;
    BOOLEAN         bSrpValid = FALSE;
    DWORD           dwRetryCount = 0;

    if ( !pUPN || !pClearTextPasswd || !pSecretResult)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocASCIIUpperToLower(pUPN->lberbv.bv_val, &pszLowerCaseUPN );
    BAIL_ON_VMDIR_ERROR(dwError);

    do
    {
        // calculate salt and v
        VmDirFreeBervalContent(&bervVBlob);
        VmDirFreeBervalContent(&bervSaltBlob);
        dwError = _VmDirSRPMakeVandSalt( pszLowerCaseUPN,
                                         pClearTextPasswd,
                                         &bervVBlob,
                                         &bervSaltBlob);
        BAIL_ON_VMDIR_ERROR(dwError);

        // Validate salt/v works with CSRP library
        dwError = _VmdirSRPValidateVandSalt(
                      pszLowerCaseUPN,
                      pClearTextPasswd->lberbv_val,
                      bervSaltBlob.lberbv_val,
                      (DWORD) bervSaltBlob.lberbv_len,
                      bervVBlob.lberbv_val,
                      (DWORD) bervVBlob.lberbv_len);
        if (dwError == 0)
        {
            bSrpValid = TRUE;
        }
        else
        {
            dwRetryCount++;
        }
    } while (!bSrpValid && dwRetryCount < 128);

    dwError = _VmDirSRPMakeSecret( gSASLMDAName[0],
                                   &bervVBlob,
                                   &bervSaltBlob,
                                   &bervSecretBlob);
    BAIL_ON_VMDIR_ERROR(dwError);

    pSecretResult->lberbv = bervSecretBlob.lberbv;
    pSecretResult->bOwnBvVal = TRUE;

    bervSecretBlob.lberbv.bv_val = NULL;
    bervSecretBlob.lberbv.bv_len = 0;
    bervSecretBlob.bOwnBvVal = FALSE;

cleanup:

    VMDIR_SAFE_FREE_MEMORY(pszLowerCaseUPN);
    VmDirFreeBervalContent(&bervVBlob);
    VmDirFreeBervalContent(&bervSaltBlob);
    VmDirFreeBervalContent(&bervSecretBlob);

    return dwError;

error:

    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
                     "VmDirSRPCreateSecret failed. (%u)",
                     dwError);

    goto cleanup;
}



/*
 *  Based on SASL srp.c implementation, the secret data should be stored in following format:
 *
 *  The secret data is stored as suggested in RFC 2945:
 *  {4 bytes len { utf8(mda) mpi(v) os(salt) }}  (base64 encoded)
 *
 */
static
DWORD
_VmDirSRPMakeSecret(
    PCSTR           pszMDAName,
    PVDIR_BERVALUE  pBervVBlob,
    PVDIR_BERVALUE  pBervSaltBlob,
    PVDIR_BERVALUE  pBervSecretBlob
    )
{
    DWORD       dwError = 0;
    PBYTE       pLocalByte = NULL;
    UINT32      uiLocalByteLen = 0;
    PBYTE       pLocalEncodedByte = NULL;
    UINT32      uiLocalEncodedBufferLen = 0;
    UINT32      uiLocalEncodedByteLen = 0;
    UINT32      uiIdx = 4;  // reserve 4 for length encoding
    UINT32      uiMDANameLen = 0;
    UINT32      uiDataLen = 0;
    unsigned short  twoByteEncodeLen = 0;
    unsigned char   oneByteEncodeLen = 0;

    if ( pszMDAName == NULL || pBervVBlob == NULL || pBervSaltBlob == NULL || pBervSecretBlob == NULL )
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    uiMDANameLen = (UINT32) VmDirStringLenA(pszMDAName);

    // calculate buffer size
    // 0. 4 byte length
    // 1. utf8(mda) : 2 bytes + string
    // 2. mpi(v)    : 2 bytes + verifier
    // 3. os(salt)  : 1 bytes + salt
    uiLocalByteLen = (UINT32) (4 + 2 + uiMDANameLen + 2 + pBervVBlob->lberbv_len + 1 + pBervSaltBlob->lberbv_len);
    dwError = VmDirAllocateMemory( uiLocalByteLen, (PVOID*) &pLocalByte );
    BAIL_ON_VMDIR_ERROR(dwError);

    // encode 2 bytes len in network order
    twoByteEncodeLen = htons( (unsigned short) uiMDANameLen);
    dwError = VmDirCopyMemory( pLocalByte + uiIdx, uiLocalByteLen - uiIdx, &twoByteEncodeLen, sizeof(twoByteEncodeLen));
    BAIL_ON_VMDIR_ERROR(dwError);
    uiIdx += sizeof(twoByteEncodeLen);
    // encode MDAName string
    dwError = VmDirCopyMemory( pLocalByte + uiIdx, uiLocalByteLen - uiIdx, (PVOID)pszMDAName, uiMDANameLen);
    BAIL_ON_VMDIR_ERROR(dwError);
    uiIdx += uiMDANameLen;

    // encode 2 bytes len in network order
    twoByteEncodeLen = htons( (unsigned short) pBervVBlob->lberbv_len);
    dwError = VmDirCopyMemory( pLocalByte + uiIdx, uiLocalByteLen - uiIdx, &twoByteEncodeLen, sizeof(twoByteEncodeLen));
    BAIL_ON_VMDIR_ERROR(dwError);
    uiIdx += sizeof(twoByteEncodeLen);
    // encode verifier
    dwError = VmDirCopyMemory( pLocalByte + uiIdx, uiLocalByteLen - uiIdx, pBervVBlob->lberbv_val, pBervVBlob->lberbv_len);
    BAIL_ON_VMDIR_ERROR(dwError);
    uiIdx += (UINT32) pBervVBlob->lberbv_len;

    if (pBervSaltBlob->lberbv_len > 255) // max one byte
    {
        dwError = VMDIR_ERROR_SRP;
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    // encode one byte salt len
    oneByteEncodeLen = (pBervSaltBlob->lberbv_len) & 0xFF;
    dwError = VmDirCopyMemory( pLocalByte + uiIdx, uiLocalByteLen - uiIdx, &oneByteEncodeLen, sizeof(oneByteEncodeLen));
    BAIL_ON_VMDIR_ERROR(dwError);
    uiIdx += sizeof(oneByteEncodeLen);
    dwError = VmDirCopyMemory( pLocalByte + uiIdx, uiLocalByteLen - uiIdx, pBervSaltBlob->lberbv_val, pBervSaltBlob->lberbv_len);
    BAIL_ON_VMDIR_ERROR(dwError);
    uiIdx += (UINT32) pBervSaltBlob->lberbv_len;

    // add 4 byte real data len (network order) to the beginning of the buffer
    uiDataLen = htonl( uiIdx - 4);
    dwError = VmDirCopyMemory( pLocalByte, 4,&uiDataLen, 4);
    BAIL_ON_VMDIR_ERROR(dwError);

    // make sure buffer is big enough for base64 encoding
    uiLocalEncodedBufferLen = (uiLocalByteLen/3 + 1) * 4 + 1;
    dwError = VmDirAllocateMemory( uiLocalEncodedBufferLen, (PVOID*)&pLocalEncodedByte );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = sasl_encode64(    pLocalByte,
                                uiLocalByteLen,
                                pLocalEncodedByte,
                                uiLocalEncodedBufferLen,
                                &uiLocalEncodedByteLen);
    BAIL_ON_VMDIR_ERROR(dwError);

    pBervSecretBlob->lberbv_val = pLocalEncodedByte;
    pBervSecretBlob->lberbv_len = uiLocalEncodedByteLen;
    pBervSecretBlob->bOwnBvVal = TRUE;

cleanup:

    VMDIR_SAFE_FREE_MEMORY( pLocalByte );

    return dwError;

error:

    VMDIR_SAFE_FREE_MEMORY( pLocalEncodedByte );
    pBervSecretBlob->bOwnBvVal = FALSE;

    goto cleanup;
}

//////////////////////////////////////////////////
// x = H(salt | H(user | ':' | pass))
//////////////////////////////////////////////////
static
DWORD
_VmDirSRPCalculateX(
    const EVP_MD*   pMD,
    PCSTR           pszUser,
    PVDIR_BERVALUE  pBervPass,
    PVDIR_BERVALUE  pBervSalt,
    BIGNUM*         pX
    )
{
    DWORD       dwError = SASL_OK;
    char        hashBufOne[EVP_MAX_MD_SIZE]={0};
    int	        iHashBufOneLen;
    char        hashBufTwo[EVP_MAX_MD_SIZE]={0};
    int	        iHashBufTwoLen;

    EVP_MD_CTX  EVPMDCtxOne = {0};
    EVP_MD_CTX  EVPMDCtxTwo = {0};

    if ( pMD == NULL || pszUser == NULL || pBervPass == NULL || pBervSalt == NULL || pX == NULL )
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    //  H(user | ':' | pass)
    if ( (EVP_DigestInit  ( &EVPMDCtxOne, pMD) != 1)
         ||
         (EVP_DigestUpdate( &EVPMDCtxOne, pszUser, strlen(pszUser)) != 1)
         ||
         (EVP_DigestUpdate( &EVPMDCtxOne, ":", 1) != 1)
         ||
         (EVP_DigestUpdate( &EVPMDCtxOne, pBervPass->lberbv_val, pBervPass->lberbv_len ) != 1)
         ||
         (EVP_DigestFinal ( &EVPMDCtxOne, &(hashBufOne[0]), &iHashBufOneLen) != 1)
       )
    {
        dwError = VMDIR_ERROR_SRP;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    // x = H(salt | H(user | ':' | pass))
    if ( (EVP_DigestInit  ( &EVPMDCtxTwo, pMD) != 1)
         ||
         (EVP_DigestUpdate( &EVPMDCtxTwo, pBervSalt->lberbv_val, pBervSalt->lberbv_len ) != 1)
         ||
         (EVP_DigestUpdate( &EVPMDCtxTwo, &(hashBufOne[0]), iHashBufOneLen ) != 1)
         ||
         (EVP_DigestFinal ( &EVPMDCtxTwo, &(hashBufTwo[0]), &iHashBufTwoLen) != 1)
       )
    {
        dwError = VMDIR_ERROR_SRP;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    BN_init(pX);

    if (BN_bin2bn(hashBufTwo, iHashBufTwoLen, pX) == NULL)
    {
        dwError = VMDIR_ERROR_SRP;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

error:

    EVP_MD_CTX_cleanup( &EVPMDCtxOne);
    EVP_MD_CTX_cleanup( &EVPMDCtxTwo);

    return dwError;
}

//////////////////////////////////////////////////
// v = g^x % N
//////////////////////////////////////////////////
static
DWORD
_VmDirSRPCalculateV(
    BIGNUM*         pN,
    BIGNUM*         pg,
    PCSTR           pszUPN,
    PVDIR_BERVALUE  pBervPass,
    PVDIR_BERVALUE  pBervSalt,
    BIGNUM*         pv)
{
    DWORD           dwError = 0;
    BIGNUM          x = {0};
    BN_CTX*         pCtx = BN_CTX_new();
    const EVP_MD*   pSHA1Hash = EVP_get_digestbyname( gSASLMDAName[1] );

    if ( pN == NULL || pg == NULL || pszUPN == NULL || pBervPass == NULL || pBervSalt == NULL || pv == NULL )
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if ( pCtx == NULL || pSHA1Hash == NULL )
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = _VmDirSRPCalculateX( pSHA1Hash, pszUPN, pBervPass, pBervSalt, &x);
    BAIL_ON_VMDIR_ERROR(dwError);

    // v = g^x % N
    BN_init(pv);

    if ( BN_mod_exp(pv, pg, &x, pN, pCtx) != 1)
    {
        dwError = VMDIR_ERROR_SRP;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

cleanup:

    if ( pCtx)
    {
        BN_CTX_free( pCtx );
    }

    BN_free( &x );

    return dwError;

error:
    goto cleanup;
}

static
DWORD
_VmDirSRPMakeVandSalt(
    PCSTR           pszUPN,
    PVDIR_BERVALUE  pBervPass,
    PVDIR_BERVALUE  pBervV,
    PVDIR_BERVALUE  pBervSalt
    )
{
    DWORD           dwError = 0;
    PSTR            pLocalV = 0;
    DWORD           dwLocalVLen = 0;
    PSTR            pLocalSalt = 0;
    DWORD           dwLocalSaltLen = 16;  // to match SASL implementation
    BIGNUM*         psrp_N = NULL;
    BIGNUM*         psrp_g = NULL;
    BIGNUM          srp_v = {0};

    if ( pszUPN == NULL || pBervPass == NULL || pBervV == NULL || pBervSalt == NULL )
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    // generate N and g: use 2048 bits to match with SASL implementation
    if ( BN_hex2bn( &(psrp_N), g2048N) == 0 )
    {
        dwError = VMDIR_ERROR_SRP;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if ( BN_hex2bn( &(psrp_g), g2048g ) == 0 )
    {
        dwError = VMDIR_ERROR_SRP;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateMemory( dwLocalSaltLen, (PVOID)&pLocalSalt);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (RAND_pseudo_bytes(pLocalSalt, dwLocalSaltLen) != 1)
    {
        dwError = VMDIR_ERROR_SRP;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    pBervSalt->lberbv_val = pLocalSalt;
    pBervSalt->lberbv_len = (ber_len_t)dwLocalSaltLen;
    pBervSalt->bOwnBvVal  = TRUE;
    pLocalSalt = NULL;

    dwError = _VmDirSRPCalculateV( psrp_N, psrp_g, pszUPN, pBervPass, pBervSalt, &srp_v);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwLocalVLen = BN_num_bytes( &srp_v );
    dwError = VmDirAllocateMemory( dwLocalVLen, (PVOID)&pLocalV);
    BAIL_ON_VMDIR_ERROR(dwError);
    BN_bn2bin( &srp_v, pLocalV);

    pBervV->lberbv_val = pLocalV;
    pBervV->lberbv_len = (ber_len_t)dwLocalVLen;
    pBervV->bOwnBvVal  = TRUE;
    pLocalV = NULL;

cleanup:

    if (psrp_N)
    {
        BN_free(psrp_N);
    }

    if (psrp_g)
    {
        BN_free(psrp_g);
    }

    BN_free(&srp_v);

    return dwError;

error:

    pBervSalt->bOwnBvVal  = FALSE;
    pBervV->bOwnBvVal     = FALSE;
    VMDIR_SAFE_FREE_MEMORY( pLocalV );
    VMDIR_SAFE_FREE_MEMORY( pLocalSalt );

    goto cleanup;
}


#define TEST_HASH      SRP_SHA1
#define TEST_NG        SRP_NG_2048


static DWORD
_VmdirSRPValidateVandSalt(
    PCSTR pUpn,
    PCSTR pPwd,
    PBYTE pSalt,
    DWORD dwSaltLen,
    PBYTE pV,
    DWORD dwVLen)
{
    struct SRPVerifier *ver = NULL;
    struct SRPUser     *usr = NULL;
    PBYTE bytes_A = 0;
    PBYTE bytes_B = 0;
    PBYTE bytes_M    = 0;
    PBYTE bytes_HAMK = 0;
    PCSTR auth_username = NULL;
    PCSTR n_hex         = NULL;
    PCSTR g_hex         = NULL;
    int len_A = 0;
    int len_B = 0;
    int len_M = 0;
    DWORD dwError = 0;

    SRP_HashAlgorithm alg     = TEST_HASH;
    SRP_NGType        ng_type = TEST_NG;

    usr =  srp_user_new(
               alg,
               ng_type,
               pUpn,
               (const unsigned char *) pPwd,
               (int) strlen(pPwd),
               n_hex,
               g_hex);
    if (!usr)
    {
        dwError = ERROR_NO_MEMORY;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    srp_user_start_authentication(
        usr,
        &auth_username,
        (const unsigned char **) &bytes_A,
        &len_A);
    if (len_A == 0 || !bytes_A[0])
    {
        dwError = ERROR_NO_MEMORY;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    /* User -> Host: (username, bytes_A) */
    ver =  srp_verifier_new(
               alg,
               ng_type,
               pUpn,
               pSalt,
               dwSaltLen,
               pV,
               dwVLen,
               bytes_A,
               len_A,
               (const unsigned char **) &bytes_B,
               &len_B,
               n_hex,
               g_hex);
    if (!ver)
    {
        dwError = ERROR_NO_MEMORY;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (!bytes_B)
    {
        /* User SRP-6a safety check violation! */
        dwError = ERROR_BAD_ARGUMENTS;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    /* Host -> User: (pSalt, bytes_B) */
    srp_user_process_challenge(
        usr,
        pSalt,
        dwSaltLen,
        bytes_B,
        len_B, (const unsigned char **) &bytes_M,
        &len_M);
    if (!bytes_M)
    {
        /* User SRP-6a safety check violation! */
        dwError = ERROR_BAD_ARGUMENTS;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    /* User -> Host: (bytes_M) */
    srp_verifier_verify_session(
        ver,
        bytes_M,
        (const unsigned char **) &bytes_HAMK);
    if (!bytes_HAMK)
    {
        /* User authentication failed! */
        dwError = ERROR_INVALID_PASSWORD;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    /* Host -> User: (HAMK) */
    srp_user_verify_session( usr, bytes_HAMK );
    if (!srp_user_is_authenticated(usr))
    {
        /* Server authentication failed! */
        dwError = ERROR_INVALID_PASSWORD;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

error:
    srp_verifier_delete(ver);
    srp_user_delete(usr);
    return dwError;
}
