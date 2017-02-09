#include <lw/types.h>

typedef struct _FILETIME2
{
    UINT32 dwLowDateTime;
    UINT32 dwHighDateTime;
} FILETIME2, *PFILETIME2;

typedef struct _RPC_UNICODE_STRING
{
    unsigned short Length;
    unsigned short MaximumLength;
#ifdef _DCE_IDL_
    [size_is(MaximumLength/2), length_is(Length/2)]
#endif
    WCHAR *Buffer;
} RPC_UNICODE_STRING, *PRPC_UNICODE_STRING;

typedef struct _CYPHER_BLOCK
{
    CHAR data[8];
} CYPHER_BLOCK;
    
typedef struct _GROUP_MEMBERSHIP
{
    ULONG RelativeId;
    ULONG Attributes;
} GROUP_MEMBERSHIP, *PGROUP_MEMBERSHIP;

typedef struct _USER_SESSION_KEY
{
    CYPHER_BLOCK data[2];
} USER_SESSION_KEY;

typedef struct _RPC_SID_IDENTIFIER_AUTHORITY
{
    UCHAR Value[6];
} RPC_SID_IDENTIFIER_AUTHORITY;

typedef struct _RPC_SID
{
    unsigned char Revision;
    UINT8 SubAuthorityCount;
    RPC_SID_IDENTIFIER_AUTHORITY IdentifierAuthority;
#ifdef _DCE_IDL_
    [size_is(SubAuthorityCount)]
#endif
    unsigned long SubAuthority[];
} RPC_SID, *PRPC_SID;

typedef RPC_SID *PISID;

typedef struct _KERB_SID_AND_ATTRIBUTES
{
    PISID Sid;
    ULONG Attributes;
} KERB_SID_AND_ATTRIBUTES, *PKERB_SID_AND_ATTRIBUTES;



/* -------------------- PAC_INFO ------------------- */

typedef struct _KERB_VALIDATION_INFO
{
    FILETIME2 LogonTime;
    FILETIME2 LogOffTime;
    FILETIME2 KickOffTime;
    FILETIME2 PasswordLastSet;
    FILETIME2 PasswordCanChange;
    FILETIME2 PasswordMustChange;
    RPC_UNICODE_STRING EffectiveName;
    RPC_UNICODE_STRING FullName;
    RPC_UNICODE_STRING LogonScript;
    RPC_UNICODE_STRING ProfilePath;
    RPC_UNICODE_STRING HomeDirectory;
    RPC_UNICODE_STRING HomeDirectoryDrive;
    USHORT LogonCount;
    USHORT BadPasswordCount;
    ULONG UserId;
    ULONG PrimaryGroupId;
    ULONG GroupCount;
#ifdef _DCE_IDL_
    [size_is(GroupCount)]
#endif
    PGROUP_MEMBERSHIP GroupIds;
    ULONG UserFlags;
    USER_SESSION_KEY UserSessionKey;
    RPC_UNICODE_STRING LogonServer;
    RPC_UNICODE_STRING LogonDomainName;
    PISID LogonDomainId;
    ULONG Reserved1[2];
    ULONG UserAccountControl;
    ULONG SubAuthStatus;
    FILETIME2 LastSuccessfulILogon;
    FILETIME2 LastFailedILogon;
    ULONG FailedILogonCount;
    ULONG Reserved3;
    ULONG SidCount;
#ifdef _DCE_IDL_
    [size_is(SidCount)]
#endif
    PKERB_SID_AND_ATTRIBUTES ExtraSids;
    PISID ResourceGroupDomainSid;
    ULONG ResourceGroupCount;
#ifdef _DCE_IDL_
    [size_is(ResourceGroupCount)]
#endif
    PGROUP_MEMBERSHIP ResourceGroupIds;
} KERB_VALIDATION_INFO;

typedef struct _MES_header
{
    UINT8 Version;
    UINT8 Endianness;
    UINT16 CommonHeaderLength;
    UINT32 Filler1;
    UINT32 ObjectBufferLength;
    UINT32 Filler2;
    UINT32 Referent;
} MES_header, *PMES_header;
