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
    USHORT                          usOpMask;
    BOOLEAN                         bSkipOnError;
    // call if
    // 1) usOpMask & pOperation->opType == TRUE and
    // 2) bSkipOnError == FALSE or prior pPluginFunc call return 0/SUCCESS
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

typedef struct _VDIR_PAGED_SEARCH_ENTRY_LIST
{
    ENTRYID *pEntryIds;
    DWORD dwCount;
} VDIR_PAGED_SEARCH_ENTRY_LIST, *PVDIR_PAGED_SEARCH_ENTRY_LIST;

typedef struct _VDIR_PAGED_SEARCH_RECORD
{
    //
    // Who's using the object.
    //
    DWORD dwRefCount;

    //
    // The number of entries in each page.
    //
    DWORD dwPageSize;

    //
    // Number of candidates we've processed.
    //
    DWORD dwCandidatesProcessed;

    //
    // Key for the hash table. Sent in the cookie to the client.
    //
    PSTR pszGuid;

    //
    // This is the original, complete list of candidates.
    //
    PVDIR_CANDIDATES pTotalCandidates;

    //
    // We cache the filter information so we don't need to re-parse it
    // every time. The candidates pointer in this pFilter will be updated
    // to include the right candidates from the total list above.
    //
    PVDIR_FILTER pFilter;

    LW_HASHTABLE_NODE Node;

    //
    // The queue of vetted ENTRYIDs. Each entry will be a page's worth of
    // IDs.
    //
    PDEQUE pQueue;
    PVMDIR_MUTEX mutex;
    PVMDIR_COND pDataAvailable;

    //
    // This is the information of our worker thread. We'll use this to signal
    // the thread that we've read all the data and it can now exit.
    //
    PVDIR_THREAD_INFO pThreadInfo;

    //
    // Indicates if the worker thread has completed processing all available
    // data. The client hasn't necessarily read it all yet, though.
    //
    BOOLEAN bProcessingCompleted;
    //
    // Indicates that the client has read all the data. This lets the worker
    // thread know that it can exit.
    //
    BOOLEAN bSearchCompleted;
} VDIR_PAGED_SEARCH_RECORD, *PVDIR_PAGED_SEARCH_RECORD;

typedef struct _VDIR_PAGED_SEARCH_CACHE
{
    // NOTE: order of fields MUST stay in sync with struct initializer...
    PVMDIR_MUTEX        mutex;
    PLW_HASHTABLE       pHashTbl;
    DWORD               dwTimeoutPeriod;
} VDIR_PAGED_SEARCH_CACHE, *PVDIR_PAGED_SEARCH_CACHE;

typedef struct _VDIR_LOCKOUT_CACHE
{
    // NOTE: order of fields MUST stay in sync with struct initializer...
    PVMDIR_MUTEX        mutex;
    PLW_HASHTABLE       pHashTbl;
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

typedef enum _VDIR_SPECIAL_SEARCH_ENTRY_TYPE
{
    SPECIAL_SEARCH_ENTRY_TYPE_DSE_ROOT,
    SPECIAL_SEARCH_ENTRY_TYPE_SCHEMA_ENTRY,
    SPECIAL_SEARCH_ENTRY_TYPE_SERVER_STATUS,
    SPECIAL_SEARCH_ENTRY_TYPE_RAFT_STATUS,
    REGULAR_SEARCH_ENTRY_TYPE
} VDIR_SPECIAL_SEARCH_ENTRY_TYPE;

