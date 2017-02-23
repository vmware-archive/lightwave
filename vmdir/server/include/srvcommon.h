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



#ifndef COMMON_INTERFACE_H_
#define COMMON_INTERFACE_H_

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_WIN32)
#define __func__ __FUNCTION__
#endif

#define VMDIR_ORIG_TIME_STR_LEN         ( 4 /* year */ + 2 /* month */ + 2 /* day */ + 2 /* hour */ + 2 /* minute */ + \
                                          2 /* sec */ + 1 /* . */ + 3 /* milli sec */ + 1 /* null byte terminator */ )

#define VMDIR_MAX_USN_STR_LEN           VMDIR_MAX_I64_ASCII_STR_LEN
#define VMDIR_MAX_VERSION_NO_STR_LEN    VMDIR_MAX_I64_ASCII_STR_LEN /* Version number used in attribute meta-data */

//
// If the new paged search is turned on then the cookie is a guid; otherwise,
// it's an ENTRYID.
//
#define VMDIR_PS_COOKIE_LEN             (VMDIR_MAX(VMDIR_MAX_I64_ASCII_STR_LEN, VMDIR_GUID_STR_LEN))

// Format is: <local USN>:<version no>:<originating server ID>:<originating time>:<originating USN>
#define VMDIR_MAX_ATTR_META_DATA_LEN    (VMDIR_MAX_USN_STR_LEN + 1 + VMDIR_MAX_VERSION_NO_STR_LEN + 1 + \
                                         VMDIR_GUID_STR_LEN + 1 + VMDIR_ORIG_TIME_STR_LEN + 1 +    \
                                         VMDIR_MAX_USN_STR_LEN + 1)

// Format is: <attr-name>:<local-usn>:<version-no>:<originating-server-id>:<value-change-originating-server-id>
//       :<value-change-originating time>:<value-change-originating-usn>:
//       Remaining portion of attr-value-meta-data to be stored in backend: <opcode>:<value-size>:<value>
#define VMDIR_MAX_ATTR_VALUE_META_DATA_LEN    (VMDIR_MAX_USN_STR_LEN + 1 + VMDIR_MAX_VERSION_NO_STR_LEN + 1 + \
                                               VMDIR_GUID_STR_LEN + 1 + VMDIR_GUID_STR_LEN + 1 + \
                                               VMDIR_ORIG_TIME_STR_LEN + 1 + VMDIR_MAX_USN_STR_LEN + 1 + 1)

#define VMDIR_IS_DELETED_TRUE_STR      "TRUE"
#define VMDIR_IS_DELETED_TRUE_STR_LEN  4

#define VMDIR_UTD_VECTOR_HASH_TABLE_SIZE  100
#define VMDIR_PAGED_SEARCH_CACHE_HASH_TABLE_SIZE 32
#define VMDIR_LOCKOUT_VECTOR_HASH_TABLE_SIZE  1000

#define VMDIR_DEFAULT_REPL_INTERVAL     "30"
#define VMDIR_DEFAULT_REPL_PAGE_SIZE    "1000"
#define VMDIR_REPL_CONT_INDICATOR       "continue:1,"
#define VMDIR_REPL_CONT_INDICATOR_LEN   sizeof(VMDIR_REPL_CONT_INDICATOR)-1

#define VMDIR_RUN_MODE_RESTORE          "restore"
#define VMDIR_RUN_MODE_STANDALONE       "standalone"

// backend generic table keys
#define VMDIR_KEY_BE_GENERIC_ACL_MODE   "acl-mode"
#define VMDIR_ACL_MODE_ENABLED          "enabled"

#define VMDIR_DEFAULT_REPL_LAST_USN_PROCESSED_LEN   sizeof(VMDIR_DEFAULT_REPL_LAST_USN_PROCESSED)

#define GENERALIZED_TIME_STR_LEN       17

#ifndef INET6_ADDRSTRLEN
#define INET6_ADDRSTRLEN 46
#endif

#define SOCK_BUF_MAX_INCOMING ((1<<24) - 1) // 16M - 1, e.g. to handle large Add object requests.

// Fix bootstrap attribute id used in schema/defines.h VDIR_SCHEMA_BOOTSTRP_ATTR_INITIALIZER definition
#define SCHEMA_BOOTSTRAP_EID_SEQ_ATTRID_22     22
#define SCHEMA_BOOTSTRAP_USN_SEQ_ATTRID_23     23

#define VDIR_FOREST_FUNCTIONAL_LEVEL    "1"
// This value is the DFL for the current version
#define VDIR_DOMAIN_FUNCTIONAL_LEVEL	"4"

// Mapping of functionality to levels
// Base DFL, support for all 6.0 and earlier functionality
#define VDIR_DFL_DEFAULT 1

// Support for 6.5 functionality, PSCHA
#define VDIR_DFL_PSCHA   2

// Support for 6.6 functionality, ModDn
// Support for LW 1.0 functionality, ModDn
#define VDIR_DFL_MODDN   3

// Support for 6.6 functionality, Custom Schema Modification
// Support for LW 1.0 functionality, Custom Schema Modification
#define VDIR_DFL_CUSTOM_SCHEMA_MODIFICATION     3

// Support for 6.6 functionality, Concurrent Attribute Value Update
// Support for LW 1.1 functionality, Concurrent Attribute Value Update
#define VDIR_DFL_CONCURRENT_ATTR_VALUE_UPDATE   4

// Support for 6.6 functionality, better write operation audit
// Support for LW 1.1 functionality, better write operation audit
#define VDIR_DFL_WRITE_OP_AUDIT                 4

#define VDIR_CUSTOM_SCHEMA_MODIFICATION_ENABLED     \
    (gVmdirServerGlobals.dwDomainFunctionalLevel >= \
            VDIR_DFL_CUSTOM_SCHEMA_MODIFICATION)

#define VDIR_WRITE_OP_AUDIT_ENABLED   \
    (gVmdirServerGlobals.dwDomainFunctionalLevel >= \
            VDIR_DFL_WRITE_OP_AUDIT)

#define VDIR_CONCURRENT_ATTR_VALUE_UPDATE_ENABLED   \
    (gVmdirServerGlobals.dwDomainFunctionalLevel >= \
            VDIR_DFL_CONCURRENT_ATTR_VALUE_UPDATE)

// Keys for backend funtion pfnBEStrkeyGet/SetValues to access attribute IDs
#define ATTR_ID_MAP_KEY   "1VmdirAttrIDToNameTb"

typedef struct _VDIR_INDEX_CFG*             PVDIR_INDEX_CFG;
typedef struct _VDIR_INDEX_UPD*             PVDIR_INDEX_UPD;
typedef struct _VDIR_BACKEND_INTERFACE*     PVDIR_BACKEND_INTERFACE;
typedef struct _VDIR_SCHEMA_CTX*            PVDIR_SCHEMA_CTX;
typedef struct _VDIR_SCHEMA_DIFF*           PVDIR_SCHEMA_DIFF;
typedef struct _VDIR_ACL_CTX*               PVDIR_ACL_CTX;

