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
 * Filename: password.c
 *
 * Abstract:
 *
 * password policy enforcement
 *
 */

#include "includes.h"

PVDIR_PASSWORD_HASH_SCHEME _gpDefaultScheme = NULL;

// NOTE: the first scheme in the table is the default scheme
// NOTE: order of fields MUST stay in sync with struct definition...
#define VDIR_PASSWORD_SCHEME_INITIALIZER                \
{                                                       \
    {                                                   \
    VMDIR_SF_INIT(.uId, 0x1),                           \
    VMDIR_SF_INIT(.uDigestSizeInByte, 64),              \
    VMDIR_SF_INIT(.uIteration, 1),                      \
    VMDIR_SF_INIT(.uSaltSizeInByte, 16),                \
    VMDIR_SF_INIT(.bPreSalt, FALSE),                    \
    VMDIR_SF_INIT(.pszName, PASSWD_SCHEME_DEFAULT),     \
    VMDIR_SF_INIT(.pHashFunc, hashFuncSHA512),          \
    VMDIR_SF_INIT(.pNext, NULL)                         \
    },                                                  \
    {                                                   \
    VMDIR_SF_INIT(.uId, 0x2),                           \
    VMDIR_SF_INIT(.uDigestSizeInByte, 32),              \
    VMDIR_SF_INIT(.uIteration, 1),                      \
    VMDIR_SF_INIT(.uSaltSizeInByte, 5),                 \
    VMDIR_SF_INIT(.bPreSalt, TRUE),                     \
    VMDIR_SF_INIT(.pszName, PASSWD_SCHEME_SSO_V1_1),    \
    VMDIR_SF_INIT(.pHashFunc, hashFuncSHA256),          \
    VMDIR_SF_INIT(.pNext, NULL)                         \
    },                                                  \
    {                                                   \
    VMDIR_SF_INIT(.uId, 0x3),                           \
    VMDIR_SF_INIT(.uDigestSizeInByte, 32),              \
    VMDIR_SF_INIT(.uIteration, 1),                      \
    VMDIR_SF_INIT(.uSaltSizeInByte, 0),                 \
    VMDIR_SF_INIT(.bPreSalt, TRUE),                     \
    VMDIR_SF_INIT(.pszName, PASSWD_SCHEME_SSO_V1_2),    \
    VMDIR_SF_INIT(.pHashFunc, hashFuncSHA256),          \
    VMDIR_SF_INIT(.pNext, NULL)                         \
    },                                                  \
    {                                                   \
    VMDIR_SF_INIT(.uId, 0x4),                           \
    VMDIR_SF_INIT(.uDigestSizeInByte, 20),              \
    VMDIR_SF_INIT(.uIteration, 1),                      \
    VMDIR_SF_INIT(.uSaltSizeInByte, 0),                 \
    VMDIR_SF_INIT(.bPreSalt, TRUE),                     \
    VMDIR_SF_INIT(.pszName, PASSWD_SCHEME_SHA1),        \
    VMDIR_SF_INIT(.pHashFunc, hashFuncSHA1),            \
    VMDIR_SF_INIT(.pNext, NULL)                         \
    },                                                  \
}

/*
extern DWORD (*YOUR_HASH_FUNCTION_NAME)(
    PCSTR     pszPassword,      // in:password string
    unit8_t   uPasswordLen,     // in:password string length
    PSTR      pszOutBuf         // caller supply buffer
    );
*/

static
DWORD
hashFuncSHA256(
    PCSTR     pszPassword,      // in:password string
    uint8_t   uPasswordLen,     // in:password string length
    PSTR      pszOutBuf         // caller supply buffer
    );

static
DWORD
hashFuncSHA512(
    PCSTR     pszPassword,      // in:password string
    uint8_t   uPasswordLen,     // in:password string length
    PSTR      pszOutBuf         // caller supply buffer
    );

static
DWORD
hashFuncSHA1(
    PCSTR     pszPassword,      // in:password string
    uint8_t   uPasswordLen,     // in:password string length
    PSTR      pszOutBuf         // caller supply buffer
    );

static
DWORD
VdirPasswordGetScheme(
    PVDIR_BERVALUE                  pBervPasswd,
    PVDIR_PASSWORD_HASH_SCHEME*     ppScheme
    );

static
DWORD
PasswdModifyRequestCheck(
    PVDIR_OPERATION         pOperation,
    PVDIR_MODIFICATION*     ppModNewPasswd,
    PVDIR_MODIFICATION*     ppModOldPasswd
    );

static
DWORD
PasswdModifyMetaDataCreate(
    PVDIR_OPERATION     pOperation,
    PCSTR               pszNewClearPasswd,
    PVDIR_BERVALUE      pCurrentPassValue,
    PVDIR_ENTRY         pEntry,
    BOOLEAN             bIsAdminRole
    );

static
DWORD
OldPasswdRetention(
    PVDIR_OPERATION     pOperation,
    PVDIR_BERVALUE      pOldUserPasswds,
    PVDIR_BERVALUE      pValueToReten,
    int                 iRetentionCnt,
    PVDIR_BERVALUE      pRetendedBlob
    );

static
DWORD
OldPasswdRecycleCheck(
    PCSTR           pszNewClearPasswd,
    PVDIR_BERVALUE  pPriorPasswd
    );

static
DWORD
PasswordStrengthCheck(
    PVDIR_BERVALUE                  pNewPasswd,
    PVDIR_PASSWD_LOCKOUT_POLICY     pPolicy
    );

static
DWORD
PasswordSchemeNameToScheme(
    PCSTR                           pszName,
    PVDIR_PASSWORD_HASH_SCHEME*     ppScheme
    );


static
DWORD
PasswordSchemeCodeToScheme(
    CHAR                            schemeCode,
    PVDIR_PASSWORD_HASH_SCHEME*     ppScheme
    );

/*
 * static initialize gpVdirPasswdSchemeGlobals
 */
