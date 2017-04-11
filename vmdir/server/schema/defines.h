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
 * Module Name: Directory Schema
 *
 * Filename: defines.h
 *
 * Abstract:
 *
 * Directory schema module
 *
 * Definitions
 *
 */

#define SCHEMA_ATTR_ID_MAP_SEP         "="

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

#define ASCII_SPACE(c) ((c) == ' ')

/*
    printablestring = 1*p
    p               = a / d / """ / "(" / ")" / "+" / "," /
                      "-" / "." / "/" / ":" / "?" / " "
 */
#define ASCII_LOWER(c)    ( (c) >= 'a' && (c) <= 'z' )
#define ASCII_UPPER(c)    ( (c) >= 'A' && (c) <= 'Z' )
#define ASCII_ALPHA(c)    ( ASCII_LOWER(c) || ASCII_UPPER(c) )
#define ASCII_DIGIT(c)    ( (c) >= '0' && (c) <= '9' )
#define VDIR_PRINTABLE_CHAR(c)    (    \
    ASCII_ALPHA(c)  ||   \
    ASCII_DIGIT(c)  ||   \
    (c) == '"' ||        \
    (c) == '(' ||        \
    (c) == ')' ||        \
    (c) == '+' ||        \
    (c) == ',' ||        \
    (c) == '-' ||        \
    (c) == '.' ||        \
    (c) == '/' ||        \
    (c) == ':' ||        \
    (c) == '?' ||        \
    (c) == ' ' )


#define VDIR_ASCII(c)            ( ((c) & 0x80) == 0 )
#define VDIR_IA5_CHAR(c)        ( VDIR_ASCII(c) )
#define VDIR_NUMERIC_CHAR(c)    ( ASCII_DIGIT(c) || ASCII_SPACE(c) )
#define VDIR_ASCII_SPACE(c)        ( (c) == ' ' )

#define VIDR_ASCII_UPPER_TO_LOWER(c, f) \
    if ( ASCII_UPPER(c))                \
    {                                   \
        (c) = ((c) + 32);               \
        (f) = TRUE;                     \
    }


#define VDIR_BERBV_ISNULL(bv)       ((bv) == NULL)
#define VDIR_BERBV_ISEMPTY(bv)      ((bv)->lberbv.bv_len == 0 || (bv)->lberbv.bv_val == NULL)

/*  RFC 2251, most not implemented yet.
   ACI Item                        N  1.3.6.1.4.1.1466.115.121.1.1
   Access Point                    Y  1.3.6.1.4.1.1466.115.121.1.2
   Attribute Type Description      Y  1.3.6.1.4.1.1466.115.121.1.3
   Audio                           N  1.3.6.1.4.1.1466.115.121.1.4
   Binary                          N  1.3.6.1.4.1.1466.115.121.1.5
   Bit String                      Y  1.3.6.1.4.1.1466.115.121.1.6
   Boolean                         Y  1.3.6.1.4.1.1466.115.121.1.7
   Certificate                     N  1.3.6.1.4.1.1466.115.121.1.8
   Certificate List                N  1.3.6.1.4.1.1466.115.121.1.9
   Certificate Pair                N  1.3.6.1.4.1.1466.115.121.1.10
   Country String                  Y  1.3.6.1.4.1.1466.115.121.1.11
   DN                              Y  1.3.6.1.4.1.1466.115.121.1.12
   Data Quality Syntax             Y  1.3.6.1.4.1.1466.115.121.1.13
   Delivery Method                 Y  1.3.6.1.4.1.1466.115.121.1.14
   Directory String                Y  1.3.6.1.4.1.1466.115.121.1.15
   DIT Content Rule Description    Y  1.3.6.1.4.1.1466.115.121.1.16
   DIT Structure Rule Description  Y  1.3.6.1.4.1.1466.115.121.1.17
   DL Submit Permission            Y  1.3.6.1.4.1.1466.115.121.1.18
   DSA Quality Syntax              Y  1.3.6.1.4.1.1466.115.121.1.19
   DSE Type                        Y  1.3.6.1.4.1.1466.115.121.1.20
   Enhanced Guide                  Y  1.3.6.1.4.1.1466.115.121.1.21
   Facsimile Telephone Number      Y  1.3.6.1.4.1.1466.115.121.1.22
   Fax                             N  1.3.6.1.4.1.1466.115.121.1.23
   Generalized Time                Y  1.3.6.1.4.1.1466.115.121.1.24
   Guide                           Y  1.3.6.1.4.1.1466.115.121.1.25
   IA5 String                      Y  1.3.6.1.4.1.1466.115.121.1.26
   INTEGER                         Y  1.3.6.1.4.1.1466.115.121.1.27
   JPEG                            N  1.3.6.1.4.1.1466.115.121.1.28
   LDAP Syntax Description         Y  1.3.6.1.4.1.1466.115.121.1.54
   LDAP Schema Definition          Y  1.3.6.1.4.1.1466.115.121.1.56
   LDAP Schema Description         Y  1.3.6.1.4.1.1466.115.121.1.57
   Master And Shadow Access Points Y  1.3.6.1.4.1.1466.115.121.1.29
   Matching Rule Description       Y  1.3.6.1.4.1.1466.115.121.1.30
   Matching Rule Use Description   Y  1.3.6.1.4.1.1466.115.121.1.31
   Mail Preference                 Y  1.3.6.1.4.1.1466.115.121.1.32
   MHS OR Address                  Y  1.3.6.1.4.1.1466.115.121.1.33
   Modify Rights                   Y  1.3.6.1.4.1.1466.115.121.1.55
   Name And Optional UID           Y  1.3.6.1.4.1.1466.115.121.1.34
   Name Form Description           Y  1.3.6.1.4.1.1466.115.121.1.35
   Numeric String                  Y  1.3.6.1.4.1.1466.115.121.1.36
   Object Class Description        Y  1.3.6.1.4.1.1466.115.121.1.37
   Octet String                    Y  1.3.6.1.4.1.1466.115.121.1.40
   OID                             Y  1.3.6.1.4.1.1466.115.121.1.38
   Other Mailbox                   Y  1.3.6.1.4.1.1466.115.121.1.39
   Postal Address                  Y  1.3.6.1.4.1.1466.115.121.1.41
   Protocol Information            Y  1.3.6.1.4.1.1466.115.121.1.42
   Presentation Address            Y  1.3.6.1.4.1.1466.115.121.1.43
   Printable String                Y  1.3.6.1.4.1.1466.115.121.1.44
   Substring Assertion             Y  1.3.6.1.4.1.1466.115.121.1.58
   Subtree Specification           Y  1.3.6.1.4.1.1466.115.121.1.45
   Supplier Information            Y  1.3.6.1.4.1.1466.115.121.1.46
   Supplier Or Consumer            Y  1.3.6.1.4.1.1466.115.121.1.47
   Supplier And Consumer           Y  1.3.6.1.4.1.1466.115.121.1.48
   Supported Algorithm             N  1.3.6.1.4.1.1466.115.121.1.49
   Telephone Number                Y  1.3.6.1.4.1.1466.115.121.1.50
   Teletex Terminal Identifier     Y  1.3.6.1.4.1.1466.115.121.1.51
   Telex Number                    Y  1.3.6.1.4.1.1466.115.121.1.52
   UTC Time                        Y  1.3.6.1.4.1.1466.115.121.1.53
 */