typedef struct _VDIR_BERVALUE
{
#define lberbv_val   lberbv.bv_val
#define lberbv_len   lberbv.bv_len
    BerValue        lberbv;     // native lber BerValue

    // TRUE if we own bv_val.
    // Generally, when data coming from BER, BER owns bv_val at the connection level.
    // If server creates VDIR_BERVAL internally, it owns bv_val and should set this TRUE.
    // (TODO, should assume server owns bv_val by default and set to FALSE at BER parse area.)
    unsigned short  bOwnBvVal; // true if owns bv_val

    // initially, bvnorm_len=0 and bvnorm_val=NULL
    // after normalize call, it becomes
    // if normalize form != original form
    //    bvnorm_val = heap with normalize string NULL terminated
    //    bvnorm_len = strlen(bvnorm_val);
    //  else
    //    bvnorm_val = bv_val and bvnorm_len = bv_len
    ber_len_t       bvnorm_len;
    char*           bvnorm_val;

} VDIR_BERVALUE, *PVDIR_BERVALUE;

#define VDIR_BERVALUE_INIT  { {0,NULL}, 0, 0, NULL }

typedef PVDIR_BERVALUE VDIR_BERVARRAY;

typedef struct _VDIR_BACKEND_CTX
{
    PVDIR_BACKEND_INTERFACE pBE;
    // per data store specific private structure to support transaction context
    int         iBEPrivateRef;
    PVOID       pBEPrivate;
    DWORD       dwBEErrorCode;
    PSTR        pszBEErrorMsg;
    USN         wTxnUSN;            // lowest USN associates with a write txn (could be nested tnx)
                                    // i.e. should be the first USN number acquired per backend write txn.
    DWORD       iMaxScanForSizeLimit;  // Maximum candiates scaned an index for the best effort candidates
                                       // build when search has a size limit hint. Unlimited if it is 0
    int         iPartialCandidates; // indicate at least one of the filters with candidates contains partial result
                                    //   valid only when iMaxScanForSizeLimt > 0
} VDIR_BACKEND_CTX, *PVDIR_BACKEND_CTX;

// accessRoleBitmap is a bit map on bind dn access role if the info is valid
#define VDIR_ACCESS_ADMIN_MEMBER_VALID_INFO 0x0001            // valid info in accessRoleBitmap on system domain admins
#define VDIR_ACCESS_IS_ADMIN_MEMBER 0x0002                    // bind dn is a member of system domain admins
#define VDIR_ACCESS_ADMIN_MEMBER_INFO VDIR_ACCESS_ADMIN_MEMBER_VALID_INFO
#define VDIR_ACCESS_DCGROUP_MEMBER_VALID_INFO 0x0004          // valid info in accessRoleBitmap member of DC group
#define VDIR_ACCESS_IS_DCGROUP_MEMBER 0x0008                  // bind dn is a member of DC group
#define VDIR_ACCESS_DCGROUP_MEMBER_INFO VDIR_ACCESS_DCGROUP_MEMBER_VALID_INFO
#define VDIR_ACCESS_DCCLIENT_GROUP_MEMBER_VALID_INFO 0x0010   // valid info in accessRoleBitmap on member of DC client group
#define VDIR_ACCESS_IS_DCCLIENT_GROUP_MEMBER 0x0020           // bind dn is a member of DC client group
#define VDIR_ACCESS_DCCLIENT_GROUP_MEMBER_INFO VDIR_ACCESS_DCCLIENT_GROUP_MEMBER_VALID_INFO
typedef struct _VDIR_ACCESS_INFO
{
    ENTRYID       bindEID;     // bind user ENTRYID
    PSTR          pszBindedDn; // original DN passed in on the wire
    PSTR          pszNormBindedDn;
    PSTR          pszBindedObjectSid;
    PACCESS_TOKEN pAccessToken;
    UINT32        accessRoleBitmap; // access role if the info is valid
} VDIR_ACCESS_INFO, *PVDIR_ACCESS_INFO;

typedef struct _VDIR_SASL_BIND_INFO*    PVDIR_SASL_BIND_INFO;

typedef struct _VDIR_SUPERLOG_RECORD_SEARCH_INFO
{
    PSTR    pszAttributes;
    PSTR    pszBaseDN;
    PSTR    pszScope;
    PSTR    pszIndexResults;
    DWORD   dwScanned;
    DWORD   dwReturned;
} VDIR_SUPERLOG_RECORD_SEARCH_INFO;

typedef union _VDIR_SUPERLOG_RECORD_OPERATION_INFO
{
    VDIR_SUPERLOG_RECORD_SEARCH_INFO searchInfo;
} VDIR_SUPERLOG_RECORD_OPERATION_INFO;

typedef struct _VDIR_SUPERLOG_RECORD
{
    uint64_t   iStartTime;
    uint64_t   iEndTime;

    // bind op
    PSTR       pszBindID; // could be either DN or UPN
    PSTR       pszOperationParameters;

    VDIR_SUPERLOG_RECORD_OPERATION_INFO opInfo;
} VDIR_SUPERLOG_RECORD, *PVDIR_SUPERLOG_RECORD;

typedef struct _VDIR_CONNECTION
{
    Sockbuf *               sb;
    ber_socket_t            sd;
    VDIR_ACCESS_INFO        AccessInfo;
    BOOLEAN                 bIsAnonymousBind;
    BOOLEAN                 bIsLdaps;
    BOOLEAN                 bInReplLock;
    PVDIR_SASL_BIND_INFO    pSaslInfo;
    char                    szServerIP[INET6_ADDRSTRLEN];
    DWORD                   dwServerPort;
    char                    szClientIP[INET6_ADDRSTRLEN];
    DWORD                   dwClientPort;
    VDIR_SUPERLOG_RECORD    SuperLogRec;
} VDIR_CONNECTION, *PVDIR_CONNECTION;

typedef struct _VDIR_CONNECTION_CTX
{
  ber_socket_t sockFd;
  Sockbuf_IO   *pSockbuf_IO;
} VDIR_CONNECTION_CTX, *PVDIR_CONNECTION_CTX;

typedef struct _VDIR_SCHEMA_AT_DESC*    PVDIR_SCHEMA_AT_DESC;