DWORD
VmDirPasswordSchemeInit(
    VOID)
{
    DWORD       dwError = 0;
    int         iCnt = 0;
    VDIR_PASSWORD_HASH_SCHEME   initPasswordSchemeTbl[] = VDIR_PASSWORD_SCHEME_INITIALIZER;
    int         iSize  = sizeof(initPasswordSchemeTbl)/sizeof(initPasswordSchemeTbl[0]);
    PVDIR_PASSWORD_HASH_SCHEME  pNewScheme = NULL;
    PVDIR_PASSWORD_HASH_SCHEME  pScheme = NULL;
    DWORD       dwOverrideSchemeId = 0;

    (VOID) VmDirGetRegKeyValueDword(
                VMDIR_CONFIG_PARAMETER_V1_KEY_PATH,
                VMDIR_REG_KEY_OVERRIDE_PASS_SCHEME,
                &dwOverrideSchemeId,
                FALSE);

    for (iCnt = iSize; iCnt > 0; iCnt--)
    {
        dwError = VmDirAllocateMemory(
                    sizeof(*pScheme),
                    (PVOID)&pScheme);
        BAIL_ON_VMDIR_ERROR(dwError);

        pScheme->pHashFunc = initPasswordSchemeTbl[iCnt-1].pHashFunc;
        pScheme->uId = initPasswordSchemeTbl[iCnt-1].uId;
        pScheme->uDigestSizeInByte = initPasswordSchemeTbl[iCnt-1].uDigestSizeInByte;
        pScheme->pszName = initPasswordSchemeTbl[iCnt-1].pszName;
        pScheme->uIteration = initPasswordSchemeTbl[iCnt-1].uIteration;
        pScheme->uSaltSizeInByte = initPasswordSchemeTbl[iCnt-1].uSaltSizeInByte;
        pScheme->bPreSalt = initPasswordSchemeTbl[iCnt-1].bPreSalt;

        assert(pScheme->uDigestSizeInByte <= MAX_PASSWROD_DIGEST_LEN);

        if (dwOverrideSchemeId == (DWORD)iCnt)
        {
            _gpDefaultScheme = pScheme;
            VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, "Override pass scheme to (%d)", dwOverrideSchemeId);
        }

        // Add pScheme to front of list
        pScheme->pNext = pNewScheme;
        pNewScheme = pScheme;
    }

    gpVdirPasswdSchemeGlobals = pScheme; // gpVdirPasswdSchemeGlobals points to the first scheme in table
    if (!_gpDefaultScheme)
    {
        // no reg key override, default to first scheme
        _gpDefaultScheme = gpVdirPasswdSchemeGlobals;
    }

cleanup:

    return dwError;

error:

    goto cleanup;
}

VOID
VmDirPasswordSchemeFree(
    VOID
    )
{
    PVDIR_PASSWORD_HASH_SCHEME   pScheme = gpVdirPasswdSchemeGlobals;

    while (pScheme != NULL)
    {
        PVDIR_PASSWORD_HASH_SCHEME   pSchemeToDelete = pScheme;

        pScheme = pScheme->pNext;
        VMDIR_SAFE_FREE_MEMORY(pSchemeToDelete);
    }

    LwRtlFreeHashTable(&gVdirLockoutCache.pHashTbl);
    gVdirLockoutCache.pHashTbl = NULL;
    _gpDefaultScheme = NULL;
}

DWORD
VmDirGenerateRandomPasswordByDefaultPolicy
(
    PSTR *ppRandPwd)
{
    DWORD   dwError = 0;
    int     iCnt = 0;
    DWORD   pwdLen = VMDIR_KDC_RANDOM_PWD_LEN;
    VDIR_BERVALUE pwdBerv = VDIR_BERVALUE_INIT;
    VDIR_PASSWD_LOCKOUT_POLICY  policy = {0};
    PSTR pRandPwd = NULL;

    *ppRandPwd = NULL;

    if ( gVmdirServerGlobals.systemDomainDN.bvnorm_val == NULL )
    {   // instance not yet initialized
        dwError = ERROR_NOT_JOINED;
        BAIL_ON_VMDIR_ERROR( dwError );
    }

    dwError = VdirGetPasswdAndLockoutPolicy(gVmdirServerGlobals.systemDomainDN.bvnorm_val, &policy);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (VMDIR_KDC_RANDOM_PWD_LEN > policy.iMaxLen)
    {
        pwdLen = policy.iMaxLen;
    } else if (VMDIR_KDC_RANDOM_PWD_LEN < policy.iMinLen)
    {
        pwdLen = policy.iMaxLen;
    }

    do
    {
        VMDIR_SAFE_FREE_MEMORY(pRandPwd);
        dwError = VmKdcGenerateRandomPassword(pwdLen, &pRandPwd);
        BAIL_ON_VMDIR_ERROR( dwError );
        pwdBerv.lberbv.bv_val = pRandPwd;
        pwdBerv.lberbv.bv_len = VmDirStringLenA(pwdBerv.lberbv.bv_val);
        dwError = PasswordStrengthCheck(&pwdBerv, &policy);
    }
    while ( dwError == VMDIR_ERROR_PASSWORD_POLICY_VIOLATION && iCnt++ < VMKDC_RANDPWD_MAX_RETRY);
    BAIL_ON_VMDIR_ERROR(dwError);
    *ppRandPwd = pRandPwd;

cleanup:
    return dwError;

error:
    if ( dwError != ERROR_NOT_JOINED )
    {
        VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "%s: fail to gernerate password error %d", __func__, dwError);
    }
    VMDIR_SAFE_FREE_MEMORY(pRandPwd);
    goto cleanup;
}

/*
 * Compare password in string format from user input against stored digest value
 */