// RFC Syntaxes
#define VDIR_SYNTAX_ATTRIBUTE_TYPE_DESC     "Attribute Type Description"
#define VDIR_SYNTAX_BINARY                  "Binary"
#define VDIR_SYNTAX_BIT_STRING              "Bit String"
#define VDIR_SYNTAX_BOOLEAN                 "Boolean"
#define VDIR_SYNTAX_DN                      "DN"
#define VDIR_SYNTAX_DIRECTORY_STRING        "Directory String"
#define VDIR_SYNTAX_DIT_STRUCTURE_RULE_DESC "DIT Structure Rule Description"
#define VDIR_SYNTAX_ENHANCED_GUID           "Enhanced Guide"
#define VDIR_SYNTAX_FAX_NUMBER              "Facsimile Telephone Number"
#define VDIR_SYNTAX_GEN_TIME                "Generalized Time"
#define VDIR_SYNTAX_IA5_STRING              "IA5 String"
#define VDIR_SYNTAX_INTERGER                "INTEGER"
#define VDIR_SYNTAX_NAME_FORM_DESC          "Name Form Description"
#define VDIR_SYNTAX_NUMERIC_STRING          "Numeric String"
#define VDIR_SYNTAX_OBJECTCLASS_DESC        "Object Class Description"
#define VDIR_SYNTAX_OCTET_STRING            "Octet String"
#define VDIR_SYNTAX_OID                     "OID"
#define VDIR_SYNTAX_POSTAL_ADDRESS          "Postal Address"
#define VDIR_SYNTAX_PRESENTATION_ADDRESS    "Presentation Address"
#define VDIR_SYNTAX_PRINTABLE_STRING        "Printable String"
#define VDIR_SYNTAX_PROTOCOL_INFORMATION    "Protocol Information"
#define VDIR_SYNTAX_SUPPORTED_ALGORITHM     "Supported Algorithm"
#define VDIR_SYNTAX_TELEPHONE_NUMBER        "Telephone Number"
#define VDIR_SYNTAX_UTC_TIME                "UTC Time"
#define VDIR_SYNTAX_UUID                    "UUID"

// AD Syntaxes
#define VDIR_SYNTAX_LARGE_INTEGER           "LargeInteger"
#define VDIR_SYNTAX_OBJECT_DN_BINARY        "Object(DN-Binary)"
#define VDIR_SYNTAX_OBJECT_DN_STRING        "Object(DN-String)"
#define VDIR_SYNTAX_OBJECT_REPLICA_LINK     "Object(Replica-Link)"
#define VDIR_SYNTAX_STRING_NT_SEC_DESC      "String(NT-Sec-Desc)"
#define VDIR_SYNTAX_STRING_TELETEX          "String(Teletex)"

// RFC Syntax OIDs
#define VDIR_OID_ATTRIBUTE_TYPE_DESC        "1.3.6.1.4.1.1466.115.121.1.3"
#define VDIR_OID_BINARY                     "1.3.6.1.4.1.1466.115.121.1.5"
#define VDIR_OID_BIT_STRING                 "1.3.6.1.4.1.1466.115.121.1.6"
#define VDIR_OID_BOOLEAN                    "1.3.6.1.4.1.1466.115.121.1.7"
#define VDIR_OID_DN                         "1.3.6.1.4.1.1466.115.121.1.12"
#define VDIR_OID_DIRECTORY_STRING           "1.3.6.1.4.1.1466.115.121.1.15"
#define VDIR_OID_DIT_STRUCTURE_RULE_DESC    "1.3.6.1.4.1.1466.115.121.1.17"
#define VDIR_OID_ENHANCED_GUID              "1.3.6.1.4.1.1466.115.121.1.21"
#define VDIR_OID_FAX_NUMBER                 "1.3.6.1.4.1.1466.115.121.1.22"
#define VDIR_OID_GEN_TIME                   "1.3.6.1.4.1.1466.115.121.1.24"
#define VDIR_OID_IA5_STRING                 "1.3.6.1.4.1.1466.115.121.1.26"
#define VDIR_OID_INTERGER                   "1.3.6.1.4.1.1466.115.121.1.27"
#define VDIR_OID_NAME_FORM_DESC             "1.3.6.1.4.1.1466.115.121.1.35"
#define VDIR_OID_NUMERIC_STRING             "1.3.6.1.4.1.1466.115.121.1.36"
#define VDIR_OID_OBJECTCLASS_DESC           "1.3.6.1.4.1.1466.115.121.1.37"
#define VDIR_OID_OCTET_STRING               "1.3.6.1.4.1.1466.115.121.1.40"
#define VDIR_OID_OID                        "1.3.6.1.4.1.1466.115.121.1.38"
#define VDIR_OID_POSTAL_ADDRESS             "1.3.6.1.4.1.1466.115.121.1.41"
#define VDIR_OID_PRESENTATION_ADDRESS       "1.3.6.1.4.1.1466.115.121.1.43"
#define VDIR_OID_PRINTABLE_STRING           "1.3.6.1.4.1.1466.115.121.1.44"
#define VDIR_OID_PROTOCOL_INFORMATION       "1.3.6.1.4.1.1466.115.121.1.42"
#define VDIR_OID_SUPPORTED_ALGORITHM        "1.3.6.1.4.1.1466.115.121.1.49"
#define VDIR_OID_TELEPHONE_NUMBER           "1.3.6.1.4.1.1466.115.121.1.50"
#define VDIR_OID_UTC_TIME                   "1.3.6.1.4.1.1466.115.121.1.53"
#define VDIR_OID_UUID                       "1.3.6.1.1.16.1"

// AD Syntax OIDs
#define VDIR_OID_LARGE_INTEGER              "1.2.840.113556.1.4.906"
#define VDIR_OID_OBJECT_DN_BINARY           "1.2.840.113556.1.4.903"
#define VDIR_OID_OBJECT_DN_STRING           "1.2.840.113556.1.4.904"
#define VDIR_OID_OBJECT_REPLICA_LINK        "OctetString"
#define VDIR_OID_STRING_NT_SEC_DESC         "1.2.840.113556.1.4.907"
#define VDIR_OID_STRING_TELETEX             "1.2.840.113556.1.4.905"