typedef struct _VDIR_ATTRIBUTE
{
   PVDIR_SCHEMA_AT_DESC pATDesc;

   // type.bv_val always points to in-place storage and should not have bvnormval.
   VDIR_BERVALUE        type;

   // allocated space for array of bervals
   VDIR_BERVARRAY       vals;
   unsigned             numVals;

   /* Multi-master replication meta data
    * Format is: <local USN>:<version no>:<originating server ID>:<originating time>:<originating USN>
    */
   char                 metaData[VMDIR_MAX_ATTR_META_DATA_LEN];

   /* A queue of attr-value-meta-data elements to add to the backend index database.
    * Each element contains a VDIR_BERVALUE variable, and its bv_val is in format:
    *     <local-usn>:<version-no>:<originating-server-id>:<value-change-originating-server-id>
    *       :<value-change-originating time>:<value-change-originating-usn>:<opcode>:<value-size>:<value>
    *     <value> is an octet string.
    */
   DEQUE  valueMetaDataToAdd;
   /* Hold obsolete attr-value-meta-data elements to be deleted.
    * The elements to be added to this queue when MOD_OP_REPLACE
    * or modify MOD_OP_DELETE with empty value on multi-value attribute.
    */
   DEQUE  valueMetaDataToDelete;

   struct _VDIR_ATTRIBUTE *  next;
} VDIR_ATTRIBUTE, *PVDIR_ATTRIBUTE;

typedef struct _ATTRIBUTE_META_DATA_NODE
{
    USHORT  attrID;
    char    metaData[VMDIR_MAX_ATTR_META_DATA_LEN];
} ATTRIBUTE_META_DATA_NODE, *PATTRIBUTE_META_DATA_NODE;

#define VDIR_DEFAULT_FORCE_VERSION_GAP  512

typedef struct _VDIR_MODIFICATION
{
    VDIR_LDAP_MOD_OP            operation;
    VDIR_ATTRIBUTE              attr;
    BOOLEAN                     ignore; // Used internally, e.g. to skip processing a Delete modification when the attribute
                                  // does not exist in the entry
    unsigned short              usForceVersionGap;  // to intentionally create gap between attribute version
    struct _VDIR_MODIFICATION * next;
} VDIR_MODIFICATION, *PVDIR_MODIFICATION;

typedef enum _VDIR_ENTRY_ALLOCATION_TYPE
{
    ENTRY_STORAGE_FORMAT_PACK,
    ENTRY_STORAGE_FORMAT_NORMAL
} VDIR_ENTRY_ALLOCATION_TYPE;

//SCW - strong consistency write
typedef enum _VDIR_ENTRY_SCW_STATUS
{
    VDIR_SCW_SUCCEEDED,
    VDIR_SCW_FAILED
} VDIR_ENTRY_SCW_STATUS;

typedef struct _VDIR_ENTRY
{

   VDIR_ENTRY_ALLOCATION_TYPE   allocType;

   // Internally constructed Entry (non-persist entry) has eId == 0
   ENTRYID                      eId;     // type must match BDB's db_seq_t

   // dn.bv_val is heap allocated; dn.bvnorm_bv follows BerValue rule
   VDIR_BERVALUE                dn;
   // pdn.bv_val is in-place into dn.bv_val.  pdn.bvnrom_val follows BerValue rule
   VDIR_BERVALUE                pdn;
   // pdnnew is set during rename operations if the entry is being re-parented
   VDIR_BERVALUE                newpdn;

   // FORMAT_PACK, savedAttrsPtr is array of Attribute from one heap allocate
   // FORMAT_NORMAL, attrs (and next...) are individually heap allocated
   PVDIR_ATTRIBUTE              attrs;
   PVDIR_ATTRIBUTE              savedAttrsPtr; // Used to free the allocated Attribute array. "attrs" could change from its
                                               // original value e.g. after a "delete attribute" modification.

   // computed (non-persist) attributes (such as memberof...)
   PVDIR_ATTRIBUTE              pComputedAttrs;

   // bvs only apply to FORMAT_PACK allocType
   VDIR_BERVALUE *              bvs; // Pointer to allocated BerValue structures.
   unsigned short               usNumBVs;

   // encodedEntry is used in PACK format (in-place for dn and bvs)
   // (though NORMAL entry could have this value as well e.g. modify op could reencode)
   unsigned char *              encodedEntry;

   // make sure entry does not out live its schema context
   // if we do not allow live schema update, then we can eliminate this and
   // schema instance lock and reference count overhead.
   PVDIR_SCHEMA_CTX             pSchemaCtx;

   // name of the structure object class (points into Entry.attrs.vals[x].bv_val)
   // its value is set after VmDirSchemaCheck call.
   PSTR                         pszStructureOC;

   // carries the SD information to support access check
   PVDIR_ACL_CTX                pAclCtx;

   // carries a GUID string used to construct entry's objectSid
   // The guid string is in the format of "00000000-0000-0001-8888-000000000000"
   // This is only given when creating root domain nodes (host instance
   // due to replication needs, metadata domain entries need fixed domainSid
   PSTR pszGuid;

   // we own parent entry if exists
   // we cache parent entry for various logic such as ACL, schema structure rule and  parentid index handling...etc.
   struct _VDIR_ENTRY*          pParentEntry;

   // Flag to indicate if the entry was sent back to client in the search result or not.
   BOOLEAN                      bSearchEntrySent;

} VDIR_ENTRY, *PVDIR_ENTRY;

typedef struct _VDIR_ENTRY_ARRAY
{
    PVDIR_ENTRY     pEntry;
    size_t          iSize;
} VDIR_ENTRY_ARRAY, *PVDIR_ENTRY_ARRAY;

typedef struct AttrValAssertion
{
    PVDIR_SCHEMA_AT_DESC pATDesc;
    VDIR_BERVALUE        type;
    VDIR_BERVALUE        value;
} AttrValAssertion;

typedef struct _VDIR_CANDIDATES
{
    ENTRYID *  eIds;
    int        size;        // Current count of eIds in the list.
    int        max;         // Size of the allocated list.
    BOOLEAN    positive;    // +ve candidates list (TRUE) or -ve candidates list (FALSE).
    BOOLEAN    eIdsSorted;  // entryIds in eIds array are sorted or not
} VDIR_CANDIDATES, *PVDIR_CANDIDATES;

typedef enum _VDIR_FILTER_COMPUTE_RESULT
{
    FILTER_RES_NORMAL = 0,
    FILTER_RES_TRUE,
    FILTER_RES_FALSE,
    FILTER_RES_UNDEFINED
} VDIR_FILTER_COMPUTE_RESULT;

typedef struct SubStringFilter
{
   PVDIR_SCHEMA_AT_DESC  pATDesc;
   VDIR_BERVALUE         type;
   VDIR_BERVALUE         initial;
   VDIR_BERVALUE *       any;
   int                   anySize;
   int                   anyMax;
   VDIR_BERVALUE         final;
} SubStringFilter;

/*
 * represents a search filter
 */

// Standard LDAP values for "choice" are present in ldap.h, and additional internal values are:
#define FILTER_ONE_LEVEL_SEARCH              0x00

typedef struct _VDIR_FILTER VDIR_FILTER, *PVDIR_FILTER;