DWORD
VdirPasswordCheck(
    PVDIR_BERVALUE      pBervCred,          // user credential in password string format
    PVDIR_ENTRY         pEntry)
{
    DWORD           dwError = 0;
    PVDIR_ATTRIBUTE pAttrPasswd = NULL;
    uint16_t        uSaltOffset = 0;
    VDIR_BERVALUE   bervDigest = VDIR_BERVALUE_INIT;
    PVDIR_PASSWORD_HASH_SCHEME pPasswdScheme = NULL;

    pAttrPasswd = VmDirFindAttrByName(pEntry, ATTR_USER_PASSWORD);
    if (pAttrPasswd == NULL)
    {
        dwError = VMDIR_ERROR_USER_NO_CREDENTIAL;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    // get the password scheme (user password is a single value attribute)
    dwError = VdirPasswordGetScheme(&(pAttrPasswd->vals[0]), &pPasswdScheme);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (pAttrPasswd->vals[0].lberbv.bv_len != PASSWD_BLOB_LENGTH(pPasswdScheme))
    {
        dwError = VMDIR_ERROR_BAD_ATTRIBUTE_DATA;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    uSaltOffset = PASSWD_SALT_OFFSET(pPasswdScheme);
    dwError = VdirPasswordHash( pPasswdScheme,
                                pBervCred,
                                &bervDigest,
                                pPasswdScheme->uSaltSizeInByte > 0 ?
                                       (PBYTE)(pAttrPasswd->vals[0].lberbv.bv_val + uSaltOffset) : NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

    if ((bervDigest.lberbv.bv_len != pAttrPasswd->vals[0].lberbv.bv_len) ||
        (memcmp(bervDigest.lberbv.bv_val, pAttrPasswd->vals[0].lberbv.bv_val, bervDigest.lberbv.bv_len) != 0) )
    {
        dwError = VMDIR_ERROR_USER_INVALID_CREDENTIAL;
    }
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:

    VmDirFreeBervalContent(&bervDigest);

    return dwError;

error:

    goto cleanup;
}

DWORD
VdirPasswordGenerateSalt(
    PVDIR_PASSWORD_HASH_SCHEME      pScheme,
    PBYTE*                          ppSalt
    )
{
    DWORD   dwError = 0;
    PBYTE   pSalt = NULL;

    assert(pScheme && ppSalt);

    if (pScheme->uSaltSizeInByte > 0)
    {
        dwError = VmDirAllocateMemory( pScheme->uSaltSizeInByte, (PVOID)&pSalt);
        BAIL_ON_VMDIR_ERROR(dwError);

        /*
        From: http://www.openssl.org/docs/crypto/RAND_bytes.html
        RAND_pseudo_bytes() returns 1 if the bytes generated are cryptographically
        strong, 0 otherwise. It return -1 if they are not supported by
        the current RAND method.
        */
        if (RAND_pseudo_bytes(pSalt, pScheme->uSaltSizeInByte) != 1)
        {
            dwError = VMDIR_ERROR_PASSWORD_HASH;
            BAIL_ON_VMDIR_ERROR(dwError);
        }

        *ppSalt = pSalt;
        pSalt   = NULL;
    }

cleanup:

    return dwError;

error:

    VMDIR_SAFE_FREE_MEMORY(pSalt);

    goto cleanup;
}

/*
 * convert password text to hash code
 * pBerv        contain clear text password
 * pHashBerv    contain out hash digest
 * pSaltValue   contain original salt value if exists (optional)
 *
 * To support multiple scheme, we stored password attribute as following -
 * ***********************************************************************
 * one byte scheme id + hash digest + (optional salt)
 * -----------------------------------------------------------------------
 * hash digest   is a fix size blob per scheme
 * optional salt is a fix size blob per scheme
 * ***********************************************************************
 */
DWORD
VdirPasswordHash(
    PVDIR_PASSWORD_HASH_SCHEME  pHashScheme,
    PVDIR_BERVALUE              pBerv,
    PVDIR_BERVALUE              pHashBerv,  // out
    PBYTE                       pSaltByte)  // in: optional
{
    DWORD           dwError = 0;
    PSTR            pszDigest = NULL;
    CHAR            digestBuf[MAX_PASSWROD_DIGEST_LEN + 1] = {0}; // one space for NULL
    uint16_t        uPasswdSize = 0;
    uint16_t        uCnt = 0;
    VDIR_BERVALUE   bervWithSalt = VDIR_BERVALUE_INIT;

    assert(pHashScheme && pBerv && pHashBerv);

    if (pSaltByte)
    {   // add salt to plan password
        dwError = VmDirAllocateMemory(
                    (pBerv->lberbv.bv_len + pHashScheme->uSaltSizeInByte + 1),
                    (PVOID*)&bervWithSalt.lberbv.bv_val);
        BAIL_ON_VMDIR_ERROR(dwError);

        bervWithSalt.bOwnBvVal = TRUE;

        if (pHashScheme->bPreSalt)
        {
            dwError = VmDirCopyMemory( bervWithSalt.lberbv.bv_val,
                                       (pBerv->lberbv.bv_len + pHashScheme->uSaltSizeInByte + 1),
                                       pSaltByte,
                                       pHashScheme->uSaltSizeInByte);
            BAIL_ON_VMDIR_ERROR(dwError);

            dwError = VmDirCopyMemory( (bervWithSalt.lberbv.bv_val + pHashScheme->uSaltSizeInByte),
                                       (pBerv->lberbv.bv_len + 1),
                                       pBerv->lberbv.bv_val,
                                       pBerv->lberbv.bv_len);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
        else
        {
            dwError = VmDirCopyMemory( bervWithSalt.lberbv.bv_val,
                                       (pBerv->lberbv.bv_len + pHashScheme->uSaltSizeInByte + 1),
                                       pBerv->lberbv.bv_val,
                                       pBerv->lberbv.bv_len);
            BAIL_ON_VMDIR_ERROR(dwError);

            dwError = VmDirCopyMemory( (bervWithSalt.lberbv.bv_val + pBerv->lberbv.bv_len),
                                       (pHashScheme->uSaltSizeInByte + 1),
                                       pSaltByte,
                                       pHashScheme->uSaltSizeInByte);
            BAIL_ON_VMDIR_ERROR(dwError);
        }

        bervWithSalt.lberbv.bv_len = pBerv->lberbv.bv_len + pHashScheme->uSaltSizeInByte;
    }

    {
        PVDIR_BERVALUE  pBervTarget = pSaltByte ? &bervWithSalt : pBerv;

        if (pBervTarget->lberbv.bv_len > UINT8_MAX)  //TODO, better to put this restriction in schema?
        {
            dwError = VMDIR_ERROR_PASSWORD_TOO_LONG;
            BAIL_ON_VMDIR_ERROR(dwError);
        }

        dwError = pHashScheme->pHashFunc(   pBervTarget->lberbv.bv_val,
                                            (uint8_t)pBervTarget->lberbv.bv_len ,
                                            &digestBuf[0]);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    for (uCnt = 1; uCnt < pHashScheme->uIteration; uCnt++)
    {
        CHAR    iterBuf[MAX_PASSWROD_DIGEST_LEN + 1] = {0}; // one space for NULL

        dwError = VmDirCopyMemory( &iterBuf[0],
                                   (MAX_PASSWROD_DIGEST_LEN + 1),
                                   &digestBuf[0],
                                   pHashScheme->uDigestSizeInByte);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = pHashScheme->pHashFunc(   &iterBuf[0],
                                            pHashScheme->uDigestSizeInByte,
                                            &digestBuf[0]);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    // scheme code + digest + salt
    uPasswdSize = PASSWD_BLOB_LENGTH(pHashScheme);
    dwError = VmDirAllocateMemory(
                sizeof(char) *  (uPasswdSize + 1),
                (PVOID*)&pszDigest);
    BAIL_ON_VMDIR_ERROR(dwError);

    // construct password value to be stored
    pszDigest[0] = (CHAR)(pHashScheme->uId);

    dwError = VmDirCopyMemory(
                (PVOID)(pszDigest + PASSWD_DIGEST_OFFSET(pHashScheme)),
                uPasswdSize,
                digestBuf,
                pHashScheme->uDigestSizeInByte);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (pSaltByte)
    {
        dwError = VmDirCopyMemory(
                    (PVOID)(pszDigest + PASSWD_SALT_OFFSET(pHashScheme)),
                    (uPasswdSize - pHashScheme->uDigestSizeInByte),
                    (PVOID)(pSaltByte),
                    pHashScheme->uSaltSizeInByte);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    VmDirFreeBervalContent(pHashBerv);
    pHashBerv->lberbv.bv_val = pszDigest;       // pHashBerv takes over pszDigest
    pHashBerv->lberbv.bv_len = uPasswdSize;
    pHashBerv->bOwnBvVal = TRUE;

cleanup:

    VmDirFreeBervalContent(&bervWithSalt);

    return dwError;

error:

    VMDIR_SAFE_FREE_MEMORY(pszDigest);

    goto cleanup;
}

/*
 * perform length checking for supported scheme.
 * this is for import users with existing vmdird supported digest scenario.
 */
DWORD
VdirPasswordVerifySupportedScheme(
    PVDIR_BERVALUE   pBerv)
{
    DWORD                       dwError = 0;
    PVDIR_PASSWORD_HASH_SCHEME  pScheme = NULL;
    PCSTR                       pszErrorContext = NULL;

    assert( pBerv );

    pszErrorContext = "scheme code to name";
    dwError = PasswordSchemeCodeToScheme(pBerv->lberbv.bv_val[0], &pScheme);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (pBerv->lberbv.bv_len != PASSWD_BLOB_LENGTH(pScheme))
    {
        pszErrorContext = "password scheme and blob size mismatch";
        dwError = VMDIR_ERROR_BAD_ATTRIBUTE_DATA;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

cleanup:

    return dwError;

error:

    VMDIR_LOG_DEBUG(LDAP_DEBUG_TRACE, "Verify supported scheme - (%s)", pszErrorContext);

    goto cleanup;
}

/*
 * prepend one byte scheme code to existing digest
 * i.e. pBerv already contain digest value. we need to find the scheme code via name and prepend it.
 */
DWORD
VdirPasswordAddSchemeCode(
    PVDIR_ATTRIBUTE  pAttrSchemeName,
    PVDIR_BERVALUE   pBerv,
    PVDIR_BERVALUE   pOutBerv)
{
    DWORD           dwError = 0;
    PSTR            pszDigest = NULL;
    ber_len_t       uPasswdSize = 0;
    PSTR            pszLocalErrMsg = NULL;
    PVDIR_PASSWORD_HASH_SCHEME  pScheme = NULL;

    assert( pAttrSchemeName && pBerv && pOutBerv );

    dwError = PasswordSchemeNameToScheme(pAttrSchemeName->vals[0].lberbv.bv_val, &pScheme);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (pBerv->lberbv.bv_len != (pScheme->uDigestSizeInByte + pScheme->uSaltSizeInByte))
    {
        dwError = VMDIR_ERROR_BAD_ATTRIBUTE_DATA;
        BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, pszLocalErrMsg, "password scheme and blob size mismatch");
    }

    uPasswdSize = pBerv->lberbv.bv_len + 1; // add one for scheme code

    dwError = VmDirAllocateMemory(
                (sizeof(char) *  (uPasswdSize + 1)),
                (PVOID*)&pszDigest);
    BAIL_ON_VMDIR_ERROR(dwError);

    pszDigest[0] = (CHAR)(pScheme->uId);
    dwError = VmDirCopyMemory(
                (PVOID)(pszDigest+1),
                (sizeof(char) *  (uPasswdSize)),
                pBerv->lberbv.bv_val,
                pBerv->lberbv.bv_len);
    BAIL_ON_VMDIR_ERROR(dwError);

    VmDirFreeBervalContent(pOutBerv);
    pOutBerv->lberbv.bv_val = pszDigest;
    pOutBerv->bOwnBvVal = TRUE;
    pszDigest = NULL;
    pOutBerv->lberbv.bv_len = uPasswdSize;

cleanup:

    VMDIR_SAFE_FREE_MEMORY(pszLocalErrMsg );

    return dwError;

error:

    VMDIR_LOG_DEBUG( LDAP_DEBUG_TRACE, "VdirPasswordAddSchemeCode failed (%u)(%s)", dwError, VDIR_SAFE_STRING(pszLocalErrMsg) );

    VMDIR_SAFE_FREE_MEMORY(pszDigest);

    goto cleanup;
}

/*
 * get default password scheme
 */
PVDIR_PASSWORD_HASH_SCHEME
VdirDefaultPasswordScheme(
    VOID)
{
    assert(_gpDefaultScheme);

    return _gpDefaultScheme;
}

/*
 * 1. password modify request check
 * 2. enforce password strength policy
 * 3. create meta data for password modification (add to pOperation->request.modifyReq.mods)
 */
DWORD
VdirPasswordModifyPreCheck(
    PVDIR_OPERATION     pOperation
    )
{
    DWORD               dwError = 0;
    PSTR                pszLocalErrMsg = NULL;
    PVDIR_MODIFICATION  pModAddPasswd = NULL;
    PVDIR_MODIFICATION  pModDeletePasswd = NULL;
    PVDIR_ENTRY         pEntry = NULL;
    PBYTE               pSalt = NULL;
    PVDIR_ATTRIBUTE     pAttrPasswd = NULL;
    BOOLEAN             bIsAdminRole = FALSE;
    PSTR                pszClearPasswd = NULL;

    assert(pOperation);

    dwError = PasswdModifyRequestCheck(
                    pOperation,
                    &pModAddPasswd,
                    &pModDeletePasswd);
    BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, pszLocalErrMsg, " invalid password modify request");

    if (pModAddPasswd)      // password modify request
    {
        PVDIR_PASSWORD_HASH_SCHEME  pPasswdScheme = VdirDefaultPasswordScheme();
        PVDIR_PASSWORD_HASH_SCHEME  pCurrPasswdScheme = NULL;

        // as password modification is a rare event and there is NO possibility of
        // bad race condition, it is ok to get entry snapshot w/o read locking it
        // till end of this modify OPERATION
        // otherwise, we have to separate password and lock policy logic in various
        // plugin point through out the InternalModifyEntry function.
        dwError = VmDirSimpleNormDNToEntry(
                        BERVAL_NORM_VAL(pOperation->request.modifyReq.dn),
                        &pEntry);
        BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, pszLocalErrMsg, " read entry (%s) failed",
                                     VDIR_SAFE_STRING(BERVAL_NORM_VAL(pOperation->request.modifyReq.dn)));

        // handle krb logic first while we have clear text password
        dwError = VmDirKrbUPNKeySet(  pOperation,
                                      pEntry,
                                      &pModAddPasswd->attr.vals[0]);
        BAIL_ON_VMDIR_ERROR(dwError);

        // handle srp password logic.
        dwError = VmDirSRPSetSecret( pOperation,
                                     pEntry,
                                     &(pModAddPasswd->attr.vals[0]) );
        BAIL_ON_VMDIR_ERROR(dwError);

        pAttrPasswd = VmDirFindAttrByName(pEntry, ATTR_USER_PASSWORD);
        if (pAttrPasswd != NULL && pAttrPasswd->numVals > 0)
        {
            dwError = VdirPasswordGetScheme(&(pAttrPasswd->vals[0]), &pCurrPasswdScheme);
            BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, pszLocalErrMsg, "Password get schema code failed.");
            // if we gets here, i.e. pCurrPasswdScheme, means we have current password in entry
        }

        pOperation->request.modifyReq.bPasswordModify = TRUE;

        dwError = VmDirAllocateStringA(pModAddPasswd->attr.vals[0].lberbv.bv_val, &pszClearPasswd);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VdirPasswordStrengthCheck(
                        &pModAddPasswd->attr.vals[0],
                        NULL,
                        &pOperation->request.modifyReq.dn);
        BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, pszLocalErrMsg, " Password strength check failed");

        // for each new password, generate a different salt
        dwError = VdirPasswordGenerateSalt(pPasswdScheme, &pSalt);
        BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, pszLocalErrMsg, " Password generate salt failed");
;
        dwError = VdirPasswordHash(
                    pPasswdScheme,                   // always use default scheme for new password
                    &pModAddPasswd->attr.vals[0],    // lberbv.bv_val from wire
                    &pModAddPasswd->attr.vals[0],    // lberbv.bv_val replaced by hash code
                    pSalt);
        BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, pszLocalErrMsg, "Password hash failed");

        if (pModDeletePasswd)
        {
            if (pCurrPasswdScheme == NULL)
            {   // no scheme means no current password attribute
                dwError = VMDIR_ERROR_USER_NO_CREDENTIAL;
                BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, pszLocalErrMsg, " No userpassword attribute");
            }

            dwError = VdirPasswordHash(
                        pCurrPasswdScheme,
                        &pModDeletePasswd->attr.vals[0],     // lberbv.bv_val from wire
                        &pModDeletePasswd->attr.vals[0],     // lberbv.bv_val replaced by hash code
                        pCurrPasswdScheme->uSaltSizeInByte > 0 ?
                            (PBYTE)(pAttrPasswd->vals[0].lberbv.bv_val + PASSWD_SALT_OFFSET(pCurrPasswdScheme)) : NULL);
            BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, pszLocalErrMsg, "Password hash failed");
        }

        // check if the bind user has admin right on modify target dn
        dwError = VmDirSrvAccessCheckIsAdminRole(
                        pOperation,
                        BERVAL_NORM_VAL(pOperation->request.modifyReq.dn),
                        &pOperation->conn->AccessInfo,
                        &bIsAdminRole);
        BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, pszLocalErrMsg, " Admin role check failed");

        dwError = PasswdModifyMetaDataCreate(
                        pOperation,
                        (PCSTR)pszClearPasswd,
                        pAttrPasswd ? &pAttrPasswd->vals[0] : NULL,
                        pEntry,
                        bIsAdminRole);
        BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, pszLocalErrMsg, "Create password meta data failed");
    }

