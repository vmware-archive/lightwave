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
 * Module Name: VDIR
 *
 * Filename: vmdirdefines.h
 *
 * Abstract:
 *
 * Common macros
 *
 *
 */

#ifndef __VDIR_DEFINE_H__
#define __VDIR_DEFINE_H__

#ifdef __cplusplus
extern "C" {
#endif

#define VMDIR_PCSTR_UNKNOWN "unknown"

#ifdef _WIN32
    #define HAVE_LMDB_H
    #define PCVOID const PVOID
    #define ssize_t SSIZE_T

    /* LDAP Content Synchronization Operation -- RFC 4533 */
    #define LDAP_SYNC_OID			"1.3.6.1.4.1.4203.1.9.1"
    #define LDAP_CONTROL_SYNC		LDAP_SYNC_OID ".1"
    #define LDAP_CONTROL_SYNC_STATE	LDAP_SYNC_OID ".2"
    #define LDAP_CONTROL_SYNC_DONE	LDAP_SYNC_OID ".3"
    #define LDAP_SYNC_INFO			LDAP_SYNC_OID ".4"

    #define LDAP_SYNC_NONE					0x00
    #define LDAP_SYNC_REFRESH_ONLY			0x01
    #define LDAP_SYNC_RESERVED				0x02
    #define LDAP_SYNC_REFRESH_AND_PERSIST	0x03

//
    /* LDAP Request Messages */
    #define LDAP_REQ_BIND		((ber_tag_t) 0x60U)	/* application + constructed */
    #define LDAP_REQ_UNBIND		((ber_tag_t) 0x42U)	/* application + primitive   */
    #define LDAP_REQ_SEARCH		((ber_tag_t) 0x63U)	/* application + constructed */
    #define LDAP_REQ_MODIFY		((ber_tag_t) 0x66U)	/* application + constructed */
    #define LDAP_REQ_ADD		((ber_tag_t) 0x68U)	/* application + constructed */
    #define LDAP_REQ_DELETE		((ber_tag_t) 0x4aU)	/* application + primitive   */
    #define LDAP_REQ_MODDN		((ber_tag_t) 0x6cU)	/* application + constructed */
    #define LDAP_REQ_MODRDN		LDAP_REQ_MODDN
    #define LDAP_REQ_RENAME		LDAP_REQ_MODDN
    #define LDAP_REQ_COMPARE	((ber_tag_t) 0x6eU)	/* application + constructed */
    #define LDAP_REQ_ABANDON	((ber_tag_t) 0x50U)	/* application + primitive   */
    #define LDAP_REQ_EXTENDED	((ber_tag_t) 0x77U)	/* application + constructed */

    /*
     * LDAP Result Codes
     */
    #define LDAP_SUCCESS				0x00

    #define LDAP_RANGE(n,x,y)	(((x) <= (n)) && ((n) <= (y)))

    #ifndef LBERLIB_ONLY

    #define LDAP_OPERATIONS_ERROR		0x01
    #define LDAP_PROTOCOL_ERROR			0x02
    #define LDAP_TIMELIMIT_EXCEEDED		0x03
    #define LDAP_SIZELIMIT_EXCEEDED		0x04
    #define LDAP_COMPARE_FALSE			0x05
    #define LDAP_COMPARE_TRUE			0x06
    #define LDAP_AUTH_METHOD_NOT_SUPPORTED	0x07
    #define LDAP_STRONG_AUTH_NOT_SUPPORTED	LDAP_AUTH_METHOD_NOT_SUPPORTED
    #define LDAP_STRONG_AUTH_REQUIRED	0x08
    #define LDAP_STRONGER_AUTH_REQUIRED	LDAP_STRONG_AUTH_REQUIRED
    #define LDAP_PARTIAL_RESULTS		0x09	/* LDAPv2+ (not LDAPv3) */

    #define	LDAP_REFERRAL				0x0a /* LDAPv3 */
    #define LDAP_ADMINLIMIT_EXCEEDED	0x0b /* LDAPv3 */
    #define	LDAP_UNAVAILABLE_CRITICAL_EXTENSION	0x0c /* LDAPv3 */
    #define LDAP_CONFIDENTIALITY_REQUIRED	0x0d /* LDAPv3 */
    #define	LDAP_SASL_BIND_IN_PROGRESS	0x0e /* LDAPv3 */

    #endif

    #define LDAP_ATTR_ERROR(n)	LDAP_RANGE((n),0x10,0x15) /* 16-21 */

    #ifndef LBERLIB_ONLY

    #define LDAP_NO_SUCH_ATTRIBUTE		0x10
    #define LDAP_UNDEFINED_TYPE			0x11
    #define LDAP_INAPPROPRIATE_MATCHING	0x12
    #define LDAP_CONSTRAINT_VIOLATION	0x13
    #define LDAP_TYPE_OR_VALUE_EXISTS	0x14
    #define LDAP_INVALID_SYNTAX			0x15

    #endif

    #define LDAP_NAME_ERROR(n)	LDAP_RANGE((n),0x20,0x24) /* 32-34,36 */

    #ifndef LBERLIB_ONLY

    #define LDAP_NO_SUCH_OBJECT			0x20
    #define LDAP_ALIAS_PROBLEM			0x21
    #define LDAP_INVALID_DN_SYNTAX		0x22
    #define LDAP_IS_LEAF				0x23 /* not LDAPv3 */
    #define LDAP_ALIAS_DEREF_PROBLEM	0x24

    #endif

    #define LDAP_SECURITY_ERROR(n)	LDAP_RANGE((n),0x2F,0x32) /* 47-50 */

    #ifndef LBERLIB_ONLY

    #define LDAP_X_PROXY_AUTHZ_FAILURE	0x2F /* LDAPv3 proxy authorization */
    #define LDAP_INAPPROPRIATE_AUTH		0x30
    #define LDAP_INVALID_CREDENTIALS	0x31
    #define LDAP_INSUFFICIENT_ACCESS	0x32

    #endif

    #define LDAP_SERVICE_ERROR(n)	LDAP_RANGE((n),0x33,0x36) /* 51-54 */

    #ifndef LBERLIB_ONLY

    #define LDAP_BUSY					0x33
    #define LDAP_UNAVAILABLE			0x34
    #define LDAP_UNWILLING_TO_PERFORM	0x35
    #define LDAP_LOOP_DETECT			0x36

    #endif

    #define LDAP_UPDATE_ERROR(n)	LDAP_RANGE((n),0x40,0x47) /* 64-69,71 */

    #ifndef LBERLIB_ONLY

    #define LDAP_NAMING_VIOLATION		0x40
    #define LDAP_OBJECT_CLASS_VIOLATION	0x41
    #define LDAP_NOT_ALLOWED_ON_NONLEAF	0x42
    #define LDAP_NOT_ALLOWED_ON_RDN		0x43
    #define LDAP_ALREADY_EXISTS			0x44
    #define LDAP_NO_OBJECT_CLASS_MODS	0x45
    #define LDAP_RESULTS_TOO_LARGE		0x46 /* CLDAP */
    #define LDAP_AFFECTS_MULTIPLE_DSAS	0x47

    #define LDAP_VLV_ERROR				0x4C

    #endif

	/* general stuff */
    #define LDAP_TAG_MESSAGE	((ber_tag_t) 0x30U)	/* constructed + 16 */
    #define LDAP_TAG_MSGID		((ber_tag_t) 0x02U)	/* integer */

    #define LDAP_TAG_LDAPDN		((ber_tag_t) 0x04U)	/* octet string */
    #define LDAP_TAG_LDAPCRED	((ber_tag_t) 0x04U)	/* octet string */

    #define LDAP_TAG_CONTROLS	((ber_tag_t) 0xa0U)	/* context specific + constructed + 0 */
    #define LDAP_TAG_REFERRAL	((ber_tag_t) 0xa3U)	/* context specific + constructed + 3 */

    #define LDAP_TAG_NEWSUPERIOR	((ber_tag_t) 0x80U)	/* context-specific + primitive + 0 */

    #define LDAP_TAG_EXOP_REQ_OID   ((ber_tag_t) 0x80U)	/* context specific + primitive */
    #define LDAP_TAG_EXOP_REQ_VALUE ((ber_tag_t) 0x81U)	/* context specific + primitive */
    #define LDAP_TAG_EXOP_RES_OID   ((ber_tag_t) 0x8aU)	/* context specific + primitive */
    #define LDAP_TAG_EXOP_RES_VALUE ((ber_tag_t) 0x8bU)	/* context specific + primitive */

    #define LDAP_TAG_IM_RES_OID   ((ber_tag_t) 0x80U)	/* context specific + primitive */
    #define LDAP_TAG_IM_RES_VALUE ((ber_tag_t) 0x81U)	/* context specific + primitive */

    #define LDAP_TAG_SASL_RES_CREDS	((ber_tag_t) 0x87U)	/* context specific + primitive */


//	#define SOCKBUF_VALID( sb )	( (sb)->sb_valid == LBER_VALID_SOCKBUF )
#endif

#define VMDIR_MIN(a, b) ((a) < (b) ? (a) : (b))
#define VMDIR_MAX(a, b) ((a) > (b) ? (a) : (b))

#define BERVAL_NORM_VAL(BERV)                \
    ((BERV).bvnorm_val ? (BERV).bvnorm_val : (BERV).lberbv.bv_val)

#define BERVAL_NORM_LEN(BERV)                \
    ((BERV).bvnorm_len != 0 ? (BERV).bvnorm_len : (BERV).lberbv.bv_len)

#define TRANSFER_BERVALUE(fromBerv, toBerv) \
    {                                                       \
        (toBerv).lberbv.bv_val = (fromBerv).lberbv.bv_val;  \
        (toBerv).lberbv.bv_len = (fromBerv).lberbv.bv_len;  \
        (toBerv).bOwnBvVal     = (fromBerv).bOwnBvVal;      \
        (toBerv).bvnorm_val    = (fromBerv).bvnorm_val;     \
        (toBerv).bvnorm_len    = (fromBerv).bvnorm_len;     \
        memset(&(fromBerv), 0, sizeof((fromBerv)));         \
    }


#ifndef _WIN32
#define VMDIR_SF_INIT( fieldName, fieldValue ) fieldName = fieldValue
#else
#define VMDIR_SF_INIT( fieldName, fieldValue ) fieldValue
#endif

#ifndef _WIN32

//TODO, make cert/key configurable via registry
#define RSA_SERVER_CERT VMDIR_CONFIG_DIR VMDIR_PATH_SEPARATOR_STR "vmdircert.pem"
#define RSA_SERVER_KEY  VMDIR_CONFIG_DIR VMDIR_PATH_SEPARATOR_STR "vmdirkey.pem"

#endif

#define VMDIR_SAFE_FREE_STRINGA(PTR)      \
    do {                                  \
        if ((PTR)) {                      \
            VmDirFreeStringA(PTR);        \
            (PTR) = NULL;                 \
        }                                 \
    } while(0)

#define VMDIR_SECURE_FREE_STRINGA(PTR)    \
    do {                                  \
        if ((PTR)) {                      \
            if (*(PTR)) {                 \
                memset(PTR, 0, strlen(PTR)); \
            }                             \
            VmDirFreeStringA(PTR);        \
            (PTR) = NULL;                 \
        }                                 \
    } while(0)

#define VMDIR_SAFE_FREE_MEMORY(PTR)       \
    do {                                  \
        if ((PTR)) {                      \
            VmDirFreeMemory(PTR);         \
            (PTR) = NULL;                 \
        }                                 \
    } while(0)

#define VMDIR_SAFE_FREE_MUTEX(mutex)      \
    do {                                  \
        if ((mutex)) {                    \
            VmDirFreeMutex(mutex);        \
            (mutex) = NULL;               \
        }                                 \
    } while(0)

#define VMDIR_SAFE_FREE_RWLOCK(lock)      \
    do {                                  \
        if ((lock)) {                     \
            VmDirFreeRWLock(lock);        \
            (lock) = NULL;                \
        }                                 \
    } while(0)

#define VMDIR_SAFE_FREE_CONDITION(cond)   \
    do {                                  \
        if ((cond)) {                     \
            VmDirFreeCondition(cond);     \
            (cond) = NULL;                \
        }                                 \
    } while(0)

#define VMDIR_SAFE_FREE_SYNCCOUNTER(pSyncCounter)   \
    do {                                            \
        if ((pSyncCounter)) {                       \
            VmDirFreeSyncCounter(pSyncCounter);     \
            (pSyncCounter) = NULL;                  \
        }                                           \
    } while(0)

#define VMDIR_LOCK_MUTEX(bInLock, mutex)        \
    do {                                        \
        if (!(bInLock))                         \
        {                                       \
            if (VmDirLockMutex(mutex) == 0)     \
            {                                   \
                (bInLock) = TRUE;               \
            }                                   \
        }                                       \
    } while (0)

#define VMDIR_UNLOCK_MUTEX(bInLock, mutex)      \
    do {                                        \
        if ((bInLock))                          \
        {                                       \
            if (VmDirUnLockMutex(mutex) == 0)   \
            {                                   \
                (bInLock) = FALSE;              \
            }                                   \
        }                                       \
    } while (0)

#define VMDIR_RWLOCK_READLOCK(bInLock, lock, dwMilliSec)        \
    do {                                                        \
        if (!(bInLock))                                         \
        {                                                       \
            if (VmDirRWLockReadLock(lock, dwMilliSec) == 0)     \
            {                                                   \
                (bInLock) = TRUE;                               \
            }                                                   \
        }                                                       \
    } while (0)

#define VMDIR_RWLOCK_WRITELOCK(bInLock, lock, dwMilliSec)       \
    do {                                                        \
        if (!(bInLock))                                         \
        {                                                       \
            if (VmDirRWLockWriteLock(lock, dwMilliSec) == 0)    \
            {                                                   \
                (bInLock) = TRUE;                               \
            }                                                   \
        }                                                       \
    } while (0)

#define VMDIR_RWLOCK_UNLOCK(bInLock, lock)          \
    do {                                            \
        if ((bInLock))                              \
        {                                           \
            if (VmDirRWLockUnlock(lock) == 0)       \
            {                                       \
                (bInLock) = FALSE;                  \
            }                                       \
        }                                           \
    } while (0)

#define BAIL_WITH_VMDIR_ERROR(dwError, ERROR_CODE)                          \
    do {                                                                    \
        dwError = ERROR_CODE;                                               \
        assert(dwError != 0);                                               \
        VMDIR_LOG_DEBUG(VMDIR_LOG_MASK_ALL, "[%s,%d]", __FILE__, __LINE__); \
        goto error;                                                         \
    } while (0)

#define BAIL_ON_VMDIR_ERROR(dwError) \
    if (dwError)                                                            \
    {                                                                       \
        VMDIR_LOG_DEBUG(VMDIR_LOG_MASK_ALL, "[%s,%d]", __FILE__, __LINE__); \
        goto error;                                                         \
    }

#define BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, pszErrMsg, Format, ... )      \
    if (dwError)                                                            \
    {                                                                       \
        if (pszErrMsg == NULL)                                              \
        {                                                                   \
            VmDirAllocateStringPrintf(                                      \
                            &(pszErrMsg),                                   \
                            Format,                                         \
                            ##__VA_ARGS__);                                 \
        }                                                                   \
        VMDIR_LOG_DEBUG( VMDIR_LOG_MASK_ALL, "[%s,%d]",__FILE__, __LINE__); \
        goto error;                                                         \
    }

#define BAIL_ON_VMDIR_ERROR_IF(condition) \
    if (condition)                                                          \
    {                                                                       \
        VMDIR_LOG_DEBUG( VMDIR_LOG_MASK_ALL, "[%s,%d]",__FILE__, __LINE__); \
        goto error;                                                         \
    }

#define BAIL_ON_VMDIR_INVALID_POINTER(p, errCode)     \
        if (p == NULL) {                          \
            errCode = ERROR_INVALID_PARAMETER;    \
            BAIL_ON_VMDIR_ERROR(errCode);          \
        }

#define BAIL_ON_INVALID_ACCESSINFO(pAccessInfo, errCode)     \
        if (pAccessInfo == NULL || VmDirIsFailedAccessInfo(pAccessInfo)) {  \
           errCode = VMDIR_ERROR_INSUFFICIENT_ACCESS;         \
           BAIL_ON_VMDIR_ERROR(errCode);          \
        }

// see ldap.h for other LDAP error code and range definitions.
#define LDAP_SERVER_ERROR(n)        LDAP_RANGE((n),0x01,0x0e) /* 1 ~ 15 */

#define NOT_LDAP_ERROR_SPACE(n)                           \
        (                                                 \
        !(LDAP_SERVER_ERROR(n))   &&                      \
        !(LDAP_ATTR_ERROR(n))     &&                      \
        !(LDAP_NAME_ERROR(n))     &&                      \
        !(LDAP_SECURITY_ERROR(n)) &&                      \
        !(LDAP_SERVICE_ERROR(n))  &&                      \
        !(LDAP_UPDATE_ERROR(n))                           \
        )
/*
 * 1. if dwError is NOT yet in LDAP error code space,
 *    return ldapErroCode for this operation
 * 2. if ldapErrMsg is NOT NULL, create one with supplied contents
 */
#define BAIL_ON_LDAP_ERROR(dwError, ldapErrCode, ldapErrMsg, Format, ... ) \
    if (dwError)                                                    \
    {                                                               \
        do                                                          \
        {                                                           \
            if (ldapErrMsg == NULL)                                 \
            {                                                       \
                VmDirAllocateStringPrintf(                          \
                                &(ldapErrMsg),                      \
                                Format,                             \
                                ##__VA_ARGS__);                     \
            }                                                       \
        } while (0);                                                \
        if (ldapErrCode && NOT_LDAP_ERROR_SPACE(dwError))           \
        {                                                           \
            dwError = ldapErrCode;                                  \
        }                                                           \
        goto ldaperror;                                             \
    }

#define BAIL_ON_SIMPLE_LDAP_ERROR(dwError)                          \
    if (dwError)                                                    \
    {                                                               \
        goto ldaperror;                                             \
    }

#define BAIL_ON_STATIC_LDAP_ERROR( errCode, ldapErrCode, ldapErrStr, errMsg) \
    do                                              \
    {                                               \
        errCode = ldapErrCode;                      \
        BAIL_ON_LDAP_ERROR( errCode,                \
                            ldapErrCode,            \
                            (ldapErrStr),           \
                            "%s",                   \
                            errMsg);                \
    } while (0)

// LBER call return -1 if error
#define BAIL_ON_LBER_ERROR(dwError) \
    do                                                                          \
    {                                                                           \
        if (dwError == -1)                                                      \
        {                                                                       \
            VMDIR_LOG_DEBUG( VMDIR_LOG_MASK_ALL, "[%s,%d]",__FILE__, __LINE__); \
            goto error;                                                         \
        }                                                                       \
        dwError = 0;                                                            \
    } while (0)

// Use this MACRO in "error:" label to add more context to return error message string.
// We ignore error in this macro and should NOT do another bail within.
#define VMDIR_APPEND_ERROR_MSG(pszOrgErrMsg, pszNewErrMsg)      \
    do                                                          \
    {                                                           \
        if (pszNewErrMsg != NULL)                               \
        {                                                       \
            if (pszOrgErrMsg == NULL)                           \
            {                                                   \
                VmDirAllocateStringA(   pszNewErrMsg,           \
                                        &(pszOrgErrMsg));       \
            }                                                   \
            else                                                \
            {                                                   \
                PSTR    pszTmp = pszOrgErrMsg;                  \
                VmDirAllocateStringPrintf(                      \
                                &(pszOrgErrMsg),                \
                                "%s %s",                        \
                                pszTmp, pszNewErrMsg);          \
                VMDIR_SAFE_FREE_MEMORY(pszTmp);                 \
            }                                                   \
        }                                                       \
    } while (0)

// set ldap result error code and message
// NOTE ---- ldapResult->pszErrMsg MAY take over pszMsg
#define VMDIR_SET_LDAP_RESULT_ERROR(pResult, dwCode, pszMsg)    \
    do                                                          \
    {                                                           \
        if ( pResult && dwCode != 0 )                           \
        {                                                       \
            if ( (pResult)->vmdirErrCode == 0 )                 \
            {                                                   \
                (pResult)->vmdirErrCode = dwCode;               \
            }                                                   \
            if ( (pResult)->errCode == 0 )                      \
            {                                                   \
                (pResult)->errCode = VmDirToLDAPError(dwCode);  \
                if ( !(pResult)->pszErrMsg && pszMsg)           \
                {                                               \
                    (pResult)->pszErrMsg = pszMsg;              \
                    pszMsg = NULL;                              \
                }                                               \
            }                                                   \
        }                                                       \
    } while (0)

// for public API, map error code.
#define VMDIR_API_ERROR_MAP( dwError, dwAPIError, MAP ) \
    do {                                        \
        int iSize = sizeof(MAP)/sizeof(MAP[0]); \
        int iCnt = 0;                           \
        for (iCnt = 0; iCnt < iSize; iCnt++)    \
        {                                       \
            if (dwError == MAP[iCnt])           \
            {                                   \
                dwAPIError = dwError;           \
                break;                          \
            }                                   \
        }                                       \
        if (iCnt == iSize)                      \
        {                                       \
            dwAPIError = VMDIR_ERROR_GENERIC;   \
        }                                       \
    } while (0)

#define VMDIR_LOG( Level, Mask, Format, ... ) \
    do                                             \
    {                                              \
        VmDirLog1( Level,                          \
                   Mask,                           \
                   Format,                         \
                   ##__VA_ARGS__);                 \
    } while (0)

#define VMDIR_LOG_GENERAL( Level, Mask, Format, ... ) \
                VMDIR_LOG( Level, Mask, Format, ##__VA_ARGS__ )

#define VMDIR_LOG_ERROR( Mask, Format, ... )   \
                VMDIR_LOG_GENERAL( VMDIR_LOG_ERROR, Mask, Format, ##__VA_ARGS__ )

#define VMDIR_LOG_WARNING( Mask, Format, ... ) \
                VMDIR_LOG_GENERAL( VMDIR_LOG_WARNING, Mask, Format, ##__VA_ARGS__ )

#define VMDIR_LOG_INFO( Mask, Format, ... )    \
                VMDIR_LOG_GENERAL( VMDIR_LOG_INFO, Mask, Format, ##__VA_ARGS__ )

#define VMDIR_LOG_VERBOSE( Mask, Format, ... ) \
                VMDIR_LOG_GENERAL( VMDIR_LOG_VERBOSE, Mask, Format, ##__VA_ARGS__ )

#define VMDIR_LOG_DEBUG( Mask, Format, ... )   \
                VMDIR_LOG_GENERAL( VMDIR_LOG_DEBUG,                     \
                                   Mask, "[file: %s][line: %d] " Format,\
                                   __FILE__, __LINE__, ##__VA_ARGS__ )

// if VDIR_CONNECTION has bind info in VDIR_ACCESS_INFO, use it; otherwise,
// this is an internal operation and hence uses default administrator DN
#define VMDIR_CURRENT_AUTHENTICATED_DN( pAccessInfo )       \
    ( ( (pAccessInfo) && (pAccessInfo)->pszBindedDn ) ?     \
       (pAccessInfo)->pszBindedDn :                         \
       gVmdirServerGlobals.bvDefaultAdminDN.lberbv_val )

#define IS_BERVALUE_EQUAL(bv1, bv2)                                 \
        ((bv1).bv_len == (bv2).bv_len                               \
         &&                                                         \
         (memcmp((bv1).bv_val,(bv2).bv_val,(bv1).bv_len) == 0))

#define IS_BERVALUE_EMPTY_OR_NULL(bv)                               \
        (((bv).bv_val == NULL) || ((bv).bv_len == 0))

#ifndef IsNullOrEmptyString
#define IsNullOrEmptyString(str) (!(str) || !*(str))
#endif

#ifndef VDIR_SAFE_STRING
#define VDIR_SAFE_STRING(str) ((str) ? (str) : "")
#endif

#ifndef VDIR_SAFE_SPACE_STRING
#define VDIR_SAFE_SPACE_STRING(str) ((str) ? (str) : " ")
#endif

#ifndef VMDIR_ARRAY_SIZE
#define VMDIR_ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#endif

//////////////////////////////////////////////////////////////////////////
// Used in attribute index cache and bdb-store
#define INDEX_TYPE_EQUALITY              0x01
#define INDEX_TYPE_SUBSTR                0x02
#define INDEX_TYPE_PRESENCE              0x04
#define INDEX_TYPE_APPROX                0x08


#define VMDIR_DB_FILE_NAME     "vmdir.db"
#define SEQ_DB_NAME            "sequence"
#define VMDIR_ENTRY_DB         "entry"
//////////////////////////////////////////////////////////////////////////

#define MAX_DEADLOCK_RETRIES      5

#ifndef LDAP_DEBUG_ANY
#define LDAP_DEBUG_ANY (-1)
#endif

#ifndef LDAP_DEBUG_DEBUG
#define LDAP_DEBUG_DEBUG (-2)
#endif

#ifndef LDAP_DEBUG_ERROR
#define LDAP_DEBUG_ERROR (-3)
#endif

#ifndef LDAP_DEBUG_TRACE
#define LDAP_DEBUG_TRACE (1)
#endif

#define METADATA_TOKEN_COUNT 6
#define HIGHWATER_USN_STEP   100
#define HIGHWATER_USN_REPL_BUFFER 10000

#define VMDIR_ASCII_aTof(c)     ( (c) >= 'a' && (c) <= 'f' )
#define VMDIR_ASCII_AToF(c)     ( (c) >= 'A' && (c) <= 'F' )
#define VMDIR_ASCII_LOWER(c)    ( (c) >= 'a' && (c) <= 'z' )
#define VMDIR_ASCII_UPPER(c)    ( (c) >= 'A' && (c) <= 'Z' )
#define VMDIR_ASCII_DIGIT(c)    ( (c) >= '0' && (c) <= '9' )
// AD compatible special chars
#define VMDIR_PASSWD_SP_CHAR(c) ( (c) == '~' ||   \
                                  (c) == '!' ||   \
                                  (c) == '@' ||   \
                                  (c) == '#' ||   \
                                  (c) == '$' ||   \
                                  (c) == '%' ||   \
                                  (c) == '^' ||   \
                                  (c) == '&' ||   \
                                  (c) == '*' ||   \
                                  (c) == '_' ||   \
                                  (c) == '-' ||   \
                                  (c) == '+' ||   \
                                  (c) == '=' ||   \
                                  (c) == '`' ||   \
                                  (c) == '|' ||   \
                                  (c) == '\\' ||  \
                                  (c) == '(' ||   \
                                  (c) == ')' ||   \
                                  (c) == '{' ||   \
                                  (c) == '}' ||   \
                                  (c) == '[' ||   \
                                  (c) == ']' ||   \
                                  (c) == ':' ||   \
                                  (c) == ';' ||   \
                                  (c) == '"' ||   \
                                  (c) == '\'' ||  \
                                  (c) == '<' ||   \
                                  (c) == '>' ||   \
                                  (c) == ',' ||   \
                                  (c) == '.' ||   \
                                  (c) == '?' ||   \
                                  (c) == '/' )

#define VMDIR_ASCII_LOWER_TO_UPPER(c)   \
if ( VMDIR_ASCII_LOWER(c) )             \
{                                       \
    (c) = ((c) - 32);                   \
}

#define VMDIR_ASCII_UPPER_TO_LOWER(c)   \
if ( VMDIR_ASCII_UPPER(c) )             \
{                                       \
    (c) = ((c) + 32);                   \
}

#define VMDIR_MAX_PATH_LEN              512
#define VMDIR_MAX_PWD_LEN               128

// Conforms to MSDN http://msdn.microsoft.com/en-us/library/windows/desktop/ms738527(v=vs.85).aspx
#define VMDIR_MAX_HOSTNAME_LEN         256

#define VMDIR_MAX_LDAP_URI_LEN         256 /* e.g. ldap://192.168.122.65 */
#define VMDIR_DEFAULT_REPL_LAST_USN_PROCESSED       "0"

#define VMDIR_UPN_REALM_SEPARATOR       '@'

#define VMDIR_KDC_RANDOM_PWD_LEN        20
#define VMDIR_MAX_REALM_LEN             256

#define VMDIR_MAX_LOG_OUTPUT_LEN        256

#define VMDIR_SASL_MIN_SSF              1
#define VMDIR_SASL_MAX_SSF              1024
#define VMDIR_SASL_MAX_BUFFER_LEN       0xfffff

#define VMDIR_LDAPS_PROTOCOL            "ldaps"
#define VMDIR_LDAP_PROTOCOL             "ldap"

#define VMDIR_MAX_EAGAIN_RETRY		5

#define MSECS_IN_SECOND   (1000)
#define SECONDS_IN_MINUTE (60)
#define MINUTES_IN_HOUR   (60)
#define HOURS_IN_DAY      (24)
#define SECONDS_IN_HOUR   (SECONDS_IN_MINUTE * MINUTES_IN_HOUR)
#define SECONDS_IN_DAY    (SECONDS_IN_HOUR * HOURS_IN_DAY)

#define WIN_EPOCH 116444736000000000LL

#ifdef _WIN32

#define VMDIR_PATH_SEPARATOR_STR "\\"
#define VMDIR_ADDR_INFO_NEXT( ai ) ai->ai_next
#define VMDIR_ADDR_INFO_FLAGS( ai ) ai->ai_flags
#define VMDIR_ADDR_INFO_ADDR( ai ) ai->ai_addr

#else

#define VMDIR_PATH_SEPARATOR_STR "/"
#define VMDIR_ADDR_INFO_NEXT( ifa ) ifa->ifa_next
#define VMDIR_ADDR_INFO_FLAGS( ifa ) ifa->ifa_flags
#define VMDIR_ADDR_INFO_ADDR( ifa ) ifa->ifa_addr

#endif

#define VMDIR_ATTR_WITH_SINGLE_VALUE(pAttr)         \
    ((pAttr != NULL)                    &&          \
     (pAttr->numVals == 1)              &&          \
     (pAttr->vals[0].lberbv.bv_len > 0) &&          \
     (pAttr->vals[0].lberbv.bv_val != NULL))

/*
 * Set pointer P at the next Nth field from the current position in attr-value-meta-data
 * attr-value-meta-data is delimited by ':'
 */
#define VALUE_META_TO_NEXT_FIELD(P, N)                                              \
    {                                                                               \
        int iii;                                                                    \
        char *ppp;                                                                  \
        for(iii=0,ppp=P;iii<N && ppp;P=VmDirStringChrA(ppp, ':')+1,ppp=P,iii++);    \
    }

//IPC API

#define VMDIR_IPC_INITIALIZE_HOST      0
#define VMDIR_IPC_INITIALIZE_TENANT    1
#define VMDIR_IPC_FORCE_RESET_PASSWORD 2
//#define VMDIR_IPC_GET_SRP_SECRET       3
#define VMDIR_IPC_SET_SRP_SECRET       4
#define VMDIR_IPC_GENERATE_PASSWORD    5
#define VMDIR_IPC_GET_SERVER_STATE     6
#define VMDIR_IPC_CREATE_TENANT        7
#define VMDIR_IPC_DELETE_TENANT        8
#define VMDIR_IPC_ENUMERATE_TENANTS    9

//VERSIONS
#define VER1_INPUT 0
#define VER1_OUTPUT 1

typedef enum
{
  VM_DIR_CONTEXT_TYPE_UNKNOWN = 0,
  VM_DIR_CONTEXT_TYPE_ROOT,
  VM_DIR_CONTEXT_TYPE_EVERYONE
} VM_DIR_CONTEXT_TYPE, *PVM_DIR_CONTEXT_TYPE;

#define GROUP_EVERYONE_W {'E','V','E','R','Y','O','N','E',0}
#define GROUP_EVERYONE "EVERYONE";

/*
 * 1.2.840.113554.1.2.10
 *
 * {iso(1) member-body(2) US(840) mit(113554) infosys(1) gssapi(2) srp(10)}
 * "Made up" SRP OID,
 * "Made up" SRP OID, which is actually in MIT GSSAPI OID namespace,
 *  based on existing GSSAPI mech OIDs.
 * This is being depricated in future releases.
 */
#ifndef GSS_SRP_MECH_OID
#define GSS_SRP_MECH_OID_LENGTH 9
#define GSS_SRP_MECH_OID "\x2a\x86\x48\x86\xf7\x12\x01\x02\x0a"
#endif

/*
 * 1.3.6.1.4.1.27433.3.1
 *
 * {iso(1) identified-organization(3) dod(6) internet(1) private(4)
 *   enterprise(1) 27433}
 * Centeris Corporation
 * Note: No OIDs are officially registered under the father Centeris Corp OID.
 * "\x06\x08\x2b\x06\x01\x04\x01\x81\xd6\x29"
 *
 * Note: This is the exact OID as found in lsass/include/ntlm/gssntlm.h;
 */
#ifndef GSS_CRED_OPT_PW
#define GSS_CRED_OPT_PW     "\x2b\x06\x01\x04\x01\x81\xd6\x29\x03\x01"
#define GSS_CRED_OPT_PW_LEN 10
#endif

/* Defines related to GSS_NTLM authentication */
#ifndef GSS_NTLM_MECH_OID
#define GSS_NTLM_MECH_OID_LENGTH 10
#define GSS_NTLM_MECH_OID "\x2b\x06\x01\x04\x01\x82\x37\x02\x02\x0a"
#endif

#ifndef GSS_NTLM_PASSWORD_OID
#define GSS_NTLM_PASSWORD_OID "\x2b\x06\x01\x04\x01\x81\xd6\x29\x03\x01"
#define GSS_NTLM_PASSWORD_LEN 10
#endif

#ifndef SPNEGO_OID
#define SPNEGO_OID_LENGTH 6
#define SPNEGO_OID "\x2b\x06\x01\x05\x05\x02"
#endif

/*
 * 1.3.6.1.4.1.6876.11711.2.1.1
 *
 * {iso(1) identified-organization(3) dod(6) internet(1) private(4)
 *   enterprise(1) 6876 vmwSecurity(11711) vmwAuthentication(2) vmwGSSAPI(1)
 *   vmwSRP(1)}
 * Official registered GSSAPI_SRP Mech OID
 */
#ifndef GSSAPI_SRP_MECH_OID_LENGTH
#define GSSAPI_SRP_MECH_OID_LENGTH 12
#endif

#ifndef GSSAPI_SRP_MECH_OID
#define GSSAPI_SRP_MECH_OID "\x2b\x06\x01\x04\x01\xb5\x5c\xdb\x3f\x02\x01\x01"
#endif

/*
 * 1.3.6.1.4.1.6876.11711.2.1.1.1
 *
 * {iso(1) identified-organization(3) dod(6) internet(1) private(4)
 *   enterprise(1) 6876 vmwSecurity(11711) vmwAuthentication(2) vmwGSSAPI(1)
 *   vmwSRP(1) vmwSrpCredOptPwd(1)}
 * Official registered GSSAPI_SRP password cred option OID
 */
#ifndef GSSAPI_SRP_CRED_OPT_PW
#define GSSAPI_SRP_CRED_OPT_PW  \
    "\x2b\x06\x01\x04\x01\xb5\x5c\xdb\x3f\x02\x01\x01\x01"
#define GSSAPI_SRP_USERNAME  \
    "\x2b\x06\x01\x04\x01\xb5\x5c\xdb\x3f\x02\x01\x01\x02"
#endif

#ifndef GSSAPI_SRP_CRED_OPT_PW_LEN
#define GSSAPI_SRP_CRED_OPT_PW_LEN  13
#endif

/*
 * vmwUnix 1.3.6.1.4.1.6876.11711.2.1.2
 *   vmwUnixCredOptPwd 1.3.6.1.4.1.6876.11711.2.1.2.1
 */

/*
 * 1.3.6.1.4.1.6876.11711.2.1.2
 *
 * {iso(1) identified-organization(3) dod(6) internet(1) private(4)
 *   enterprise(1) 6876 vmwSecurity(11711) vmwAuthentication(2) vmwGSSAPI(1)
 *   vmwUNIX(2)}
 * Official registered GSSAPI_UNIX Mech OID
 */
#ifndef GSSAPI_UNIX_MECH_OID_LENGTH
#define GSSAPI_UNIX_MECH_OID_LENGTH 12
#endif

#ifndef GSSAPI_UNIX_MECH_OID
#define GSSAPI_UNIX_MECH_OID "\x2b\x06\x01\x04\x01\xb5\x5c\xdb\x3f\x02\x01\x02"
#endif

/*
 * 1.3.6.1.4.1.6876.11711.2.1.2.1
 *
 * {iso(1) identified-organization(3) dod(6) internet(1) private(4)
 *   enterprise(1) 6876 vmwSecurity(11711) vmwAuthentication(2) vmwGSSAPI(1)
 *   vmwUNIX(2) vmwSrpCredOptPwd(1)}
 * Official registered GSSAPI_UNIX password cred option OID
 */
#ifndef GSSAPI_UNIX_CRED_OPT_PW
#define GSSAPI_UNIX_CRED_OPT_PW  \
    "\x2b\x06\x01\x04\x01\xb5\x5c\xdb\x3f\x02\x01\x02\x01"
#define GSSAPI_UNIX_USERNAME  \
    "\x2b\x06\x01\x04\x01\xb5\x5c\xdb\x3f\x02\x01\x02\x02"
#endif

#ifndef GSSAPI_UNIX_CRED_OPT_PW_LEN
#define GSSAPI_UNIX_CRED_OPT_PW_LEN  13
#endif

#ifdef __cplusplus
}
#endif

#endif /* __VDIR_DEFINE_H__ */
