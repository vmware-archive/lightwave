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
 * Module Name: Directory middle-layer
 *
 * Filename: structs.h
 *
 * Abstract:
 *
 *
 * Private Structures
 *
 */

typedef struct _VDIR_OP_PLUGIN_INFO
{
    // NOTE: order of fields MUST stay in sync with struct initializer...
    BOOLEAN                         bCallAlways;
    // call if bCallAlways == TRUE or prior pPluginFunc call return 0/SUCCESS
    VDIR_OP_PLUGIN_FUNCTION         pPluginFunc;
    struct _VDIR_OP_PLUGIN_INFO*    pNext;

} VDIR_OP_PLUGIN_INFO;

// hash function prototype
typedef DWORD (*VDIR_HASH_FUNCTION)(
                    PCSTR     pszPassword,      // in:password string
                    uint8_t   uPasswordLen,     // in:password string length
                    PSTR      pszOutBuf         // caller supply buffer
                    );

typedef struct _VDIR_PASSWORD_HASH_SCHEME
{
    // NOTE: order of fields MUST stay in sync with struct initializer...
    uint8_t                             uId;
    uint8_t                             uDigestSizeInByte;
    uint16_t                            uIteration;
    uint8_t                             uSaltSizeInByte;
    BOOLEAN                             bPreSalt;
    PCSTR                               pszName;
    VDIR_HASH_FUNCTION                  pHashFunc;
    struct _VDIR_PASSWORD_HASH_SCHEME*  pNext;

} VDIR_PASSWORD_HASH_SCHEME, *PVDIR_PASSWORD_HASH_SCHEME;

#define MAX_PASSWORD_SPECIAL_CHARS  32
typedef struct _VDIR_PASSWD_LOCKOUT_POLICY
{
    BOOLEAN bEnabled;
    int     iAutoUnlockIntervalSec;
    int     iFailedAttemptIntervalSec;
    int     iMaxFailedAttempt;

    int     iExpireInDay;
    int     iRecycleCnt;

    int     iMaxSameAdjacentCharCnt;
    int     iMinSpecialCharCnt;
    int     iMinNumericCnt;
    int     iMinUpperCaseCnt;
    int     iMinLowerCaseCnt;
    int     iMinAlphaCnt;
    int     iMinLen;
    int     iMaxLen;
    char    specialChars[MAX_PASSWORD_SPECIAL_CHARS+1];

} VDIR_PASSWD_LOCKOUT_POLICY, *PVDIR_PASSWD_LOCKOUT_POLICY;

typedef struct _VDIR_LOCKOUT_REC
{
    PSTR        pszNormDN;          // hash key, may consider using eid?

    time_t      firstFailedTime;
    time_t      lockoutTime;
    int         iFailedAttempt;
    int         iExpireInDay;
    BOOLEAN     bAutoUnlockAccount;

    // could remove value from lockout policy when polices are cached
    // otherwise, keep these value for performance
    int         iMaxFailedAttempt;          // value from lockout policy
    int         iFailedAttemptIntervalSec;  // value from lockout policy
    int         iAutoUnlockIntervalSec;     // value from lockout policy

    LW_HASHTABLE_NODE       Node;

} VDIR_LOCKOUT_REC, *PVDIR_LOCKOUT_REC;

typedef struct _VDIR_LOCKOUT_CACHE
{
    // NOTE: order of fields MUST stay in sync with struct initializer...
    PVMDIR_MUTEX        mutex;
    PLW_HASHTABLE    pHashTbl;

} VDIR_LOCKOUT_CACHE, *PVDIR_LOCKOUT_CACHE;

typedef enum _VDIR_SALS_STATUS
{
    SASL_STATUS_NONE = 0,
    SASL_STATUS_IN_PROGRESS,
    SASL_STATUS_DONE
} VDIR_SASL_STATUS;

typedef struct _VDIR_SASL_BIND_INFO
{
    Sockbuf*            pSockbuf;
    VDIR_SASL_STATUS    saslStatus;
    sasl_conn_t*        pSaslCtx;           // sasl context
    sasl_callback_t*    pSessionCB;         // per session CB
    VDIR_BERVALUE       bvMechnism;         // sasl mechanism for this pSaslCtx
    PSTR                pszBindUserName;    // krb UPN
    sasl_ssf_t          saslSSF;            // sasl security strength factor
    DWORD               vmdirCode;          // vmdir level error code
} VDIR_SASL_BIND_INFO;


typedef DWORD (*VDIR_COMPUTED_ATTRIBUE_FUNCTION)(
                PVDIR_OPERATION     pOperation,
                PVDIR_ENTRY         pEntry,
                PVDIR_ATTRIBUTE*    ppComputedAttr
                );

typedef struct _VDIR_DERIVED_ATTRIBUTE_INFO
{
    PCSTR                               pszComputedAttributeName;
    VDIR_COMPUTED_ATTRIBUE_FUNCTION     pfnComputedAttr;

} VDIR_COMPUTED_ATTRIBUTE_INFO, *PVDIR_COMPUTED_ATTRIBUTE_INFO;