// NOTE: order of fields MUST stay in sync with struct definition...
#define VDIR_SYNTAX_INIT_TABLE_INITIALIZER                            \
{                                                                     \
    {                                                                 \
        VMDIR_SF_INIT(.pszName, VDIR_SYNTAX_ATTRIBUTE_TYPE_DESC),     \
        VMDIR_SF_INIT(.pszOid, VDIR_OID_ATTRIBUTE_TYPE_DESC),         \
        VMDIR_SF_INIT(.readableFlag, VDIR_SYNTAX_HUMAN_READABLE_YES), \
        VMDIR_SF_INIT(.pValidateFunc, syntaxNotImplemented),          \
        VMDIR_SF_INIT(.bPublic, TRUE)                                 \
    },                                                                \
   {                                                                  \
        VMDIR_SF_INIT(.pszName, VDIR_SYNTAX_BINARY),                  \
        VMDIR_SF_INIT(.pszOid, VDIR_OID_BINARY),                      \
        VMDIR_SF_INIT(.readableFlag, VDIR_SYNTAX_HUMAN_READABLE_NO),  \
        VMDIR_SF_INIT(.pValidateFunc, syntaxBinary),                  \
        VMDIR_SF_INIT(.bPublic, TRUE)                                 \
    },                                                                \
    {                                                                 \
        VMDIR_SF_INIT(.pszName, VDIR_SYNTAX_BIT_STRING),              \
        VMDIR_SF_INIT(.pszOid, VDIR_OID_BIT_STRING),                  \
        VMDIR_SF_INIT(.readableFlag, VDIR_SYNTAX_HUMAN_READABLE_NO),  \
        VMDIR_SF_INIT(.pValidateFunc, syntaxNotImplemented),          \
        VMDIR_SF_INIT(.bPublic, TRUE)                                 \
    },                                                                \
    {                                                                 \
        VMDIR_SF_INIT(.pszName, VDIR_SYNTAX_BOOLEAN),                 \
        VMDIR_SF_INIT(.pszOid, VDIR_OID_BOOLEAN),                     \
        VMDIR_SF_INIT(.readableFlag, VDIR_SYNTAX_HUMAN_READABLE_YES), \
        VMDIR_SF_INIT(.pValidateFunc, syntaxBoolean),                 \
        VMDIR_SF_INIT(.bPublic, TRUE)                                 \
    },                                                                \
    {                                                                 \
        VMDIR_SF_INIT(.pszName, VDIR_SYNTAX_DN),                      \
        VMDIR_SF_INIT(.pszOid, VDIR_OID_DN),                          \
        VMDIR_SF_INIT(.readableFlag, VDIR_SYNTAX_HUMAN_READABLE_YES), \
        VMDIR_SF_INIT(.pValidateFunc, syntaxNotImplemented),          \
        VMDIR_SF_INIT(.bPublic, TRUE)                                 \
    },                                                                \
    {                                                                 \
        VMDIR_SF_INIT(.pszName, VDIR_SYNTAX_DIRECTORY_STRING),        \
        VMDIR_SF_INIT(.pszOid, VDIR_OID_DIRECTORY_STRING),            \
        VMDIR_SF_INIT(.readableFlag, VDIR_SYNTAX_HUMAN_READABLE_YES), \
        VMDIR_SF_INIT(.pValidateFunc, syntaxDirectoryString),         \
        VMDIR_SF_INIT(.bPublic, TRUE)                                 \
    },                                                                \
    {                                                                 \
        VMDIR_SF_INIT(.pszName, VDIR_SYNTAX_DIT_STRUCTURE_RULE_DESC), \
        VMDIR_SF_INIT(.pszOid, VDIR_OID_DIT_STRUCTURE_RULE_DESC),     \
        VMDIR_SF_INIT(.readableFlag, VDIR_SYNTAX_HUMAN_READABLE_YES), \
        VMDIR_SF_INIT(.pValidateFunc, syntaxNotImplemented),          \
        VMDIR_SF_INIT(.bPublic, FALSE)                                \
    },                                                                \
    {                                                                 \
        VMDIR_SF_INIT(.pszName, VDIR_SYNTAX_ENHANCED_GUID),           \
        VMDIR_SF_INIT(.pszOid, VDIR_OID_ENHANCED_GUID),               \
        VMDIR_SF_INIT(.readableFlag, VDIR_SYNTAX_HUMAN_READABLE_YES), \
        VMDIR_SF_INIT(.pValidateFunc, syntaxNotImplemented),          \
        VMDIR_SF_INIT(.bPublic, FALSE)                                \
    },                                                                \
    {                                                                 \
        VMDIR_SF_INIT(.pszName, VDIR_SYNTAX_FAX_NUMBER),              \
        VMDIR_SF_INIT(.pszOid, VDIR_OID_FAX_NUMBER),                  \
        VMDIR_SF_INIT(.readableFlag, VDIR_SYNTAX_HUMAN_READABLE_YES), \
        VMDIR_SF_INIT(.pValidateFunc, syntaxNotImplemented),          \
        VMDIR_SF_INIT(.bPublic, FALSE)                                \
    },                                                                \
    {                                                                 \
        VMDIR_SF_INIT(.pszName, VDIR_SYNTAX_GEN_TIME),                \
        VMDIR_SF_INIT(.pszOid, VDIR_OID_GEN_TIME),                    \
        VMDIR_SF_INIT(.readableFlag, VDIR_SYNTAX_HUMAN_READABLE_YES), \
        VMDIR_SF_INIT(.pValidateFunc, syntaxGeneralizedTime),         \
        VMDIR_SF_INIT(.bPublic, TRUE)                                 \
    },                                                                \
    {                                                                 \
        VMDIR_SF_INIT(.pszName, VDIR_SYNTAX_IA5_STRING),              \
        VMDIR_SF_INIT(.pszOid, VDIR_OID_IA5_STRING),                  \
        VMDIR_SF_INIT(.readableFlag, VDIR_SYNTAX_HUMAN_READABLE_YES), \
        VMDIR_SF_INIT(.pValidateFunc, syntaxIA5String),               \
        VMDIR_SF_INIT(.bPublic, TRUE)                                 \
    },                                                                \
    {                                                                 \
        VMDIR_SF_INIT(.pszName, VDIR_SYNTAX_INTERGER),                \
        VMDIR_SF_INIT(.pszOid, VDIR_OID_INTERGER),                    \
        VMDIR_SF_INIT(.readableFlag, VDIR_SYNTAX_HUMAN_READABLE_YES), \
        VMDIR_SF_INIT(.pValidateFunc, syntaxInteger),                 \
        VMDIR_SF_INIT(.bPublic, TRUE)                                 \
    },                                                                \
    {                                                                 \
        VMDIR_SF_INIT(.pszName, VDIR_SYNTAX_NAME_FORM_DESC),          \
        VMDIR_SF_INIT(.pszOid, VDIR_OID_NAME_FORM_DESC),              \
        VMDIR_SF_INIT(.readableFlag, VDIR_SYNTAX_HUMAN_READABLE_YES), \
        VMDIR_SF_INIT(.pValidateFunc, syntaxNotImplemented),          \
        VMDIR_SF_INIT(.bPublic, FALSE)                                \
    },                                                                \
    {                                                                 \
        VMDIR_SF_INIT(.pszName, VDIR_SYNTAX_NUMERIC_STRING),          \
        VMDIR_SF_INIT(.pszOid, VDIR_OID_NUMERIC_STRING),              \
        VMDIR_SF_INIT(.readableFlag, VDIR_SYNTAX_HUMAN_READABLE_YES), \
        VMDIR_SF_INIT(.pValidateFunc, syntaxNumericString),           \
        VMDIR_SF_INIT(.bPublic, TRUE)                                 \
    },                                                                \
    {                                                                 \
        VMDIR_SF_INIT(.pszName, VDIR_SYNTAX_OCTET_STRING),            \
        VMDIR_SF_INIT(.pszOid, VDIR_OID_OCTET_STRING),                \
        VMDIR_SF_INIT(.readableFlag, VDIR_SYNTAX_HUMAN_READABLE_YES), \
        VMDIR_SF_INIT(.pValidateFunc, syntaxNotImplemented),          \
        VMDIR_SF_INIT(.bPublic, TRUE)                                 \
    },                                                                \
    {                                                                 \
        VMDIR_SF_INIT(.pszName, VDIR_SYNTAX_OBJECTCLASS_DESC),        \
        VMDIR_SF_INIT(.pszOid, VDIR_OID_OBJECTCLASS_DESC),            \
        VMDIR_SF_INIT(.readableFlag, VDIR_SYNTAX_HUMAN_READABLE_YES), \
        VMDIR_SF_INIT(.pValidateFunc, syntaxNotImplemented),          \
        VMDIR_SF_INIT(.bPublic, TRUE)                                 \
    },                                                                \
    {                                                                 \
        VMDIR_SF_INIT(.pszName, VDIR_SYNTAX_OID),                     \
        VMDIR_SF_INIT(.pszOid, VDIR_OID_OID),                         \
        VMDIR_SF_INIT(.readableFlag, VDIR_SYNTAX_HUMAN_READABLE_YES), \
        VMDIR_SF_INIT(.pValidateFunc, syntaxOID),                     \
        VMDIR_SF_INIT(.bPublic, TRUE)                                 \
    },                                                                \
    {                                                                 \
        VMDIR_SF_INIT(.pszName, VDIR_SYNTAX_POSTAL_ADDRESS),          \
        VMDIR_SF_INIT(.pszOid, VDIR_OID_POSTAL_ADDRESS),              \
        VMDIR_SF_INIT(.readableFlag, VDIR_SYNTAX_HUMAN_READABLE_YES), \
        VMDIR_SF_INIT(.pValidateFunc, syntaxNotImplemented),          \
        VMDIR_SF_INIT(.bPublic, FALSE)                                \
    },                                                                \
    {                                                                 \
        VMDIR_SF_INIT(.pszName, VDIR_SYNTAX_PRESENTATION_ADDRESS),    \
        VMDIR_SF_INIT(.pszOid, VDIR_OID_PRESENTATION_ADDRESS),        \
        VMDIR_SF_INIT(.readableFlag, VDIR_SYNTAX_HUMAN_READABLE_YES), \
        VMDIR_SF_INIT(.pValidateFunc, syntaxNotImplemented),          \
        VMDIR_SF_INIT(.bPublic, FALSE)                                \
    },                                                                \
    {                                                                 \
        VMDIR_SF_INIT(.pszName, VDIR_SYNTAX_PRINTABLE_STRING),        \
        VMDIR_SF_INIT(.pszOid, VDIR_OID_PRINTABLE_STRING),            \
        VMDIR_SF_INIT(.readableFlag, VDIR_SYNTAX_HUMAN_READABLE_YES), \
        VMDIR_SF_INIT(.pValidateFunc, syntaxPrintableString),         \
        VMDIR_SF_INIT(.bPublic, TRUE)                                 \
    },                                                                \
    {                                                                 \
        VMDIR_SF_INIT(.pszName, VDIR_SYNTAX_PROTOCOL_INFORMATION),    \
        VMDIR_SF_INIT(.pszOid, VDIR_OID_PROTOCOL_INFORMATION),        \
        VMDIR_SF_INIT(.readableFlag, VDIR_SYNTAX_HUMAN_READABLE_YES), \
        VMDIR_SF_INIT(.pValidateFunc, syntaxNotImplemented),          \
        VMDIR_SF_INIT(.bPublic, FALSE)                                \
    },                                                                \
    {                                                                 \
        VMDIR_SF_INIT(.pszName, VDIR_SYNTAX_SUPPORTED_ALGORITHM),     \
        VMDIR_SF_INIT(.pszOid, VDIR_OID_SUPPORTED_ALGORITHM),         \
        VMDIR_SF_INIT(.readableFlag, VDIR_SYNTAX_HUMAN_READABLE_NO),  \
        VMDIR_SF_INIT(.pValidateFunc, syntaxNotImplemented),          \
        VMDIR_SF_INIT(.bPublic, FALSE)                                \
    },                                                                \
    {                                                                 \
        VMDIR_SF_INIT(.pszName, VDIR_SYNTAX_TELEPHONE_NUMBER),        \
        VMDIR_SF_INIT(.pszOid, VDIR_OID_TELEPHONE_NUMBER),            \
        VMDIR_SF_INIT(.readableFlag, VDIR_SYNTAX_HUMAN_READABLE_YES), \
        VMDIR_SF_INIT(.pValidateFunc, syntaxNotImplemented),          \
        VMDIR_SF_INIT(.bPublic, FALSE)                                \
    },                                                                \
    {                                                                 \
        VMDIR_SF_INIT(.pszName, VDIR_SYNTAX_UTC_TIME),                \
        VMDIR_SF_INIT(.pszOid, VDIR_OID_UTC_TIME),                    \
        VMDIR_SF_INIT(.readableFlag, VDIR_SYNTAX_HUMAN_READABLE_YES), \
        VMDIR_SF_INIT(.pValidateFunc, syntaxNotImplemented),          \
        VMDIR_SF_INIT(.bPublic, FALSE)                                \
    },                                                                \
    {                                                                 \
        VMDIR_SF_INIT(.pszName, VDIR_SYNTAX_UUID),                    \
        VMDIR_SF_INIT(.pszOid, VDIR_OID_UUID),                        \
        VMDIR_SF_INIT(.readableFlag, VDIR_SYNTAX_HUMAN_READABLE_YES), \
        VMDIR_SF_INIT(.pValidateFunc, syntaxNotImplemented),          \
        VMDIR_SF_INIT(.bPublic, TRUE)                                 \
    },                                                                \
    {                                                                 \
        VMDIR_SF_INIT(.pszName, VDIR_SYNTAX_LARGE_INTEGER),           \
        VMDIR_SF_INIT(.pszOid, VDIR_OID_LARGE_INTEGER),               \
        VMDIR_SF_INIT(.readableFlag, VDIR_SYNTAX_HUMAN_READABLE_YES), \
        VMDIR_SF_INIT(.pValidateFunc, syntaxNotImplemented),          \
        VMDIR_SF_INIT(.bPublic, FALSE)                                \
    },                                                                \
    {                                                                 \
        VMDIR_SF_INIT(.pszName, VDIR_SYNTAX_OBJECT_DN_BINARY),        \
        VMDIR_SF_INIT(.pszOid, VDIR_OID_OBJECT_DN_BINARY),            \
        VMDIR_SF_INIT(.readableFlag, VDIR_SYNTAX_HUMAN_READABLE_NO),  \
        VMDIR_SF_INIT(.pValidateFunc, syntaxNotImplemented),          \
        VMDIR_SF_INIT(.bPublic, FALSE)                                \
    },                                                                \
    {                                                                 \
        VMDIR_SF_INIT(.pszName, VDIR_SYNTAX_OBJECT_DN_STRING),        \
        VMDIR_SF_INIT(.pszOid, VDIR_OID_OBJECT_DN_STRING),            \
        VMDIR_SF_INIT(.readableFlag, VDIR_SYNTAX_HUMAN_READABLE_YES), \
        VMDIR_SF_INIT(.pValidateFunc, syntaxNotImplemented),          \
        VMDIR_SF_INIT(.bPublic, FALSE)                                \
    },                                                                \
    {                                                                 \
        VMDIR_SF_INIT(.pszName, VDIR_SYNTAX_OBJECT_REPLICA_LINK),     \
        VMDIR_SF_INIT(.pszOid, VDIR_OID_OBJECT_REPLICA_LINK),         \
        VMDIR_SF_INIT(.readableFlag, VDIR_SYNTAX_HUMAN_READABLE_YES), \
        VMDIR_SF_INIT(.pValidateFunc, syntaxNotImplemented),          \
        VMDIR_SF_INIT(.bPublic, FALSE)                                \
    },                                                                \
    {                                                                 \
        VMDIR_SF_INIT(.pszName, VDIR_SYNTAX_STRING_NT_SEC_DESC),      \
        VMDIR_SF_INIT(.pszOid, VDIR_OID_STRING_NT_SEC_DESC),          \
        VMDIR_SF_INIT(.readableFlag, VDIR_SYNTAX_HUMAN_READABLE_NO),  \
        VMDIR_SF_INIT(.pValidateFunc, syntaxNotImplemented),          \
        VMDIR_SF_INIT(.bPublic, FALSE)                                \
    },                                                                \
    {                                                                 \
        VMDIR_SF_INIT(.pszName, VDIR_SYNTAX_STRING_TELETEX),          \
        VMDIR_SF_INIT(.pszOid, VDIR_OID_STRING_TELETEX),              \
        VMDIR_SF_INIT(.readableFlag, VDIR_SYNTAX_HUMAN_READABLE_YES), \
        VMDIR_SF_INIT(.pValidateFunc, syntaxNotImplemented),          \
        VMDIR_SF_INIT(.bPublic, FALSE)                                \
    },                                                                \
};

