#include <lw/types.h>

typedef struct _RPC_UNICODE_STRING
{
    unsigned short Length;
    unsigned short MaximumLength;
#ifdef _DCE_IDL_
    [size_is(MaximumLength/2), length_is(Length/2)]
#endif
    WCHAR *Buffer;
} RPC_UNICODE_STRING, *PRPC_UNICODE_STRING;

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

typedef struct _VMDIR_GROUP_MEMBERSHIP
{
    ULONG Identifier[2];
    ULONG Attributes;
} VMDIR_GROUP_MEMBERSHIP, *PVMDIR_GROUP_MEMBERSHIP;

typedef struct _VMDIR_AUTHZ_INFO
{
    RPC_UNICODE_STRING AccountName;
    PISID UserSid;
    RPC_UNICODE_STRING DomainName;
    PISID DomainSid;
    ULONG GroupIdCount;
#ifdef _DCE_IDL_
    [size_is(GroupIdCount)]
#endif
    PVMDIR_GROUP_MEMBERSHIP GroupIds;
    ULONG OtherSidCount;
#ifdef _DCE_IDL_
    [size_is(OtherSidCount)]
#endif
    PKERB_SID_AND_ATTRIBUTES OtherSids;
} VMDIR_AUTHZ_INFO, *PVMDIR_AUTHZ_INFO;

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