typedef union FilterComponent
{
    VDIR_BERVALUE       present;    // Present filter just containing the attribute type
    AttrValAssertion    ava;        // simple value assertion
    SubStringFilter     subStrings; // Sub-string filter
    PVDIR_FILTER        complex;    // and, or, not
    VDIR_BERVALUE       parentDn;   // Parent DN, relevant when f_choice = LDAP_FILTER_ONE_LEVEL_SRCH
} FilterComponent;

struct _VDIR_FILTER
{
    ber_tag_t                   choice;
    FilterComponent             filtComp;
    struct _VDIR_FILTER *       next;
    VDIR_FILTER_COMPUTE_RESULT  computeResult;
    int                         iMaxIndexScan;  // limit scan to qualify for good filter. 0 means unlimited.
    BOOLEAN                     bAncestorGotPositiveCandidateSet;  // any of ancestor filters got a positive candidate set
    VDIR_CANDIDATES *           candidates;    // Entry IDs candidate list that matches this filter, maintained for internal Ber operation.
    BerElement *                pBer; // If this filter was built by the server, then 'ber' must be deallocated when the filter is deallocated and this will not be NULL. Otherwise the filter components are 'owned' by the operation / client connection.
};

typedef struct AddReq
{
    PVDIR_ENTRY     pEntry;
} AddReq;

typedef struct _BindReq
{
    ber_tag_t       method;
    VDIR_BERVALUE   bvMechanism;        // SASL bind mechanism
    VDIR_BERVALUE   cred;
} BindReq, VDIR_BIND_REQ, *PVDIR_BIND_REQ;

typedef struct DeleteReq
{
    VDIR_BERVALUE           dn;
    PVDIR_MODIFICATION      mods;
    unsigned                numMods;
} DeleteReq;

typedef struct ModifyReq
{
    VDIR_BERVALUE           dn;
    PVDIR_MODIFICATION      mods;
    unsigned                numMods;
    BOOLEAN                 bPasswordModify;
    VDIR_BERVALUE           newrdn;
    BOOLEAN                 bDeleteOldRdn;
    VDIR_BERVALUE           newSuperior;
    VDIR_BERVALUE           newdn;
} ModifyReq;

typedef struct SearchReq
{
    int             scope;
    int             derefAlias;
    int             sizeLimit;
    int             timeLimit;
    int             attrsOnly;
    VDIR_BERVALUE * attrs;
    VDIR_FILTER *   filter;
    VDIR_BERVALUE   filterStr;
    size_t          iNumEntrySent;      // total number entries sent for this request
    BOOLEAN         bStoreRsltInMem;    // store results in mem vs. writing to ber
} SearchReq;

typedef union _VDIR_LDAP_REQUEST
{
    AddReq     addReq;
    BindReq    bindReq;
    DeleteReq  deleteReq;
    ModifyReq  modifyReq;
    SearchReq  searchReq;
} VDIR_LDAP_REQUEST;

typedef enum _VDIR_REPLY_TYPE
{
    REP_SASL = 1
} VDIR_REPLY_TYPE;

typedef struct _VDIR_LDAP_REPLY {
    VDIR_REPLY_TYPE     type;

    union VDIR_LDAP_REPLY_UNION {
        VDIR_BERVALUE   bvSaslReply;
    } replyData;

} VDIR_LDAP_REPLY, *PVDIR_LDAP_REPLY;

typedef struct VDIR_LDAP_RESULT
{
    DWORD           vmdirErrCode;   // internal VMDIR_ERROR_XXX
    ber_int_t       errCode;        // ldap error code
    VDIR_BERVALUE   matchedDn;
    PSTR            pszErrMsg;      // owns this heap allocated error message
    VDIR_BERVALUE * referral;
    VDIR_LDAP_REPLY replyInfo;
} VDIR_LDAP_RESULT, *PVDIR_LDAP_RESULT;

typedef enum SyncRequestMode
{
    UNUSED              = LDAP_SYNC_NONE,
    REFRESH_ONLY        = LDAP_SYNC_REFRESH_ONLY,
    RESERVED            = LDAP_SYNC_RESERVED,
    REFRESH_AND_PERSIST = LDAP_SYNC_REFRESH_AND_PERSIST
} SyncRequestMode;

typedef struct UptoDateVectorEntry
{
    VDIR_BERVALUE           invocationId;
    USN                     reqLastOrigUsnProcessed;
    USN                     currMaxOrigUsnProcessed;
    LW_HASHTABLE_NODE       Node;
} UptoDateVectorEntry;

typedef struct SyncRequestControlValue
{
    SyncRequestMode         mode;
    VDIR_BERVALUE           reqInvocationId;
    VDIR_BERVALUE           bvLastLocalUsnProcessed;
    USN                     intLastLocalUsnProcessed;
    VDIR_BERVALUE           bvUtdVector;
    BOOLEAN                 reloadHint;
} SyncRequestControlValue;

typedef struct SyncDoneControlValue
{
    USN                     intLastLocalUsnProcessed;
    PLW_HASHTABLE           htUtdVector;
    BOOLEAN                 bContinue;
} SyncDoneControlValue;

typedef struct _VDIR_PAGED_RESULT_CONTROL_VALUE
{
    DWORD                   pageSize;
    CHAR                    cookie[VMDIR_PS_COOKIE_LEN];
} VDIR_PAGED_RESULT_CONTROL_VALUE;

//SCW - Strong Consistency Write
typedef struct _VMDIR_SCW_DONE_CONTROL_VALUE
{
    DWORD    status;
} VMDIR_SCW_DONE_CONTROL_VALUE;

typedef union LdapControlValue
{
    SyncRequestControlValue            syncReqCtrlVal;
    SyncDoneControlValue               syncDoneCtrlVal;
    VDIR_PAGED_RESULT_CONTROL_VALUE    pagedResultCtrlVal;
    VMDIR_SCW_DONE_CONTROL_VALUE       scwDoneCtrlVal;
} LdapControlValue;

typedef struct _VDIR_LDAP_CONTROL
{
    char *                type;
    BOOLEAN               criticality;
    LdapControlValue      value;
    struct _VDIR_LDAP_CONTROL *  next;
} VDIR_LDAP_CONTROL, *PVDIR_LDAP_CONTROL;

typedef enum
{
    VDIR_OPERATION_TYPE_EXTERNAL = 1,
    VDIR_OPERATION_TYPE_INTERNAL = 2,
    VDIR_OPERATION_TYPE_REPL = 4,
} VDIR_OPERATION_TYPE;