/*  TODO, only support string type matching rule now.
 */

// NOTE: order of fields MUST stay in sync with struct definition...
#define VDIR_EQAULITY_MATCHING_RULE_INIT_TABLE_INITIALIZER                         \
{                                                                                  \
    {                                                                              \
        VMDIR_SF_INIT(.pszSyntaxOid, VDIR_OID_DN),                                 \
        VMDIR_SF_INIT(.pSyntax, NULL),                                             \
        VMDIR_SF_INIT(.pNormalizeFunc, VmDirNormalizeDNWrapper),                   \
        VMDIR_SF_INIT(.pCompareFunc, compareString)                                \
    },                                                                             \
    {                                                                              \
        VMDIR_SF_INIT(.pszSyntaxOid, VDIR_OID_BIT_STRING),                         \
        VMDIR_SF_INIT(.pSyntax, NULL),                                             \
        VMDIR_SF_INIT(.pNormalizeFunc, NULL),                                      \
        VMDIR_SF_INIT(.pCompareFunc, NULL)                                         \
    },                                                                             \
    {                                                                              \
        VMDIR_SF_INIT(.pszSyntaxOid, VDIR_OID_OID),                                \
        VMDIR_SF_INIT(.pSyntax, NULL),                                             \
        VMDIR_SF_INIT(.pNormalizeFunc, normalizeCaseIgnore),                       \
        VMDIR_SF_INIT(.pCompareFunc, compareString)                                \
    },                                                                             \
    {                                                                              \
        VMDIR_SF_INIT(.pszSyntaxOid, VDIR_OID_DIRECTORY_STRING),                   \
        VMDIR_SF_INIT(.pSyntax, NULL),                                             \
        VMDIR_SF_INIT(.pNormalizeFunc, normalizeCaseIgnore),                       \
        VMDIR_SF_INIT(.pCompareFunc, compareString)                                \
    },                                                                             \
    {                                                                              \
        VMDIR_SF_INIT(.pszSyntaxOid, VDIR_OID_IA5_STRING),                         \
        VMDIR_SF_INIT(.pSyntax, NULL),                                             \
        VMDIR_SF_INIT(.pNormalizeFunc, normalizeCaseExact),                        \
        VMDIR_SF_INIT(.pCompareFunc, compareString)                                \
    },                                                                             \
    {                                                                              \
        VMDIR_SF_INIT(.pszSyntaxOid, VDIR_OID_PRINTABLE_STRING),                   \
        VMDIR_SF_INIT(.pSyntax, NULL),                                             \
        VMDIR_SF_INIT(.pNormalizeFunc, normalizeCaseExact),                        \
        VMDIR_SF_INIT(.pCompareFunc, compareString)                                \
    },                                                                             \
    {                                                                              \
        VMDIR_SF_INIT(.pszSyntaxOid, VDIR_OID_NUMERIC_STRING),                     \
        VMDIR_SF_INIT(.pSyntax, NULL),                                             \
        VMDIR_SF_INIT(.pNormalizeFunc, normalizeCaseExact),                        \
        VMDIR_SF_INIT(.pCompareFunc, compareString)                                \
    },                                                                             \
    {                                                                              \
        VMDIR_SF_INIT(.pszSyntaxOid, VDIR_OID_INTERGER),                           \
        VMDIR_SF_INIT(.pSyntax, NULL),                                             \
        VMDIR_SF_INIT(.pNormalizeFunc, NULL),                                      \
        VMDIR_SF_INIT(.pCompareFunc, compareIntegerString)                         \
    },                                                                             \
};