cleanup:

    if (pEntry)
    {
        VmDirFreeEntry(pEntry);
    }

    VMDIR_SAFE_FREE_MEMORY(pSalt);
    VMDIR_SAFE_FREE_MEMORY(pszClearPasswd);
    VMDIR_SAFE_FREE_MEMORY(pszLocalErrMsg);

    return dwError;

error:

    VMDIR_SET_LDAP_RESULT_ERROR( &(pOperation->ldapResult), dwError, pszLocalErrMsg );

    goto cleanup;
}

/*
 * Validate Password strength.
 * If pPolicy is not NULL, use it as the policy;
 * Otherwise, use pDNBerv to locate corresponding policy.
 */
DWORD
VdirPasswordStrengthCheck(
    PVDIR_BERVALUE                  pPasswdBerv,
    PVDIR_PASSWD_LOCKOUT_POLICY     pPolicy,       // optional
    PVDIR_BERVALUE                  pDNBerv        // optional
    )
{
    DWORD                       dwError = 0;
    VDIR_PASSWD_LOCKOUT_POLICY  policy = {0};
    PVDIR_PASSWD_LOCKOUT_POLICY pLocalPolicy = NULL;

    assert( pPasswdBerv != NULL );

    if (pPolicy == NULL && pDNBerv == NULL)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (pPolicy != NULL)
    {
        pLocalPolicy = pPolicy;
    }
    else
    {
        dwError = VdirGetPasswdAndLockoutPolicy(
                        BERVAL_NORM_VAL((*pDNBerv)),
                        &policy);
        if (dwError == 0)
        {   // found policy entry
            pLocalPolicy = &policy;
        }
        else if (dwError == VMDIR_ERROR_BACKEND_ENTRY_NOTFOUND)
        {   // no policy entry defined
            dwError = 0;
        }

        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (pLocalPolicy != NULL && pLocalPolicy->bEnabled)
    {
        dwError = PasswordStrengthCheck(pPasswdBerv, pLocalPolicy);
        BAIL_ON_VMDIR_ERROR(dwError);
    }


cleanup:

    return dwError;

error:

    goto cleanup;
}

/*
 * Password set/change scenario -
 * 1. Set Password by admin user- MOD request must have
 *    replace: userpassword
 *    userpassword: NEW_USER_PASSWORD
 * 2. Change Password by self user - MOD request must have -
      delete: userpassword
      userpassword: EXISTING_USER_PASSWORD
      -
      add: userpassword
      userpassword: NEW_USER_PASSWORD
 *
 * TODO, for self change, consider replace as well?
 */
static
DWORD
PasswdModifyRequestCheck(
    PVDIR_OPERATION         pOperation,
    PVDIR_MODIFICATION*     ppModNewPasswd,
    PVDIR_MODIFICATION*     ppModOldPasswd
    )
{
    DWORD           dwError = 0;
    PSTR            pszLocalErrMsg = NULL;
    BOOLEAN         bPasswdDelete    = FALSE;
    BOOLEAN         bPasswdAdd       = FALSE;
    BOOLEAN         bPasswdReplace   = FALSE;
    PVDIR_MODIFICATION   pMod          = NULL;

    assert(pOperation && ppModNewPasswd && ppModOldPasswd);

    for (pMod = pOperation->request.modifyReq.mods; pMod; pMod = pMod->next)
    {
        if (VmDirStringCompareA( pMod->attr.type.lberbv.bv_val, ATTR_USER_PASSWORD, FALSE) == 0)
        {
            switch (pMod->operation)
            {
            case MOD_OP_ADD:
                if (bPasswdAdd || pMod->attr.numVals != 1 || pMod->attr.vals[0].lberbv.bv_len < 1)
                {
                    dwError = LDAP_UNWILLING_TO_PERFORM;
                    BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, pszLocalErrMsg, "Invalid MOD_OP_ADD");
                }
                *ppModNewPasswd = pMod;
                bPasswdAdd = TRUE;
                break;

            case MOD_OP_DELETE:
                if (bPasswdDelete || pMod->attr.numVals != 1 || pMod->attr.vals[0].lberbv.bv_len < 1)
                {
                    dwError = LDAP_UNWILLING_TO_PERFORM;
                    BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, pszLocalErrMsg, "Invalid MOD_OP_DELETE");
                }
                *ppModOldPasswd = pMod;
                bPasswdDelete = TRUE;
                break;

            case MOD_OP_REPLACE:
                if (bPasswdReplace || pMod->attr.numVals != 1 || pMod->attr.vals[0].lberbv.bv_len < 1)
                {
                    dwError = LDAP_UNWILLING_TO_PERFORM;
                    BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, pszLocalErrMsg, "Invalid MOD_REPLACE");
                }
                *ppModNewPasswd = pMod;
                bPasswdReplace = TRUE;
                break;

            default:
                dwError = LDAP_UNWILLING_TO_PERFORM;
                BAIL_ON_VMDIR_ERROR(dwError);
                break;
            }
        }
    }

    // password change: DELETE + ADD must come in pair
    if (( bPasswdAdd && !bPasswdDelete) ||
        (!bPasswdAdd &&  bPasswdDelete))
    {
        dwError = LDAP_UNWILLING_TO_PERFORM;
        BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, pszLocalErrMsg, "Password add and delete must come in pair" );
    }