typedef struct _VDIR_OPERATION
{
    VDIR_OPERATION_TYPE opType;

    ///////////////////////////////////////////////////////////////////////////
    // fields valid only for EXTERNAL operation
    ///////////////////////////////////////////////////////////////////////////
    ber_int_t           protocol;    // version of the LDAP protocol used by client
    BerElement *        ber;         // ber of the request
    ber_int_t           msgId;       // msgid of the request
    PVDIR_CONNECTION    conn;        // Connection
    VDIR_LDAP_CONTROL *       reqControls; // Request Controls, sent by client.
    VDIR_LDAP_CONTROL *       syncReqCtrl; // Sync Request Control, points in reqControls list.
    VDIR_LDAP_CONTROL *       syncDoneCtrl; // Sync Done Control.
    VDIR_LDAP_CONTROL *       showDeletedObjectsCtrl; // points in reqControls list.
    VDIR_LDAP_CONTROL *       showMasterKeyCtrl;
    VDIR_LDAP_CONTROL *       showPagedResultsCtrl;
    VDIR_LDAP_CONTROL *       strongConsistencyWriteCtrl;
                                     // SJ-TBD: If we add quite a few controls, we should consider defining a
                                     // structure to hold all those pointers.
    BOOLEAN             bSchemaWriteOp;  // this operation is schema modification

    ///////////////////////////////////////////////////////////////////////////
    // fields valid for both INTERNAL and EXTERNAL operations
    ///////////////////////////////////////////////////////////////////////////
    VDIR_BERVALUE       reqDn;       // request's target DN (in-place storage)
    ber_tag_t           reqCode;     // LDAP_REQ_ADD/MODIFY/DELETE/SEARCH....
    VDIR_LDAP_REQUEST   request;     // LDAP request (parameters)
    VDIR_LDAP_RESULT    ldapResult;

    PVDIR_SCHEMA_CTX    pSchemaCtx;

#define pBEIF       pBECtx->pBE
#define pBEErrorMsg pBECtx->pszBEErrorMsg
    // backend context
    PVDIR_BACKEND_CTX   pBECtx;

    ///////////////////////////////////////////////////////////////////////////
    // fields valid for INTERNAL operations
    ///////////////////////////////////////////////////////////////////////////

    VDIR_ENTRY_ARRAY    internalSearchEntryArray; // internal search result

    USN                 lowestPendingUncommittedUsn; // recorded at the beginning of replication search operation.

    PSTR                pszFilters; // filter candidates' size recorded in string

    DWORD               dwSentEntries; // number of entries sent back to client

    ///////////////////////////////////////////////////////////////////////////
    // fields valid for REPLICATION operations
    ///////////////////////////////////////////////////////////////////////////
    USN                 ulPartnerUSN; // in replication, the partner USN been processed.

} VDIR_OPERATION, *PVDIR_OPERATION;

typedef struct _VDIR_THREAD_INFO
{
    VMDIR_THREAD                tid;
    BOOLEAN                     bJoinThr;       // join by main thr

    // mutexUsed is real mutex used (i.e. it may not == mutex)
    PVMDIR_MUTEX mutex;
    PVMDIR_MUTEX mutexUsed;

    // conditionUsed is real condition used (i.e. it may not == condition)
    PVMDIR_COND               condition;
    PVMDIR_COND               conditionUsed;

    struct _VDIR_THREAD_INFO*   pNext;

} REPO_THREAD_INFO, *PVDIR_THREAD_INFO;

typedef struct _VMDIR_REPLICATION_AGREEMENT
{
    VDIR_BERVALUE       dn;
    char                ldapURI[VMDIR_MAX_LDAP_URI_LEN];
    VDIR_BERVALUE       lastLocalUsnProcessed;
    BOOLEAN             isDeleted;
    time_t              oldPasswordFailTime;
    time_t              newPasswordFailTime;
    struct _VMDIR_REPLICATION_AGREEMENT *   next;

} VMDIR_REPLICATION_AGREEMENT, *PVMDIR_REPLICATION_AGREEMENT;

typedef struct _VMDIR_OPERATION_STATISTIC
{
    PVMDIR_MUTEX    pmutex;
    PCSTR           pszOPName;
    uint64_t        iTotalCount;
    uint64_t        iCount;
    uint64_t        iTimeInMilliSec;

} VMDIR_OPERATION_STATISTIC, *PVMDIR_OPERATION_STATISTIC;

extern VMDIR_FIRST_REPL_CYCLE_MODE   gFirstReplCycleMode;

typedef struct _VMDIR_URGENT_REPL_SERVER_LIST
{
    PSTR    pInitiatorServerName;
    struct _VMDIR_URGENT_REPL_SERVER_LIST *next;
} VMDIR_URGENT_REPL_SERVER_LIST, *PVMDIR_URGENT_REPL_SERVER_LIST;

typedef struct _VMDIR_STRONG_WRITE_PARTNER_CONTENT
{
    PSTR     pInvocationId;
    PSTR     pServerName;
    BOOLEAN  isDeleted;
    USN      lastConfirmedUSN;
    char     lastNotifiedTimeStamp[VMDIR_ORIG_TIME_STR_LEN];
    char     lastConfirmedTimeStamp[VMDIR_ORIG_TIME_STR_LEN];
    struct _VMDIR_STRONG_WRITE_PARTNER_CONTENT   *next;
} VMDIR_STRONG_WRITE_PARTNER_CONTENT, *PVMDIR_STRONG_WRITE_PARTNER_CONTENT;

//
// Wrapper for a relative security descriptor and some of its related info.
//
typedef struct _VMDIR_SECURITY_DESCRIPTOR
{
    PSECURITY_DESCRIPTOR_RELATIVE pSecDesc;
    ULONG ulSecDesc;
    SECURITY_INFORMATION SecInfo;
} VMDIR_SECURITY_DESCRIPTOR, *PVMDIR_SECURITY_DESCRIPTOR;

DWORD
VmDirInitBackend();

// vmdirentry.c

/*
 * allocate structure resources (but not content) of an entry
 * used in SEARCH_REPLY type entry
 */
DWORD
VmDirInitializeEntry(
   PVDIR_ENTRY pEntry,
   VDIR_ENTRY_ALLOCATION_TYPE   allocType,
   int                          nAttrs,
   int                          nVals);

/*
 * Convert entry allocType from ENTRY_FROM_DB to ENTRY_FROM_WIRE
 */
DWORD
VmDirEntryUnpack(
    PVDIR_ENTRY  pEntry
    );

/*
 * release contents of an entry (but not entry itself, e.g. stack entry)
 */
void
VmDirFreeEntryContent(
    PVDIR_ENTRY pEntry
    );

/*
 * free heap allocated entry (used in but not all ADD_REQUEST type entry)
 */
void
VmDirFreeEntry(
    PVDIR_ENTRY pEntry
    );

void
VmDirFreeEntryArrayContent(
    PVDIR_ENTRY_ARRAY   pArray
    );

void
VmDirFreeEntryArray(
    PVDIR_ENTRY_ARRAY   pEntryAry
    );

/*
 * if success, pEntry takes ownership of pAttr.
 */
DWORD
VmDirEntryAddAttribute(
    PVDIR_ENTRY        pEntry,
    PVDIR_ATTRIBUTE    pAttr
    );