#define VDIR_ORDERING_MATCHING_RULE_INIT_TABLE_INITIALIZER                         \
{                                                                                  \
    {                                                                              \
        VMDIR_SF_INIT(.pszSyntaxOid, VDIR_OID_OID),                                \
        VMDIR_SF_INIT(.pSyntax, NULL),                                             \
        VMDIR_SF_INIT(.pNormalizeFunc, normalizeCaseIgnore),                       \
        VMDIR_SF_INIT(.pCompareFunc, compareString)                                \
    },                                                                             \
    {                                                                              \
        VMDIR_SF_INIT(.pszSyntaxOid, VDIR_OID_DIRECTORY_STRING),                   \
        VMDIR_SF_INIT(.pSyntax, NULL),                                             \
        VMDIR_SF_INIT(.pNormalizeFunc, normalizeCaseIgnore),                       \
        VMDIR_SF_INIT(.pCompareFunc, compareString)                                \
    },                                                                             \
    {                                                                              \
        VMDIR_SF_INIT(.pszSyntaxOid, VDIR_OID_IA5_STRING),                         \
        VMDIR_SF_INIT(.pSyntax, NULL),                                             \
        VMDIR_SF_INIT(.pNormalizeFunc, normalizeCaseExact),                        \
        VMDIR_SF_INIT(.pCompareFunc, compareString)                                \
    },                                                                             \
    {                                                                              \
        VMDIR_SF_INIT(.pszSyntaxOid, VDIR_OID_PRINTABLE_STRING),                   \
        VMDIR_SF_INIT(.pSyntax, NULL),                                             \
        VMDIR_SF_INIT(.pNormalizeFunc, normalizeCaseExact),                        \
        VMDIR_SF_INIT(.pCompareFunc, compareString)                                \
    },                                                                             \
    {                                                                              \
        VMDIR_SF_INIT(.pszSyntaxOid, VDIR_OID_NUMERIC_STRING),                     \
        VMDIR_SF_INIT(.pSyntax, NULL),                                             \
        VMDIR_SF_INIT(.pNormalizeFunc, normalizeCaseExact),                        \
        VMDIR_SF_INIT(.pCompareFunc, compareString)                                \
    },                                                                             \
    {                                                                              \
        VMDIR_SF_INIT(.pszSyntaxOid, VDIR_OID_INTERGER),                           \
        VMDIR_SF_INIT(.pSyntax, NULL),                                             \
        VMDIR_SF_INIT(.pNormalizeFunc, NULL),                                      \
        VMDIR_SF_INIT(.pCompareFunc, compareIntegerString)                         \
    },                                                                             \
};

#define VDIR_SUBSTR_MATCHING_RULE_INIT_TABLE_INITIALIZER                           \
{                                                                                  \
    {                                                                              \
        VMDIR_SF_INIT(.pszSyntaxOid, VDIR_OID_OID),                                \
        VMDIR_SF_INIT(.pSyntax, NULL),                                             \
        VMDIR_SF_INIT(.pNormalizeFunc, normalizeCaseIgnore),                       \
        VMDIR_SF_INIT(.pCompareFunc, compareSubString)                             \
    },                                                                             \
    {                                                                              \
        VMDIR_SF_INIT(.pszSyntaxOid, VDIR_OID_DIRECTORY_STRING),                   \
        VMDIR_SF_INIT(.pSyntax, NULL),                                             \
        VMDIR_SF_INIT(.pNormalizeFunc, normalizeCaseIgnore),                       \
        VMDIR_SF_INIT(.pCompareFunc, compareSubString)                             \
    },                                                                             \
    {                                                                              \
        VMDIR_SF_INIT(.pszSyntaxOid, VDIR_OID_IA5_STRING),                         \
        VMDIR_SF_INIT(.pSyntax, NULL),                                             \
        VMDIR_SF_INIT(.pNormalizeFunc, normalizeCaseExact),                        \
        VMDIR_SF_INIT(.pCompareFunc, compareSubString)                             \
    },                                                                             \
    {                                                                              \
        VMDIR_SF_INIT(.pszSyntaxOid, VDIR_OID_PRINTABLE_STRING),                   \
        VMDIR_SF_INIT(.pSyntax, NULL),                                             \
        VMDIR_SF_INIT(.pNormalizeFunc, normalizeCaseExact),                        \
        VMDIR_SF_INIT(.pCompareFunc, compareSubString)                             \
    },                                                                             \
    {                                                                              \
        VMDIR_SF_INIT(.pszSyntaxOid, VDIR_OID_NUMERIC_STRING),                     \
        VMDIR_SF_INIT(.pSyntax, NULL),                                             \
        VMDIR_SF_INIT(.pNormalizeFunc, normalizeCaseExact),                        \
        VMDIR_SF_INIT(.pCompareFunc, compareSubString)                             \
    },                                                                             \
};