cleanup:

    VMDIR_SAFE_FREE_MEMORY(pszLocalErrMsg);

    return dwError;

error:

    VMDIR_SET_LDAP_RESULT_ERROR( &(pOperation->ldapResult), dwError, pszLocalErrMsg );

    goto cleanup;
}

/*
 * password and lockout policy meta data include -
 * 1. olduserpassword: for recycle prevention
 * 2. pwdLastSet: for password expiration
 */
static
DWORD
PasswdModifyMetaDataCreate(
    PVDIR_OPERATION     pOperation,
    PCSTR               pszNewClearPasswd,
    PVDIR_BERVALUE      pCurrentPassValue,
    PVDIR_ENTRY         pEntry,
    BOOLEAN             bIsAdminRole
    )
{
    DWORD               dwError = 0;
    PSTR                pszLocalErrMsg = NULL;
    char                pszTimeBuf[GENERALIZED_TIME_STR_LEN + 1] = {0};
    PVDIR_ATTRIBUTE     pOldPasswdAttr = NULL;
    VDIR_BERVALUE       bvRetendedBlob = VDIR_BERVALUE_INIT;

    assert(pOperation && pszNewClearPasswd && pEntry);

    // set pwdLastSet attribute as well
    VmDirStringNPrintFA( pszTimeBuf, sizeof(pszTimeBuf), sizeof(pszTimeBuf) - 1, "%d", (int)time(NULL) );

    dwError = VmDirAppendAMod(
                        pOperation,
                        MOD_OP_REPLACE,
                        ATTR_PWD_LAST_SET,
                        ATTR_PWD_LAST_SET_LEN,
                        pszTimeBuf,
                        VmDirStringLenA(pszTimeBuf));
    BAIL_ON_VMDIR_ERROR(dwError);

    // handle password recycle prevention only for non-admin users
    if ((! bIsAdminRole) && pCurrentPassValue)
    {
        VDIR_PASSWD_LOCKOUT_POLICY  policy = {0};

        pOldPasswdAttr = VmDirFindAttrByName(pEntry, ATTR_OLD_USER_PASSWORD);

        dwError = VdirGetPasswdAndLockoutPolicy(
                        BERVAL_NORM_VAL(pOperation->request.modifyReq.dn),
                        &policy);
        if (dwError == 0 && policy.bEnabled && policy.iRecycleCnt > 0)
        {   // if we have an enabled policy, do recycle related task/check
            dwError = OldPasswdRetention(
                            pOperation,
                            pOldPasswdAttr ? (&(pOldPasswdAttr->vals[0])) : NULL, // current retended blob
                            pCurrentPassValue,              // current (before modify) passwd blob
                            policy.iRecycleCnt,
                            &bvRetendedBlob);               // new retended blob
            BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, pszLocalErrMsg, "Old password save failed");

            dwError = OldPasswdRecycleCheck(
                            pszNewClearPasswd,
                            &bvRetendedBlob);
            BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, pszLocalErrMsg, "Password recycle");
        }
        else if (dwError)
        {
            dwError = 0; // ignore policy lookup error
        }
    }