/*
 * Add an array of bervalue attribute values into an entry.
 */
DWORD
VmDirEntryAddBervArrayAttribute(
    PVDIR_ENTRY     pEntry,
    PCSTR           pszAttrName,
    VDIR_BERVARRAY  attrVals,
    USHORT          usNumVals
    );

/*
 * add a single "string" type value attribute to entry.
 */
DWORD
VmDirEntryAddSingleValueStrAttribute(
    PVDIR_ENTRY pEntry,
    PCSTR pszAttrName,
    PCSTR pszAttrValue
    );

/*
 * add a single value attribute to entry.
 */
DWORD
VmDirEntryAddSingleValueAttribute(
    PVDIR_ENTRY pEntry,
    PCSTR pszAttrName,
    PCSTR pszAttrValue,
    size_t iAttrValueLen
    );

/*
 * remove an attribute of an entry.
 */
DWORD
VmDirEntryRemoveAttribute(
    PVDIR_ENTRY     pEntry,
    PCSTR           pszName
    );

/*
 * find attribute(pszName) in pEntry
 */
PVDIR_ATTRIBUTE
VmDirEntryFindAttribute(
    PSTR pszName,
    PVDIR_ENTRY pEntry
    );

DWORD
VmDirAttributeInitialize(
    PSTR    pszName,
    USHORT  usBerSize,
    PVDIR_SCHEMA_CTX pCtx,
    PVDIR_ATTRIBUTE pAttr
    );

VOID
VmDirFreeAttrValueMetaDataContent(
    PDEQUE  pValueMetaData
    );

VOID
VmDirFreeAttribute(
    PVDIR_ATTRIBUTE pAttr
    );

DWORD
VmDirAttributeAllocate(
    PCSTR               pszName,
    USHORT              usBerSize,
    PVDIR_SCHEMA_CTX    pCtx,
    PVDIR_ATTRIBUTE*    ppOutAttr
    );


DWORD
VmDirAttributeDup(
    PVDIR_ATTRIBUTE  pAttr,
    PVDIR_ATTRIBUTE* ppDupAttr
    );

DWORD
VmDirStringToBervalContent(
    PCSTR              pszBerval,
    PVDIR_BERVALUE     pDupBerval
    );

VOID
VmDirFreeBervalArrayContent(
    PVDIR_BERVALUE pBervs,
    USHORT  usSize
    );

BOOLEAN
VmDirIsInternalEntry(
    PVDIR_ENTRY pEntry
    );

BOOLEAN
VmDirEntryIsObjectclass(
    PVDIR_ENTRY     pEntry,
    PCSTR           pszOCName
    );

DWORD
VmDirEntryIsAttrAllowed(
    PVDIR_ENTRY pEntry,
    PSTR        pszAttrName,
    PBOOLEAN    pbMust,
    PBOOLEAN    pbMay
    );

/*
 * free a heap allocated bervalue, bervalue.bv_val and bervalue.bvnorm_val
 */
VOID
VmDirFreeBerval(
    VDIR_BERVALUE* pBerv
    );

/*
 * free bervalue.bvnorm_val and bervalue.bv_val
 */
VOID
VmDirFreeBervalContent(
    VDIR_BERVALUE *pBerv);

DWORD
VmDirBervalContentDup(
    PVDIR_BERVALUE     pBerval,
    PVDIR_BERVALUE     pDupBerval
    );

DWORD
VmDirAttrListToNewEntry(
    PVDIR_SCHEMA_CTX    pSchemaCtx,
    PSTR                pszDN,
    PSTR*               ppszAttrList,
    BOOLEAN             bAllowAnonymousRead,
    PVDIR_ENTRY*        ppEntry
    );

PVDIR_ATTRIBUTE
VmDirFindAttrByName(
    PVDIR_ENTRY      pEntry,
    PSTR        pszName
    );

DWORD
VmDirEntryReplaceAttribute(
    PVDIR_ENTRY     pEntry,
    PVDIR_ATTRIBUTE pNewAttr
    );

DWORD
VmDirDeleteEntry(
    PVDIR_ENTRY pEntry
    );

// util.c
DWORD
VmDirToLDAPError(
    DWORD   dwVmDirError
    );

void const *
UtdVectorEntryGetKey(
    PLW_HASHTABLE_NODE     pNode,
    PVOID                  pUnused
    );

int
VmDirQsortPPCHARCmp(
    const void*		ppStr1,
    const void*		ppStr2
    );

int
VmDirQsortPEIDCmp(
    const void*     pEID1,
    const void*     pEID2
    );

void
VmDirCurrentGeneralizedTime(
    PSTR    pszTimeBuf,
    int     iBufSize
    );

void
VmDirCurrentGeneralizedTimeWithOffset(
    PSTR    pszTimeBuf,
    int     iBufSize,
    DWORD   dwOffset
    );

VOID
VmDirForceExit(
    VOID
    );

DWORD
VmDirUuidFromString(
    PCSTR pStr,
    uuid_t* pGuid
);

DWORD
VmDirFQDNToDNSize(
    PCSTR pszFQDN,
    UINT32 *sizeOfDN
);

DWORD
VmDirFQDNToDN(
    PCSTR pszFQDN,
    PSTR* ppszDN
);

VOID
VmDirLogStackFrame(
    int     logLevel
    );

DWORD
VmDirSrvCreateDN(
    PCSTR pszContainerName,
    PCSTR pszDomainDN,
    PSTR* ppszContainerDN
    );

DWORD
VmDirSrvCreateServerObj(
    PVDIR_SCHEMA_CTX pSchemaCtx
    );

DWORD
VmDirSrvCreateReplAgrsContainer(
    PVDIR_SCHEMA_CTX pSchemaCtx);

DWORD
VmDirKrbInit(
    VOID
    );

DWORD
VmDirSrvCreateContainerWithEID(
    PVDIR_SCHEMA_CTX pSchemaCtx,
    PCSTR            pszContainerDN,
    PCSTR            pszContainerName,
    PVMDIR_SECURITY_DESCRIPTOR pSecDesc,
    ENTRYID          eID);

DWORD
VmDirSrvCreateContainer(
    PVDIR_SCHEMA_CTX pSchemaCtx,
    PCSTR            pszContainerDN,
    PCSTR            pszContainerName);

DWORD
VmDirSrvCreateDomain(
    PVDIR_SCHEMA_CTX pSchemaCtx,
    BOOLEAN          bSetupHost,
    PCSTR            pszDomainDN
    );

DWORD
VmDirFindMemberOfAttribute(
    PVDIR_ENTRY pEntry,
    PVDIR_ATTRIBUTE* ppMemberOfAttr
    );

DWORD
VmDirBuildMemberOfAttribute(
    PVDIR_OPERATION     pOperation,
    PVDIR_ENTRY         pEntry,
    PVDIR_ATTRIBUTE*    ppComputedAttr
    );