// 0 is reserved for NO Id case
// 1~99 is reserved for bootstrap attrs
// auto generated Id starts from 100
#define NO_ATTR_ID_MAP               0
#define MAX_RESERVED_ATTR_ID_MAP    99

// NOTE: order of fields MUST stay in sync with struct definition...
#define VDIR_SCHEMA_BOOTSTRP_ATTR_INITIALIZER                           \
{                                                                       \
    {                                                                   \
    VMDIR_SF_INIT(.usAttrID, 1),                                        \
    VMDIR_SF_INIT(.pszDesc, "attributeTypes: ("                         \
            " 2.5.21.6"                                                 \
            " NAME 'objectClasses'"                                     \
            " SYNTAX 1.3.6.1.4.1.1466.115.121.1.15"                     \
            " USAGE directoryOperation )")                              \
    },                                                                  \
    {                                                                   \
    VMDIR_SF_INIT(.usAttrID, 2),                                        \
    VMDIR_SF_INIT(.pszDesc, "attributeTypes: ("                         \
            " 2.5.21.5 "                                                \
            " NAME 'attributeTypes' "                                   \
            " SYNTAX 1.3.6.1.4.1.1466.115.121.1.15"                     \
            " USAGE directoryOperation )")                              \
    },                                                                  \
    {                                                                   \
    VMDIR_SF_INIT(.usAttrID, 3),                                        \
    VMDIR_SF_INIT(.pszDesc, "attributeTypes: ("                         \
            " 1.3.6.1.4.1.1466.101.120.16"                              \
            " NAME 'ldapSyntaxes'"                                      \
            " SYNTAX 1.3.6.1.4.1.1466.115.121.1.5"                      \
            " USAGE directoryOperation )")                              \
    },                                                                  \
    {                                                                   \
    VMDIR_SF_INIT(.usAttrID, 4),                                        \
    VMDIR_SF_INIT(.pszDesc, "attributeTypes: ("                         \
            " 2.5.21.8"                                                 \
            " NAME 'matchingRuleUse'"                                   \
            " SYNTAX 1.3.6.1.4.1.1466.115.121.1.5"                      \
            " USAGE directoryOperation )")                              \
    },                                                                  \
    {                                                                   \
    VMDIR_SF_INIT(.usAttrID, 5),                                        \
    VMDIR_SF_INIT(.pszDesc, "attributeTypes: ("                         \
            " 2.5.21.4"                                                 \
            " NAME 'matchingRules'"                                     \
            " SYNTAX 1.3.6.1.4.1.1466.115.121.1.5"                      \
            " USAGE directoryOperation )")                              \
    },                                                                  \
    {                                                                   \
    VMDIR_SF_INIT(.usAttrID, 6),                                        \
    VMDIR_SF_INIT(.pszDesc, "attributeTypes: ("                         \
            " 2.5.18.10"                                                \
            " NAME 'subSchemaSubentry'"                                 \
            " SYNTAX 1.3.6.1.4.1.1466.115.121.1.12"                     \
            " NO-USER-MODIFICATION )")                                  \
    },                                                                  \
    {                                                                   \
    VMDIR_SF_INIT(.usAttrID, 7),                                        \
    VMDIR_SF_INIT(.pszDesc, "attributeTypes: ("                         \
            " 2.5.21.9"                                                 \
            " NAME 'structuralObjectClass'"                             \
            " SYNTAX 1.3.6.1.4.1.1466.115.121.1.38 )")                  \
    },                                                                  \
    {                                                                   \
    VMDIR_SF_INIT(.usAttrID, 8),                                        \
    VMDIR_SF_INIT(.pszDesc, "attributeTypes: ("                         \
            " 1.3.6.1.1.20"                                             \
            " NAME 'entryDN'"                                           \
            " SYNTAX 1.3.6.1.4.1.1466.115.121.1.12"                     \
            " SINGLE-VALUE"                                             \
            " NO-USER-MODIFICATION"                                     \
            " USAGE directoryOperation )")                              \
    },                                                                  \
    {                                                                   \
    VMDIR_SF_INIT(.usAttrID, 9),                                        \
    VMDIR_SF_INIT(.pszDesc, "attributeTypes: ("                         \
            " 2.5.4.0"                                                  \
            " NAME 'objectClass'"                                       \
            " SYNTAX 1.3.6.1.4.1.1466.115.121.1.38 )")                  \
    },                                                                  \
    {                                                                   \
    VMDIR_SF_INIT(.usAttrID, 10),                                       \
    VMDIR_SF_INIT(.pszDesc, "attributeTypes: ("                         \
            " 2.5.21.1"                                                 \
            " NAME 'dITStructureRules'"                                 \
            " SYNTAX 1.3.6.1.4.1.1466.115.121.1.17"                     \
            " USAGE directoryOperation )")                              \
    },                                                                  \
    {                                                                   \
    VMDIR_SF_INIT(.usAttrID, 11),                                       \
    VMDIR_SF_INIT(.pszDesc, "attributeTypes: ("                         \
            " 2.5.21.2"                                                 \
            " NAME 'dITContentRules'"                                   \
            " SYNTAX 1.3.6.1.4.1.1466.115.121.1.15"                     \
            " USAGE directoryOperation )")                              \
    },                                                                  \
    {                                                                   \
    VMDIR_SF_INIT(.usAttrID, 12),                                       \
    VMDIR_SF_INIT(.pszDesc, "attributeTypes: ("                         \
            " 2.5.21.7"                                                 \
            " NAME 'nameForms'"                                         \
            " SYNTAX 1.3.6.1.4.1.1466.115.121.1.35"                     \
            " USAGE directoryOperation )")                              \
    },                                                                  \
    {                                                                   \
    VMDIR_SF_INIT(.usAttrID, 13),                                       \
    VMDIR_SF_INIT(.pszDesc, "attributeTypes: ("                         \
            " 2.5.18.3"                                                 \
            " NAME 'creatorsName'"                                      \
            " SYNTAX 1.3.6.1.4.1.1466.115.121.1.12"                     \
            " SINGLE-VALUE"                                             \
            " NO-USER-MODIFICATION"                                     \
            " USAGE directoryOperation )")                              \
    },                                                                  \
    {                                                                   \
    VMDIR_SF_INIT(.usAttrID, 14),                                       \
    VMDIR_SF_INIT(.pszDesc, "attributeTypes: ("                         \
            " 2.5.18.1"                                                 \
            " NAME 'createTimeStamp'"                                   \
            " SYNTAX 1.3.6.1.4.1.1466.115.121.1.24"                     \
            " SINGLE-VALUE"                                             \
            " NO-USER-MODIFICATION"                                     \
            " USAGE directoryOperation )")                              \
    },                                                                  \
    {                                                                   \
    VMDIR_SF_INIT(.usAttrID, 15),                                       \
    VMDIR_SF_INIT(.pszDesc, "attributeTypes: ("                         \
            " 2.5.18.2"                                                 \
            " NAME 'modifyTimeStamp'"                                   \
            " SYNTAX 1.3.6.1.4.1.1466.115.121.1.24"                     \
            " SINGLE-VALUE"                                             \
            " NO-USER-MODIFICATION"                                     \
            " USAGE directoryOperation )")                              \
    },                                                                  \
    {                                                                   \
    VMDIR_SF_INIT(.usAttrID, 16),                                       \
    VMDIR_SF_INIT(.pszDesc, "attributeTypes: ("                         \
            " VMWare.DIR.attribute.0.1"                                 \
            " NAME 'vmwAttributeToIdMap'"                               \
            " SYNTAX 1.3.6.1.4.1.1466.115.121.1.15"                     \
            " SINGLE-VALUE"                                             \
            " NO-USER-MODIFICATION"                                     \
            " USAGE directoryOperation )")                              \
    },                                                                  \
    {                                                                   \
    VMDIR_SF_INIT(.usAttrID, 17),                                       \
    VMDIR_SF_INIT(.pszDesc, "attributeTypes: ("                         \
            " 2.5.4.3"                                                  \
            " NAME 'cn'"                                                \
            " SYNTAX 1.3.6.1.4.1.1466.115.121.1.15"                     \
            " SINGLE-VALUE )")                                          \
    },                                                                  \
    {                                                                   \
    VMDIR_SF_INIT(.usAttrID, 18),                                       \
    VMDIR_SF_INIT(.pszDesc, "attributeTypes: ("                         \
            " VMWare.DIR.attribute.1.1"                                 \
            " NAME 'uSNCreated'"                                        \
            " SYNTAX 1.3.6.1.4.1.1466.115.121.1.27"                     \
            " SINGLE-VALUE"                                             \
            " NO-USER-MODIFICATION"                                     \
            " USAGE directoryOperation )")                              \
    },                                                                  \
    {                                                                   \
    VMDIR_SF_INIT(.usAttrID, 19),                                       \
    VMDIR_SF_INIT(.pszDesc, "attributeTypes: ("                         \
            " VMWare.DIR.attribute.1.2"                                 \
            " NAME 'uSNChanged'"                                        \
            " SYNTAX 1.3.6.1.4.1.1466.115.121.1.27"                     \
            " SINGLE-VALUE"                                             \
            " NO-USER-MODIFICATION"                                     \
            " USAGE directoryOperation )")                              \
    },                                                                  \
    {                                                                   \
    VMDIR_SF_INIT(.usAttrID, 20),                                       \
    VMDIR_SF_INIT(.pszDesc, "attributeTypes: ("                         \
            " 999.999.0.9"                                              \
            " NAME 'dn'"                                                \
            " SYNTAX 1.3.6.1.4.1.1466.115.121.1.12 )")                  \
    },                                                                  \
    {                                                                   \
    VMDIR_SF_INIT(.usAttrID, 21),                                       \
    VMDIR_SF_INIT(.pszDesc, "attributeTypes: ("                         \
            " VMWare.DIR.attribute.0.20"                                \
            " NAME 'vmwSecurityDescriptor'"                             \
            " SYNTAX 1.3.6.1.4.1.1466.115.121.1.40 )")                  \
    },                                                                  \
    {                                                                   \
    VMDIR_SF_INIT(.usAttrID, SCHEMA_BOOTSTRAP_EID_SEQ_ATTRID_22),       \
    VMDIR_SF_INIT(.pszDesc, "attributeTypes: ("                         \
            " VMWare.DIR.attribute.0.23"                                \
            " NAME 'vmwEntryIdSequenceNumber'"                          \
            " DESC 'ID NEEDS TO BE 22'"                                 \
            " SYNTAX 1.3.6.1.4.1.1466.115.121.1.27"                     \
            " SINGLE-VALUE"                                             \
            " NO-USER-MODIFICATION"                                     \
            " USAGE directoryOperation )")                              \
    },                                                                  \
    {                                                                   \
    VMDIR_SF_INIT(.usAttrID, SCHEMA_BOOTSTRAP_USN_SEQ_ATTRID_23),       \
    VMDIR_SF_INIT(.pszDesc, "attributeTypes: ("                         \
            " VMWare.DIR.attribute.0.24"                                \
            " NAME 'vmwUSNSequenceNumber'"                              \
            " DESC 'ID NEEDS TO BE 23'"                                 \
            " SYNTAX 1.3.6.1.4.1.1466.115.121.1.27"                     \
            " SINGLE-VALUE"                                             \
            " NO-USER-MODIFICATION"                                     \
            " USAGE directoryOperation )")                              \
    },                                                                  \
    {                                                                   \
    VMDIR_SF_INIT(.usAttrID, 24),                                       \
    VMDIR_SF_INIT(.pszDesc, "attributeTypes: ("                         \
            " VMWare.DIR.attribute.1.4"                                 \
            " NAME 'objectGUID'"                                        \
            " DESC 'ID NEEDS TO BE 24'"                                 \
            " SYNTAX 1.3.6.1.4.1.1466.115.121.1.15"                     \
            " SINGLE-VALUE"                                             \
            " NO-USER-MODIFICATION"                                     \
            " USAGE directoryOperation )")                              \
    },                                                                  \
    {                                                                   \
    VMDIR_SF_INIT(.usAttrID, 25),                                       \
    VMDIR_SF_INIT(.pszDesc, "attributeTypes: ("                         \
            " 2.5.18.4"                                                 \
            " NAME 'modifiersName'"                                     \
            " SYNTAX 1.3.6.1.4.1.1466.115.121.1.12"                     \
            " SINGLE-VALUE"                                             \
            " NO-USER-MODIFICATION"                                     \
            " USAGE directoryOperation )")                              \
    },                                                                  \
    {                                                                   \
    VMDIR_SF_INIT(.usAttrID, 26),                                       \
    VMDIR_SF_INIT(.pszDesc, "attributeTypes: ("                         \
            " 1.2.840.113556.1.2.33"                                    \
            " NAME 'isSingleValued'"                                    \
            " SYNTAX 1.3.6.1.4.1.1466.115.121.1.7"                      \
            " SINGLE-VALUE"                                             \
            " NO-USER-MODIFICATION )")                                  \
    },                                                                  \
    {                                                                   \
    VMDIR_SF_INIT(.usAttrID, 27),                                       \
    VMDIR_SF_INIT(.pszDesc, "attributeTypes: ("                         \
            " 1.2.840.113556.1.2.32"                                    \
            " NAME 'attributeSyntax'"                                   \
            " SYNTAX 1.3.6.1.4.1.1466.115.121.1.38"                     \
            " SINGLE-VALUE"                                             \
            " NO-USER-MODIFICATION )")                                  \
    },                                                                  \
    {                                                                   \
    VMDIR_SF_INIT(.usAttrID, 28),                                       \
    VMDIR_SF_INIT(.pszDesc, "attributeTypes: ("                         \
            " 1.2.840.113556.1.2.460"                                   \
            " NAME 'lDAPDisplayName'"                                   \
            " SYNTAX 1.3.6.1.4.1.1466.115.121.1.15"                     \
            " SINGLE-VALUE )")                                          \
    },                                                                  \
    {                                                                   \
    VMDIR_SF_INIT(.usAttrID, 29),                                       \
    VMDIR_SF_INIT(.pszDesc, "attributeTypes: ("                         \
            " 1.2.840.113556.1.2.30"                                    \
            " NAME 'attributeID'"                                       \
            " SYNTAX 1.3.6.1.4.1.1466.115.121.1.38"                     \
            " SINGLE-VALUE"                                             \
            " NO-USER-MODIFICATION )")                                  \
    },                                                                  \
    {                                                                   \
    VMDIR_SF_INIT(.usAttrID, 30),                                       \
    VMDIR_SF_INIT(.pszDesc, "attributeTypes: ("                         \
            " 1.2.840.113556.1.2.231"                                   \
            " NAME 'oMSyntax'"                                          \
            " SYNTAX 1.3.6.1.4.1.1466.115.121.1.27"                     \
            " SINGLE-VALUE"                                             \
            " NO-USER-MODIFICATION )")                                  \
    },                                                                  \
    {                                                                   \
    VMDIR_SF_INIT(.usAttrID, 31),                                       \
    VMDIR_SF_INIT(.pszDesc, "attributeTypes: ("                         \
            " 1.2.840.113556.1.4.148"                                   \
            " NAME 'schemaIDGUID'"                                      \
            " SYNTAX 1.3.6.1.4.1.1466.115.121.1.40"                     \
            " SINGLE-VALUE"                                             \
            " NO-USER-MODIFICATION )")                                  \
    },                                                                  \
    {                                                                   \
    VMDIR_SF_INIT(.usAttrID, 32),                                       \
    VMDIR_SF_INIT(.pszDesc, "attributeTypes: ("                         \
            " 1.2.840.113556.1.2.21"                                    \
            " NAME 'subClassOf'"                                        \
            " SYNTAX 1.3.6.1.4.1.1466.115.121.1.38"                     \
            " SINGLE-VALUE"                                             \
            " NO-USER-MODIFICATION )")                                  \
    },                                                                  \
    {                                                                   \
    VMDIR_SF_INIT(.usAttrID, 33),                                       \
    VMDIR_SF_INIT(.pszDesc, "attributeTypes: ("                         \
            " 1.2.840.113556.1.2.22"                                    \
            " NAME 'governsID'"                                         \
            " SYNTAX 1.3.6.1.4.1.1466.115.121.1.38"                     \
            " SINGLE-VALUE"                                             \
            " NO-USER-MODIFICATION )")                                  \
    },                                                                  \
    {                                                                   \
    VMDIR_SF_INIT(.usAttrID, 34),                                       \
    VMDIR_SF_INIT(.pszDesc, "attributeTypes: ("                         \
            " 1.2.840.113556.1.2.370"                                   \
            " NAME 'objectClassCategory'"                               \
            " SYNTAX 1.3.6.1.4.1.1466.115.121.1.27"                     \
            " SINGLE-VALUE"                                             \
            " NO-USER-MODIFICATION )")                                  \
    },                                                                  \
    {                                                                   \
    VMDIR_SF_INIT(.usAttrID, 35),                                       \
    VMDIR_SF_INIT(.pszDesc, "attributeTypes: ("                         \
            " 1.2.840.113556.1.4.783"                                   \
            " NAME 'defaultObjectCategory'"                             \
            " SYNTAX 1.3.6.1.4.1.1466.115.121.1.12"                     \
            " SINGLE-VALUE )")                                          \
    },                                                                  \
    {                                                                   \
    VMDIR_SF_INIT(.usAttrID, 36),                                       \
    VMDIR_SF_INIT(.pszDesc, "attributeTypes: ("                         \
            " 1.2.840.113556.1.4.197"                                   \
            " NAME 'systemMustContain'"                                 \
            " SYNTAX 1.3.6.1.4.1.1466.115.121.1.38"                     \
            " NO-USER-MODIFICATION )")                                  \
    },                                                                  \
    {                                                                   \
    VMDIR_SF_INIT(.usAttrID, 37),                                       \
    VMDIR_SF_INIT(.pszDesc, "attributeTypes: ("                         \
            " 1.2.840.113556.1.4.196"                                   \
            " NAME 'systemMayContain'"                                  \
            " SYNTAX 1.3.6.1.4.1.1466.115.121.1.38"                     \
            " NO-USER-MODIFICATION )")                                  \
    },                                                                  \
    {                                                                   \
    VMDIR_SF_INIT(.usAttrID, 38),                                       \
    VMDIR_SF_INIT(.pszDesc, "attributeTypes: ("                         \
            " 1.2.840.113556.1.4.198"                                   \
            " NAME 'systemAuxiliaryClass'"                              \
            " SYNTAX 1.3.6.1.4.1.1466.115.121.1.38"                     \
            " NO-USER-MODIFICATION )")                                  \
    },                                                                  \
    {                                                                   \
    VMDIR_SF_INIT(.usAttrID, 39),                                       \
    VMDIR_SF_INIT(.pszDesc, "attributeTypes: ("                         \
            " 1.2.840.113556.1.2.24"                                    \
            " NAME 'mustContain'"                                       \
            " SYNTAX 1.3.6.1.4.1.1466.115.121.1.38 )")                  \
    },                                                                  \
    {                                                                   \
    VMDIR_SF_INIT(.usAttrID, 40),                                       \
    VMDIR_SF_INIT(.pszDesc, "attributeTypes: ("                         \
            " 1.2.840.113556.1.2.25"                                    \
            " NAME 'mayContain'"                                        \
            " SYNTAX 1.3.6.1.4.1.1466.115.121.1.38 )")                  \
    },                                                                  \
    {                                                                   \
    VMDIR_SF_INIT(.usAttrID, 41),                                       \
    VMDIR_SF_INIT(.pszDesc, "attributeTypes: ("                         \
            " 1.2.840.113556.1.2.351"                                   \
            " NAME 'auxiliaryClass'"                                    \
            " SYNTAX 1.3.6.1.4.1.1466.115.121.1.38 )")                  \
    },                                                                  \
    {                                                                   \
    VMDIR_SF_INIT(.usAttrID, 42),                                       \
    VMDIR_SF_INIT(.pszDesc, "attributeTypes: ("                         \
            " 1.2.840.113556.1.4.661"                                   \
            " NAME 'isDefunct'"                                         \
            " SYNTAX 1.3.6.1.4.1.1466.115.121.1.7"                      \
            " SINGLE-VALUE )")                                          \
    },                                                                  \
    {                                                                   \
    VMDIR_SF_INIT(.usAttrID, 43),                                       \
    VMDIR_SF_INIT(.pszDesc, "attributeTypes: ("                         \
            " VMWare.DIR.attribute.0.58"                                \
            " NAME 'vmwAttributeUsage'"                                 \
            " SYNTAX 1.3.6.1.4.1.1466.115.121.1.27"                     \
            " SINGLE-VALUE"                                             \
            " NO-USER-MODIFICATION )")                                  \
    },                                                                  \
    {                                                                   \
    VMDIR_SF_INIT(.usAttrID, 44),                                       \
    VMDIR_SF_INIT(.pszDesc, "attributeTypes: ("                         \
            " 2.5.4.13"                                                 \
            " NAME 'description'"                                       \
            " SYNTAX 1.3.6.1.4.1.1466.115.121.1.15 )")                  \
    },                                                                  \
    {                                                                   \
    VMDIR_SF_INIT(.usAttrID, 45),                                       \
    VMDIR_SF_INIT(.pszDesc, "attributeTypes: ("                         \
            " 1.2.840.113556.1.2.334"                                   \
            " NAME 'searchFlags'"                                       \
            " SYNTAX 1.3.6.1.4.1.1466.115.121.1.27"                     \
            " SINGLE-VALUE )")                                          \
    },                                                                  \
    {                                                                   \
    VMDIR_SF_INIT(.usAttrID, 46),                                       \
    VMDIR_SF_INIT(.pszDesc, "attributeTypes: ("                         \
            " VMWare.DIR.attribute.1.7"                                 \
            " NAME 'vmwAttrUniquenessScope'"                            \
            " SYNTAX 1.3.6.1.4.1.1466.115.121.1.12 )")                  \
    },                                                                  \
    {                                                                   \
    VMDIR_SF_INIT(.usAttrID, 47),                                       \
    VMDIR_SF_INIT(.pszDesc, "attributeTypes: ("                         \
            " 1.2.840.113556.1.2.281"                                   \
            " NAME 'nTSecurityDescriptor'"                              \
            " SYNTAX 1.2.840.113556.1.4.907"                            \
            " SINGLE-VALUE )")                                          \
    },                                                                  \
    {                                                                   \
    VMDIR_SF_INIT(.usAttrID, 0),                                        \
    VMDIR_SF_INIT(.pszDesc, NULL)                                       \
    },                                                                  \
}