cleanup:

    VmDirFreeBervalContent(&bvRetendedBlob);
    VMDIR_SAFE_FREE_MEMORY(pszLocalErrMsg);

    return dwError;

error:

    VMDIR_SET_LDAP_RESULT_ERROR( &(pOperation->ldapResult), dwError, pszLocalErrMsg );

    goto cleanup;
}

/*
 * OldUserPassword has this format: newest_digest+.....+oldest_digest
 */
static
DWORD
OldPasswdRetention(
    PVDIR_OPERATION     pOperation,
    PVDIR_BERVALUE      pOldUserPasswds,
    PVDIR_BERVALUE      pValueToReten,
    int                 iRetentionCnt,
    PVDIR_BERVALUE      pRetendedBlob
    )
{
    DWORD       dwError = 0;
    ber_len_t   iBlobLen = 0;
    PSTR        pszBlob = NULL;
    ber_len_t   iKeepSize = 0;
    int         iCnt = 0;

    if ( !pOperation || !pValueToReten )
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if ( pOldUserPasswds )
    {
        for (iCnt = 0; iCnt < (iRetentionCnt - 1) && iKeepSize < pOldUserPasswds->lberbv.bv_len ; iCnt++)
        {
            PVDIR_PASSWORD_HASH_SCHEME pScheme = NULL;

            dwError = PasswordSchemeCodeToScheme(pOldUserPasswds->lberbv.bv_val[iKeepSize], &pScheme);
            BAIL_ON_VMDIR_ERROR(dwError);

            iKeepSize += PASSWD_BLOB_LENGTH(pScheme);
        }

        if (iCnt < iRetentionCnt -1)
        {   // not yet reach max retention count. keep all old passwords.
            iKeepSize = pOldUserPasswds->lberbv.bv_len;
        }

        iBlobLen = pValueToReten->lberbv.bv_len + iKeepSize;

        dwError = VmDirAllocateMemory(
                sizeof(char) * (iBlobLen+1),
                (PVOID*)&pszBlob);
        BAIL_ON_VMDIR_ERROR(dwError);

        // put pValueToReten fist
        dwError = VmDirCopyMemory(
                pszBlob,
                (sizeof(char) * (iBlobLen+1)),
                pValueToReten->lberbv.bv_val,
                pValueToReten->lberbv.bv_len);
        BAIL_ON_VMDIR_ERROR(dwError);

        // append other older passwords
        dwError = VmDirCopyMemory(
                (PVOID)(pszBlob + pValueToReten->lberbv.bv_len),
                (sizeof(char) * ((iBlobLen + 1) - pValueToReten->lberbv.bv_len)),
                pOldUserPasswds->lberbv.bv_val,
                iKeepSize);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirAppendAMod(
                pOperation,
                MOD_OP_REPLACE,
                ATTR_OLD_USER_PASSWORD,
                ATTR_OLD_USER_PASSWORD_LEN,
                pszBlob,
                iBlobLen);
        BAIL_ON_VMDIR_ERROR(dwError);

        if (pRetendedBlob)
        {
            pRetendedBlob->lberbv.bv_val = pszBlob;
            pRetendedBlob->lberbv.bv_len = iBlobLen;
            pRetendedBlob->bOwnBvVal = TRUE;
            pszBlob = NULL;
        }
    }
    else
    {   // add first/current password blob to ATTR_OLD_USER_PASSWORD
        dwError = VmDirAppendAMod(  pOperation,
                                    MOD_OP_REPLACE,
                                    ATTR_OLD_USER_PASSWORD,
                                    ATTR_OLD_USER_PASSWORD_LEN,
                                    pValueToReten->lberbv.bv_val,
                                    pValueToReten->lberbv.bv_len);
        BAIL_ON_VMDIR_ERROR(dwError);

        if (pRetendedBlob)
        {
            pRetendedBlob->lberbv.bv_val = pValueToReten->lberbv.bv_val;
            pRetendedBlob->lberbv.bv_len = pValueToReten->lberbv.bv_len;
            pRetendedBlob->bOwnBvVal = FALSE;
        }
    }

cleanup:

    VMDIR_SAFE_FREE_MEMORY(pszBlob);

    return dwError;

error:

    goto cleanup;
}