DWORD
VmDirSASLGSSBind(
     LDAP*  pLD
     );

DWORD
VmDirUPNToDN(
    PCSTR           pszUPN,
    PSTR*           ppszEntryDN
    );

DWORD
VmDirUPNToDNBerWrap(
    PCSTR           pszUPN,
    PVDIR_BERVALUE  pBervDN
    );

DWORD
VmDirIsAncestorDN(
    PVDIR_BERVALUE  pBervAncestorDN,
    PVDIR_BERVALUE  pBervTargetDN,
    PBOOLEAN        pbResult
    );

DWORD
VmDirHasSingleAttrValue(
    PVDIR_ATTRIBUTE pAttr
    );

DWORD
VmDirValidatePrincipalName(
    PVDIR_ATTRIBUTE pAttr,
    PSTR*           ppErrMsg
    );

DWORD
VmDirSrvGetDomainFunctionalLevel(
    PDWORD pdwLevel
    );

BOOLEAN
VmDirValidValueMetaEntry(
    PVDIR_BERVALUE  pValueMetaData
    );

PCSTR
VmDirLdapModOpTypeToName(
    VDIR_LDAP_MOD_OP modOp
    );

PCSTR
VmDirLdapReqCodeToName(
    ber_tag_t reqCode
    );

PCSTR
VmDirOperationTypeToName(
    VDIR_OPERATION_TYPE opType
    );

// candidates.c
void
AndFilterResults(
    VDIR_FILTER * src,
    VDIR_FILTER * dst);

void
DeleteCandidates(
    PVDIR_CANDIDATES* ppCans);

DWORD
VmDirAddToCandidates(
    PVDIR_CANDIDATES    pCands,
    ENTRYID             eId
    );

PVDIR_CANDIDATES
NewCandidates(
    int      startAllocSize,
    BOOLEAN  positive);

void
NotFilterResults(
    VDIR_FILTER * src,
    VDIR_FILTER * dst);

void
OrFilterResults(
    VDIR_FILTER * src,
    VDIR_FILTER * dst);

// entryencodedecode.c
DWORD
VmDirComputeEncodedEntrySize(
    PVDIR_ENTRY     pEntry,
    int *           nAttrs,
    int *           nVals,
    ber_len_t*      pEncodedEntrySize);

DWORD
VmDirEncodeEntry(
    PVDIR_ENTRY              pEntry,
    VDIR_BERVALUE*           pEncodedBerval);

unsigned short
VmDirDecodeShort(
    unsigned char ** ppbuf);

void
VmDirEncodeShort(
    unsigned char ** ppbuf,
    ber_len_t        len);

DWORD
VmDirDecodeEntry(
   PVDIR_SCHEMA_CTX     pSchemaCtx,
   PVDIR_ENTRY          pEntry);

int
VmDirGenOriginatingTimeStr(
    char * timeStr);

// oprequestutil.c

void
VmDirModificationFree(
    PVDIR_MODIFICATION pMod
    );

DWORD
VmDirOperationAddModReq(
    PVDIR_OPERATION   pOperation,
    int               modOp,
    char *            pszAttrName,
    PVDIR_BERVALUE    pBerValue,
    size_t            iBerValueSize
    );

DWORD
VmDirAppendAMod(
    PVDIR_OPERATION   pOperation,
    int          modOp,
    const char*  attrName,
    int          attrNameLen,
    const char*  attrVal,
    size_t       attrValLen
    );

DWORD
VmDirSimpleEntryCreate(
    PVDIR_SCHEMA_CTX    pSchemaCtx,
    PSTR*               ppszEntryInitializer,
    PSTR                pszDN,
    ENTRYID             ulEntryId
    );

DWORD
VmDirSimpleEntryCreateWithGuid(
    PVDIR_SCHEMA_CTX    pSchemaCtx,
    PSTR*               ppszEntryInitializer,
    PSTR                pszDN,
    ENTRYID             ulEntryId,
    PSTR                pszGuid
    );

DWORD
VmDirModAddSingleValueAttribute(
    PVDIR_MODIFICATION      pMod,
    PVDIR_SCHEMA_CTX        pSchemaCtx,
    PCSTR                   pszAttrName,
    PCSTR                   pszAttrValue,
    size_t                  iAttrValueLen
    );

DWORD
VmDirModAddSingleStrValueAttribute(
    PVDIR_MODIFICATION      pMod,
    PVDIR_SCHEMA_CTX        pSchemaCtx,
    PCSTR                   pszAttrName,
    PCSTR                   pszAttrValue
    );

// ldap-head/operation.c
DWORD
VmDirInitStackOperation(
    PVDIR_OPERATION         pOp,
    VDIR_OPERATION_TYPE     opType,
    ber_tag_t               requestCode,
    PVDIR_SCHEMA_CTX        pSchemaCtx
    );

void
VmDirFreeOperationContent(
    PVDIR_OPERATION     pOp
    );

// middle-layer search.c
DWORD
VmDirSimpleEqualFilterInternalSearch(
    PCSTR               pszBaseDN,
    int                 searchScope,
    PCSTR               pszAttrName,
    PCSTR               pszAttrValue,
    PVDIR_ENTRY_ARRAY   pEntryArray
    );

DWORD
VmDirFilterInternalSearch(
        PCSTR               pszBaseDN,
        int                 searchScope,
        PCSTR               pszFilter,
        unsigned long       ulPageSize,
        PSTR                *ppszPageCookie,
        PVDIR_ENTRY_ARRAY   pEntryArray
    );

// middle-layer result.c
int
VmDirSendSearchEntry(
   PVDIR_OPERATION     pOperation,
   PVDIR_ENTRY         pSrEntry
   );

// middle-layer password.c
DWORD
VdirPasswordCheck(
    PVDIR_BERVALUE      pClearTextPassword,
    PVDIR_ENTRY         pEntry
    );

// security-sd.c
DWORD
VmDirSetGroupSecurityDescriptor(
    PSECURITY_DESCRIPTOR_ABSOLUTE SecurityDescriptor,
    PSID Group,
    BOOLEAN IsGroupDefaulted
    );

ULONG
VmDirLengthSid(
    PSID Sid
    );

DWORD
VmDirCreateAcl(
    PACL Acl,
    ULONG AclLength,
    ULONG AclRevision
    );

DWORD
VmDirGetAce(
    PACL pAcl,
    ULONG dwIndex,
    PACE_HEADER *ppAce
    );

DWORD
VmDirAddAccessAllowedAceEx(
    PACL Acl,
    ULONG AceRevision,
    ULONG AceFlags,
    ACCESS_MASK AccessMask,
    PSID Sid
    );

DWORD
VmDirAddAccessDeniedAceEx(
    PACL Acl,
    ULONG AceRevision,
    ULONG AceFlags,
    ACCESS_MASK AccessMask,
    PSID Sid
    );

DWORD
VmDirSetDaclSecurityDescriptor(
    PSECURITY_DESCRIPTOR_ABSOLUTE SecurityDescriptor,
    BOOLEAN IsDaclPresent,
    PACL Dacl,
    BOOLEAN IsDaclDefaulted
    );

BOOLEAN
VmDirValidSecurityDescriptor(
    PSECURITY_DESCRIPTOR_ABSOLUTE SecurityDescriptor
    );

DWORD
VmDirAbsoluteToSelfRelativeSD(
    PSECURITY_DESCRIPTOR_ABSOLUTE AbsoluteSecurityDescriptor,
    PSECURITY_DESCRIPTOR_RELATIVE SelfRelativeSecurityDescriptor,
    PULONG BufferLength
    );

DWORD
VmDirQuerySecurityDescriptorInfo(
    SECURITY_INFORMATION SecurityInformationNeeded,
    PSECURITY_DESCRIPTOR_RELATIVE SecurityDescriptorInput,
    PSECURITY_DESCRIPTOR_RELATIVE SecurityDescriptorOutput,
    PULONG Length
    );

DWORD
VmDirSelfRelativeToAbsoluteSD(
    PSECURITY_DESCRIPTOR_RELATIVE SelfRelativeSecurityDescriptor,
    PSECURITY_DESCRIPTOR_ABSOLUTE AbsoluteSecurityDescriptor,
    PULONG AbsoluteSecurityDescriptorSize,
    PACL pDacl,
    PULONG pDaclSize,
    PACL pSacl,
    PULONG pSaclSize,
    PSID Owner,
    PULONG pOwnerSize,
    PSID PrimaryGroup,
    PULONG pPrimaryGroupSize
    );

BOOLEAN
VmDirAccessCheck(
    PSECURITY_DESCRIPTOR_ABSOLUTE SecurityDescriptor,
    PACCESS_TOKEN AccessToken,
    ACCESS_MASK DesiredAccess,
    ACCESS_MASK PreviouslyGrantedAccess,
    PGENERIC_MAPPING GenericMapping,
    PACCESS_MASK GrantedAccess,
    PDWORD pAccessError
    );

DWORD
VmDirGetOwnerSecurityDescriptor(
    PSECURITY_DESCRIPTOR_ABSOLUTE SecurityDescriptor,
    PSID* Owner,
    PBOOLEAN pIsOwnerDefaulted
    );

DWORD
VmDirGetGroupSecurityDescriptor(
    PSECURITY_DESCRIPTOR_ABSOLUTE SecurityDescriptor,
    PSID* Group,
    PBOOLEAN pIsGroupDefaulted
    );

DWORD
VmDirGetDaclSecurityDescriptor(
    PSECURITY_DESCRIPTOR_ABSOLUTE SecurityDescriptor,
    PBOOLEAN pIsDaclPresent,
    PACL* Dacl,
    PBOOLEAN pIsDaclDefaulted
    );

DWORD
VmDirGetSaclSecurityDescriptor(
    PSECURITY_DESCRIPTOR_ABSOLUTE SecurityDescriptor,
    PBOOLEAN pIsSaclPresent,
    PACL* Sacl,
    PBOOLEAN pIsSaclDefaulted
    );

BOOLEAN
VmDirValidRelativeSecurityDescriptor(
    PSECURITY_DESCRIPTOR_RELATIVE SecurityDescriptor,
    ULONG SecurityDescriptorLength,
    SECURITY_INFORMATION RequiredInformation
    );

DWORD
VmDirSetSecurityDescriptorInfo(
    SECURITY_INFORMATION SecurityInformation,
    PSECURITY_DESCRIPTOR_RELATIVE InputSecurityDescriptor,
    PSECURITY_DESCRIPTOR_RELATIVE ObjectSecurityDescriptor,
    PSECURITY_DESCRIPTOR_RELATIVE NewObjectSecurityDescriptor,
    PULONG NewObjectSecurityDescripptorLength,
    PGENERIC_MAPPING GenericMapping
    );

DWORD
VmDirCreateSecurityDescriptorAbsolute(
    PSECURITY_DESCRIPTOR_ABSOLUTE *ppSecurityDescriptor
    );

VOID
VmDirReleaseAccessToken(
    PACCESS_TOKEN* AccessToken
    );

DWORD
VmDirSetOwnerSecurityDescriptor(
    PSECURITY_DESCRIPTOR_ABSOLUTE SecurityDescriptor,
    PSID Owner,
    BOOLEAN IsOwnerDefaulted
    );

DWORD
VmDirCreateWellKnownSid(
    WELL_KNOWN_SID_TYPE wellKnownSidType,
    PSID pDomainSid,
    PSID pSid,
    DWORD* pcbSid
);

VOID
VmDirMapGenericMask(
    PDWORD pdwAccessMask,
    PGENERIC_MAPPING pGenericMapping
);

DWORD
VmDirQueryAccessTokenInformation(
    HANDLE hTokenHandle,
    TOKEN_INFORMATION_CLASS tokenInformationClass,
    PVOID pTokenInformation,
    DWORD dwTokenInformationLength,
    PDWORD pdwReturnLength
);

DWORD
VmDirAllocateSddlCStringFromSecurityDescriptor(
    PSECURITY_DESCRIPTOR_RELATIVE pSecurityDescriptor,
    DWORD dwRequestedStringSDRevision,
    SECURITY_INFORMATION securityInformation,
    PSTR* ppStringSecurityDescriptor
);

DWORD
VmDirSetSecurityDescriptorControl(
    PSECURITY_DESCRIPTOR_ABSOLUTE pSecurityDescriptor,
    SECURITY_DESCRIPTOR_CONTROL BitsToChange,
    SECURITY_DESCRIPTOR_CONTROL BitsToSet
    );

// srp.c
DWORD
VmDirSRPCreateSecret(
    PVDIR_BERVALUE   pUPN,
    PVDIR_BERVALUE   pClearTextPasswd,
    PVDIR_BERVALUE   pSecretResult
    );

//server/common/urgentrepl.c
BOOLEAN
VmDirPerformUrgentReplication(
    PVDIR_OPERATION pOperation,
    USN currentTxnUSN
    );

VOID
VmDirRetryUrgentReplication(
    VOID
    );

VOID
VmDirPerformUrgentReplIfRequired(
    PVDIR_OPERATION pOperation,
    USN currentTxnUSN
    );

//vmafdlib.c
DWORD
VmDirOpenVmAfdClientLib(
    VMDIR_LIB_HANDLE*   pplibHandle
    );

DWORD
VmDirKeySetGetKvno(
    PBYTE pUpnKeys,
    DWORD upnKeysLen,
    DWORD *kvno
);

#ifdef __cplusplus
}
#endif

#endif /* COMMON_INTERFACE_H_ */