/*
 * prevent password recycle
 */
static
DWORD
OldPasswdRecycleCheck(
    PCSTR           pszNewClearPasswd,
    PVDIR_BERVALUE  pPriorPasswd
    )
{
    DWORD           dwError = 0;
    uint16_t        uOffset = 0;
    uint16_t        uPasswdSize = 0;
    VDIR_BERVALUE   bvNewPasswd = VDIR_BERVALUE_INIT;
    VDIR_BERVALUE   bvDigest = VDIR_BERVALUE_INIT;

    assert( pszNewClearPasswd && pPriorPasswd );

    bvNewPasswd.lberbv.bv_val = (PSTR)pszNewClearPasswd;
    bvNewPasswd.lberbv.bv_len = strlen(pszNewClearPasswd);



    for (uOffset = 0; uOffset < pPriorPasswd->lberbv.bv_len; uOffset += uPasswdSize)
    {
        PVDIR_PASSWORD_HASH_SCHEME  pScheme = NULL;
        PSTR                        pszBlob = pPriorPasswd->lberbv.bv_val + uOffset;

        VmDirFreeBervalContent(&bvDigest);

        dwError = PasswordSchemeCodeToScheme(pszBlob[0], &pScheme);
        BAIL_ON_VMDIR_ERROR(dwError);

        uPasswdSize = PASSWD_BLOB_LENGTH(pScheme);

        // regenerate digest using prior scheme and salt
        dwError = VdirPasswordHash(
                        pScheme,
                        &bvNewPasswd,
                        &bvDigest,      // we own bvDigest
                        pScheme->uSaltSizeInByte > 0 ?
                            (PBYTE)(pszBlob + PASSWD_SALT_OFFSET(pScheme)) : NULL);
        BAIL_ON_VMDIR_ERROR(dwError);

        if (memcmp(bvDigest.lberbv.bv_val, pszBlob, uPasswdSize) == 0)
        {
            dwError = VMDIR_ERROR_PASSWORD_POLICY_VIOLATION;
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }


cleanup:

    VmDirFreeBervalContent(&bvDigest);

    return dwError;

error:

    goto cleanup;
}

static
DWORD
hashFuncSHA256(
    PCSTR     pszPassword,      // in:password string
    uint8_t   uPasswordLen,     // in:password string length
    PSTR      pszOutBuf         // caller supply buffer
    )
{
    DWORD   dwError = 0;

    // TODO: error handling?
    SHA256(pszPassword, uPasswordLen, pszOutBuf);

    return dwError;
}

static
DWORD
hashFuncSHA512(
    PCSTR     pszPassword,      // in:password string
    uint8_t   uPasswordLen,     // in:password string length
    PSTR      pszOutBuf         // caller supply buffer
    )
{
    DWORD       dwError = 0;

    // TODO: error handling?
    SHA512(pszPassword, uPasswordLen, pszOutBuf);

    return dwError;
}

static
DWORD
hashFuncSHA1(
             PCSTR     pszPassword,      // in:password string
             uint8_t   uPasswordLen,     // in:password string length
             PSTR      pszOutBuf         // caller supply buffer
             )
{
    DWORD       dwError = 0;

    SHA1(pszPassword, uPasswordLen, pszOutBuf);

    return dwError;
}

static
DWORD
PasswordStrengthCheck(
    PVDIR_BERVALUE                  pNewPasswd,
    PVDIR_PASSWD_LOCKOUT_POLICY     pPolicy
    )
{
    DWORD       dwError = 0;
    int         iCnt = 0;
    int         iSameCharCnt = 0;
    int         iUpperCnt = 0;
    int         iLowerCnt = 0;
    int         iNumericCnt = 0;
    int         iSPCharCnt = 0;
    int         iAlphaCnt = 0;

    assert(pNewPasswd && pPolicy);

    if (pNewPasswd->lberbv.bv_len > pPolicy->iMaxLen ||
        pNewPasswd->lberbv.bv_len < pPolicy->iMinLen)
    {
        dwError = VMDIR_ERROR_PASSWORD_POLICY_VIOLATION;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    for (iCnt = 0; iCnt < pNewPasswd->lberbv.bv_len; iCnt++)
    {
        if (VMDIR_ASCII_LOWER(pNewPasswd->lberbv.bv_val[iCnt]))
        {
            iAlphaCnt++;
            iLowerCnt++;
        }

        if (VMDIR_ASCII_UPPER(pNewPasswd->lberbv.bv_val[iCnt]))
        {
            iAlphaCnt++;
            iUpperCnt++;
        }

        if (VMDIR_ASCII_DIGIT(pNewPasswd->lberbv.bv_val[iCnt]))
        {
            iNumericCnt++;
        }

        if ( pPolicy->specialChars[0] != '\0' ) // user defined special chars
        {
            if (VmDirStringChrA (pPolicy->specialChars, pNewPasswd->lberbv.bv_val[iCnt]) != NULL )
            {
                iSPCharCnt++;
            }
        }
        else if (VMDIR_PASSWD_SP_CHAR(pNewPasswd->lberbv.bv_val[iCnt])) // default special chars
        {
            iSPCharCnt++;
        }

        if (pNewPasswd->lberbv.bv_val[iCnt] == pNewPasswd->lberbv.bv_val[iCnt+1])
        {
            iSameCharCnt++;
            if (iSameCharCnt >= pPolicy->iMaxSameAdjacentCharCnt)
            {
                dwError = VMDIR_ERROR_PASSWORD_POLICY_VIOLATION;
                BAIL_ON_VMDIR_ERROR(dwError);
            }
        }

        if (pNewPasswd->lberbv.bv_val[iCnt] != pNewPasswd->lberbv.bv_val[iCnt+1])
        {
            iSameCharCnt = 0;
        }
    }

    if (iSPCharCnt  < pPolicy->iMinSpecialCharCnt        ||
        iNumericCnt < pPolicy->iMinNumericCnt            ||
        iUpperCnt   < pPolicy->iMinUpperCaseCnt          ||
        iLowerCnt   < pPolicy->iMinLowerCaseCnt          ||
        iAlphaCnt   < pPolicy->iMinAlphaCnt)
    {
        dwError = VMDIR_ERROR_PASSWORD_POLICY_VIOLATION;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

error:

    return dwError;
}

static
DWORD
PasswordSchemeNameToScheme(
    PCSTR                           pszName,
    PVDIR_PASSWORD_HASH_SCHEME*     ppScheme
    )
{
    DWORD   dwError = 0;
    PVDIR_PASSWORD_HASH_SCHEME  pScheme = gpVdirPasswdSchemeGlobals;

    assert( pszName && ppScheme );

    while (pScheme)
    {
        if (VmDirStringCompareA(pScheme->pszName, pszName, FALSE) == 0)
        {
            break;
        }

        pScheme = pScheme->pNext;
    }

    if (! pScheme)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    *ppScheme = pScheme;

error:

    return dwError;
}

static
DWORD
PasswordSchemeCodeToScheme(
    CHAR                            schemeCode,
    PVDIR_PASSWORD_HASH_SCHEME*     ppScheme
    )
{
    DWORD   dwError = 0;
    PVDIR_PASSWORD_HASH_SCHEME  pScheme = gpVdirPasswdSchemeGlobals;

    assert( ppScheme );

    while (pScheme)
    {
        if (pScheme->uId == (uint8_t) schemeCode)
        {
            break;
        }

        pScheme = pScheme->pNext;
    }

    if (! pScheme)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    *ppScheme = pScheme;

error:

    return dwError;
}

static
DWORD
VdirPasswordGetScheme(
    PVDIR_BERVALUE                  pBervPasswd,
    PVDIR_PASSWORD_HASH_SCHEME*     ppScheme
    )
{
    DWORD       dwError = 0;
    PVDIR_PASSWORD_HASH_SCHEME  pLocalScheme = NULL;

    assert (pBervPasswd && ppScheme);

    if (pBervPasswd->lberbv.bv_len <=0)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = PasswordSchemeCodeToScheme(pBervPasswd->lberbv.bv_val[0], &pLocalScheme);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppScheme = pLocalScheme;

cleanup:

    return dwError;

error:

    *ppScheme = NULL;

    goto cleanup;
}
