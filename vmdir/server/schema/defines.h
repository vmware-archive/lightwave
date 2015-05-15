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

#define SCHEMA_SIZE_5  5

#define ATTRIBUTETYPS_TAG           "attributetypes:"
#define ATTRIBUTETYPS_TAG_LEN       sizeof(ATTRIBUTETYPS_TAG)-1
#define IS_ATTRIBUTETYPES_TAG(tag)    \
    (VmDirStringNCompareA(tag, ATTRIBUTETYPS_TAG,  ATTRIBUTETYPS_TAG_LEN, FALSE) == 0)

#define OBJECTCLASSES_TAG           "objectclasses:"
#define OBJECTCLASSES_TAG_LEN       sizeof(OBJECTCLASSES_TAG)-1
#define IS_OBJECTCLASSES_TAG(tag)    \
    (VmDirStringNCompareA(tag, OBJECTCLASSES_TAG, OBJECTCLASSES_TAG_LEN, FALSE) == 0)

#define CONTENTRULES_TAG           "ditcontentrules:"
#define CONTENTRULES_TAG_LEN       sizeof(CONTENTRULES_TAG)-1
#define IS_CONTENTRULES_TAG(tag)    \
    (VmDirStringNCompareA(tag, CONTENTRULES_TAG, CONTENTRULES_TAG_LEN, FALSE) == 0)

#define STRUCTURERULES_TAG           "ditstructurerules:"
#define STRUCTURERULES_TAG_LEN       sizeof(STRUCTURERULES_TAG)-1
#define IS_STRUCTURERULES_TAG(tag)    \
    (VmDirStringNCompareA(tag, STRUCTURERULES_TAG, STRUCTURERULES_TAG_LEN, FALSE) == 0)

#define NAMEFORM_TAG                "nameforms:"
#define NAMEFORM_TAG_LEN            sizeof(NAMEFORM_TAG)-1
#define IS_NAMEFORM_TAG(tag)    \
    (VmDirStringNCompareA(tag, NAMEFORM_TAG, NAMEFORM_TAG_LEN, FALSE) == 0)

#define ATTRIBUTETOIDMAP_TAG        "vmwAttributeToIdMap:"
#define ATTRIBUTETOIDMAP_TAG_LEN    sizeof(ATTRIBUTETOIDMAP_TAG)-1
#define IS_ATTRIBUTETOIDMAP_TAG(tag)    \
    (VmDirStringNCompareA(tag, ATTRIBUTETOIDMAP_TAG, ATTRIBUTETOIDMAP_TAG_LEN, FALSE) == 0)

#define SCHEMA_GEN_TOKEN_SEP                " '$"
#define SCHEMA_DESC_TOKEN_SEP               "'"
#define SCHEMA_ATTRIBUTE_ID_MAP_TOKEN_SEP   ":"
#define SCHEMA_ATTRIBUTE_ID_MAP_SEP_CHAR    '='

#define LEFT_PARENTHESIS                    "("
#define RIGHT_PARENTHESIS                   ")"

#define IS_EMPTY_PARENTHESIS_STR(pToken) \
    (pToken[0] == '(' && pToken[1] == ')' && pToken[2] == '\0')

#define IS_L_PARENTHESIS(pToken) \
    (pToken[0] == '(' )

#define IS_R_PARENTHESIS(pToken) \
    (pToken[0] == ')' )

#define IS_L_PARENTHESIS_STR(pToken) \
    (pToken[0] == '(' && pToken[1] == '\0')

#define IS_R_PARENTHESIS_STR(pToken) \
    (pToken[0] == ')' && pToken[1] == '\0')

#define GET_NEXT_TOKEN(pToken, pSep, pRest) \
    (pToken) = VmDirStringTokA(NULL, (pSep), &(pRest));     \
    if (!(pToken))                                          \
    {                                                       \
        dwError = ERROR_INVALID_PARAMETER;                  \
        BAIL_ON_VMDIR_ERROR(dwError);                       \
    }

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

#define VDIR_SYNTAX_ATTRIBUTE_TYPE_DESC   "Attribute Type Description"
#define VDIR_SYNTAX_BINARY                "Binary"
#define VDIR_SYNTAX_BIT_STRING            "Bit String"
#define VDIR_SYNTAX_BOOLEAN               "Boolean"
#define VDIR_SYNTAX_DN                    "DN"
#define VDIR_SYNTAX_DIRECTORY_STRING      "Directory String"
#define VDIR_SYNTAX_FAX_NUMBER            "Facsimile Telephone Number"
#define VDIR_SYNTAX_GEN_TIME              "Generalized Time"
#define VDIR_SYNTAX_IA5_STRING            "IA5 String"
#define VDIR_SYNTAX_INTERGER              "INTEGER"
#define VDIR_SYNTAX_NUMERIC_STRING        "Numeric String"
#define VDIR_SYNTAX_OBJECTCLASS_DESC      "bject Class Description"
#define VDIR_SYNTAX_OCTET_STRING          "Octet String"
#define VDIR_SYNTAX_OID                   "OID"
#define VDIR_SYNTAX_POSTAL_ADDRESS        "Postal Address"
#define VDIR_SYNTAX_PRINTABLE_STRING      "Printable String"
#define VDIR_SYNTAX_TELEPHONE_NUMBER      "Telephone Number"
#define VDIR_SYNTAX_UTC_TIME              "UCT Time"

#define VDIR_OID_ATTRIBUTE_TYPE_DESC    "1.3.6.1.4.1.1466.115.121.1.3"
#define VDIR_OID_BINARY                 "1.3.6.1.4.1.1466.115.121.1.5"
#define VDIR_OID_BIT_STRING             "1.3.6.1.4.1.1466.115.121.1.6"
#define VDIR_OID_BOOLEAN                "1.3.6.1.4.1.1466.115.121.1.7"
#define VDIR_OID_DN                     "1.3.6.1.4.1.1466.115.121.1.12"
#define VDIR_OID_DIRECTORY_STRING       "1.3.6.1.4.1.1466.115.121.1.15"
#define VDIR_OID_FAX_NUMBER             "1.3.6.1.4.1.1466.115.121.1.22"
#define VDIR_OID_GEN_TIME               "1.3.6.1.4.1.1466.115.121.1.24"
#define VDIR_OID_IA5_STRING             "1.3.6.1.4.1.1466.115.121.1.26"
#define VDIR_OID_INTERGER               "1.3.6.1.4.1.1466.115.121.1.27"
#define VDIR_OID_NUMERIC_STRING         "1.3.6.1.4.1.1466.115.121.1.36"
#define VDIR_OID_OBJECTCLASS_DESC       "1.3.6.1.4.1.1466.115.121.1.37"
#define VDIR_OID_OCTET_STRING           "1.3.6.1.4.1.1466.115.121.1.40"
#define VDIR_OID_OID                    "1.3.6.1.4.1.1466.115.121.1.38"
#define VDIR_OID_POSTAL_ADDRESS         "1.3.6.1.4.1.1466.115.121.1.41"
#define VDIR_OID_PRINTABLE_STRING       "1.3.6.1.4.1.1466.115.121.1.44"
#define VDIR_OID_TELEPHONE_NUMBER       "1.3.6.1.4.1.1466.115.121.1.50"
#define VDIR_OID_UTC_TIME               "1.3.6.1.4.1.1466.115.121.1.53"

// NOTE: order of fields MUST stay in sync with struct definition...
#define VDIR_SYNTAX_INIT_TABLE_INITIALIZER                            \
{                                                                     \
    {                                                                 \
        VMDIR_SF_INIT(.pszName, VDIR_SYNTAX_ATTRIBUTE_TYPE_DESC),     \
        VMDIR_SF_INIT(.pszOid, VDIR_OID_ATTRIBUTE_TYPE_DESC),         \
        VMDIR_SF_INIT(.readableFlag, VDIR_SYNTAX_HUMAN_READABLE_YES), \
        VMDIR_SF_INIT(.pValidateFunc, syntaxAttrTypeDesc)             \
    },                                                                \
   {                                                                  \
        VMDIR_SF_INIT(.pszName, VDIR_SYNTAX_BINARY),                  \
        VMDIR_SF_INIT(.pszOid, VDIR_OID_BINARY),                      \
        VMDIR_SF_INIT(.readableFlag, VDIR_SYNTAX_HUMAN_READABLE_NO),  \
        VMDIR_SF_INIT(.pValidateFunc, syntaxBinary)                   \
    },                                                                \
    {                                                                 \
        VMDIR_SF_INIT(.pszName, VDIR_SYNTAX_BIT_STRING),              \
        VMDIR_SF_INIT(.pszOid, VDIR_OID_BIT_STRING),                  \
        VMDIR_SF_INIT(.readableFlag, VDIR_SYNTAX_HUMAN_READABLE_NO),  \
        VMDIR_SF_INIT(.pValidateFunc, syntaxBitString)                \
    },                                                                \
    {                                                                 \
        VMDIR_SF_INIT(.pszName, VDIR_SYNTAX_BOOLEAN),                 \
        VMDIR_SF_INIT(.pszOid, VDIR_OID_BOOLEAN),                     \
        VMDIR_SF_INIT(.readableFlag, VDIR_SYNTAX_HUMAN_READABLE_YES), \
        VMDIR_SF_INIT(.pValidateFunc, syntaxBoolean)                  \
    },                                                                \
    {                                                                 \
        VMDIR_SF_INIT(.pszName, VDIR_SYNTAX_DN),                      \
        VMDIR_SF_INIT(.pszOid, VDIR_OID_DN),                          \
        VMDIR_SF_INIT(.readableFlag, VDIR_SYNTAX_HUMAN_READABLE_YES), \
        VMDIR_SF_INIT(.pValidateFunc, syntaxDN)                       \
    },                                                                \
    {                                                                 \
        VMDIR_SF_INIT(.pszName, VDIR_SYNTAX_DIRECTORY_STRING),        \
        VMDIR_SF_INIT(.pszOid, VDIR_OID_DIRECTORY_STRING),            \
        VMDIR_SF_INIT(.readableFlag, VDIR_SYNTAX_HUMAN_READABLE_YES), \
        VMDIR_SF_INIT(.pValidateFunc, syntaxDirectoryString)          \
    },                                                                \
    {                                                                 \
        VMDIR_SF_INIT(.pszName, VDIR_SYNTAX_FAX_NUMBER),              \
        VMDIR_SF_INIT(.pszOid, VDIR_OID_FAX_NUMBER),                  \
        VMDIR_SF_INIT(.readableFlag, VDIR_SYNTAX_HUMAN_READABLE_YES), \
        VMDIR_SF_INIT(.pValidateFunc, syntaxFaxNumber)                \
    },                                                                \
    {                                                                 \
        VMDIR_SF_INIT(.pszName, VDIR_SYNTAX_GEN_TIME),                \
        VMDIR_SF_INIT(.pszOid, VDIR_OID_GEN_TIME),                    \
        VMDIR_SF_INIT(.readableFlag, VDIR_SYNTAX_HUMAN_READABLE_YES), \
        VMDIR_SF_INIT(.pValidateFunc, syntaxGeneralizedTime)          \
    },                                                                \
    {                                                                 \
        VMDIR_SF_INIT(.pszName, VDIR_SYNTAX_IA5_STRING),              \
        VMDIR_SF_INIT(.pszOid, VDIR_OID_IA5_STRING),                  \
        VMDIR_SF_INIT(.readableFlag, VDIR_SYNTAX_HUMAN_READABLE_YES), \
        VMDIR_SF_INIT(.pValidateFunc, syntaxIA5String)                \
    },                                                                \
    {                                                                 \
        VMDIR_SF_INIT(.pszName, VDIR_SYNTAX_INTERGER),                \
        VMDIR_SF_INIT(.pszOid, VDIR_OID_INTERGER),                    \
        VMDIR_SF_INIT(.readableFlag, VDIR_SYNTAX_HUMAN_READABLE_YES), \
        VMDIR_SF_INIT(.pValidateFunc, syntaxInteger)                  \
    },                                                                \
    {                                                                 \
        VMDIR_SF_INIT(.pszName, VDIR_SYNTAX_NUMERIC_STRING),          \
        VMDIR_SF_INIT(.pszOid, VDIR_OID_NUMERIC_STRING),              \
        VMDIR_SF_INIT(.readableFlag, VDIR_SYNTAX_HUMAN_READABLE_YES), \
        VMDIR_SF_INIT(.pValidateFunc, syntaxNumericString)            \
    },                                                                \
    {                                                                 \
        VMDIR_SF_INIT(.pszName, VDIR_SYNTAX_OCTET_STRING),            \
        VMDIR_SF_INIT(.pszOid, VDIR_OID_OCTET_STRING),                \
        VMDIR_SF_INIT(.readableFlag, VDIR_SYNTAX_HUMAN_READABLE_YES), \
        VMDIR_SF_INIT(.pValidateFunc, syntaxOctetString)              \
    },                                                                \
    {                                                                 \
        VMDIR_SF_INIT(.pszName, VDIR_SYNTAX_OBJECTCLASS_DESC),        \
        VMDIR_SF_INIT(.pszOid, VDIR_OID_OBJECTCLASS_DESC),            \
        VMDIR_SF_INIT(.readableFlag, VDIR_SYNTAX_HUMAN_READABLE_YES), \
        VMDIR_SF_INIT(.pValidateFunc, syntaxObjectClassDesc)          \
    },                                                                \
    {                                                                 \
        VMDIR_SF_INIT(.pszName, VDIR_SYNTAX_OID),                     \
        VMDIR_SF_INIT(.pszOid, VDIR_OID_OID),                         \
        VMDIR_SF_INIT(.readableFlag, VDIR_SYNTAX_HUMAN_READABLE_YES), \
        VMDIR_SF_INIT(.pValidateFunc, syntaxOID)                      \
    },                                                                \
    {                                                                 \
        VMDIR_SF_INIT(.pszName, VDIR_SYNTAX_POSTAL_ADDRESS),          \
        VMDIR_SF_INIT(.pszOid, VDIR_OID_POSTAL_ADDRESS),              \
        VMDIR_SF_INIT(.readableFlag, VDIR_SYNTAX_HUMAN_READABLE_YES), \
        VMDIR_SF_INIT(.pValidateFunc, syntaxPostalAddress)            \
    },                                                                \
    {                                                                 \
        VMDIR_SF_INIT(.pszName, VDIR_SYNTAX_PRINTABLE_STRING),        \
        VMDIR_SF_INIT(.pszOid, VDIR_OID_PRINTABLE_STRING),            \
        VMDIR_SF_INIT(.readableFlag, VDIR_SYNTAX_HUMAN_READABLE_YES), \
        VMDIR_SF_INIT(.pValidateFunc, syntaxPrintableString)          \
    },                                                                \
    {                                                                 \
        VMDIR_SF_INIT(.pszName, VDIR_SYNTAX_TELEPHONE_NUMBER),        \
        VMDIR_SF_INIT(.pszOid, VDIR_OID_TELEPHONE_NUMBER),            \
        VMDIR_SF_INIT(.readableFlag, VDIR_SYNTAX_HUMAN_READABLE_YES), \
        VMDIR_SF_INIT(.pValidateFunc, syntaxTelephoneNumber)          \
    },                                                                \
    {                                                                 \
        VMDIR_SF_INIT(.pszName, VDIR_SYNTAX_UTC_TIME),                \
        VMDIR_SF_INIT(.pszOid, VDIR_OID_UTC_TIME),                    \
        VMDIR_SF_INIT(.readableFlag, VDIR_SYNTAX_HUMAN_READABLE_YES), \
        VMDIR_SF_INIT(.pValidateFunc, syntaxUTCTime)                  \
    },                                                                \
};

/*  TODO, only support string type matching rule now.
   ( 2.5.13.0 NAME 'objectIdentifierMatch' SYNTAX 1.3.6.1.4.1.1466.115.121.1.38 )
   ( 2.5.13.1 NAME 'distinguishedNameMatch' SYNTAX 1.3.6.1.4.1.1466.115.121.1.12 )
   ( 2.5.13.2 NAME 'caseIgnoreMatch' SYNTAX 1.3.6.1.4.1.1466.115.121.1.15 )
   ( 2.5.13.3 NAME 'caseIgnoreOrderingMatch' SYNTAX 1.3.6.1.4.1.1466.115.121.1.15 )
   ( 2.5.13.4 NAME 'caseIgnoreSubstringsMatch' SYNTAX 1.3.6.1.4.1.1466.115.121.1.58 )
   ( 2.5.13.5 NAME 'caseExactMatch' SYNTAX 1.3.6.1.4.1.1466.115.121.1.15 )
   ( 2.5.13.6 NAME 'caseExactOrderingMatch' SYNTAX 1.3.6.1.4.1.1466.115.121.1.15 )
   ( 2.5.13.7 NAME 'caseExactSubstringsMatch' SYNTAX 1.3.6.1.4.1.1466.115.121.1.58 )
   ( 2.5.13.16 NAME 'bitStringMatch' SYNTAX 1.3.6.1.4.1.1466.115.121.1.6 )
   ( 2.5.13.13 NAME 'booleanMatch' SYNTAX 1.3.6.1.4.1.1466.115.121.1.7 )
   ( 2.5.13.30 NAME 'objectIdentifierFirstComponentMatch' SYNTAX 1.3.6.1.4.1.1466.115.121.1.38 )
   ( 1.3.6.1.4.1.1466.109.114.1 NAME 'caseExactIA5Match' SYNTAX 1.3.6.1.4.1.1466.115.121.1.26 )
   ( 1.3.6.1.4.1.1466.109.114.2 NAME 'caseIgnoreIA5Match' SYNTAX 1.3.6.1.4.1.1466.115.121.1.26 )
   ( 1.3.6.1.4.1.1466.109.114.3 NAME 'caseIgnoreIA5SubstringsMatch' SYNTAX 1.3.6.1.4.1.1466.115.121.1.58 )
   ( 2.5.13.8 NAME 'numericStringMatch' SYNTAX 1.3.6.1.4.1.1466.115.121.1.36 )
   ( 2.5.13.9 NAME 'numericStringOrderingMatch' SYNTAX 1.3.6.1.4.1.1466.115.121.1.36 )
   ( 2.5.13.10 NAME 'numericStringSubstringsMatch' SYNTAX 1.3.6.1.4.1.1466.115.121.1.58 )
   ( 2.5.13.14 NAME 'integerMatch' SYNTAX 1.3.6.1.4.1.1466.115.121.1.27 )
   ( 2.5.13.15 NAME 'integerOrderingMatch' SYNTAX 1.3.6.1.4.1.1466.115.121.1.27 )
 */

#define VDIR_MATCHING_RULE_BITSTRING_MATCH                  "bitStringMatch"
#define VDIR_MATCHING_RULE_BOOLEAN_MATCH                    "booleanMatch"
#define VDIR_MATCHING_RULE_OBJID_FIRST_COMPONENT_MATCH      "objectIdentifierFirstComponentMatch"
#define VDIR_MATCHING_RULE_OBJID_MATCH                      "objectIdentifierMatch"
#define VDIR_MATCHING_RULE_DN_MATCH                         "distinguishedNameMatch"

#define VDIR_MATCHING_RULE_CASEEXACT_MATCH                  "caseExactMatch"
#define VDIR_MATCHING_RULE_CASEEXACT_ORDERING_MATCH         "caseExactOrderingMatch"
#define VDIR_MATCHING_RULE_CASEEXACT_SUBSTR_MATCH           "caseExactSubstringsMatch"

#define VDIR_MATCHING_RULE_CASEIGNORE_MATCH                 "caseIgnoreMatch"
#define VDIR_MATCHING_RULE_CASEIGNORE_ORDERING_MATCH        "caseIgnoreOrderingMatch"
#define VDIR_MATCHING_RULE_CASEIGNORE_SUBSTR_MATCH          "caseIgnoreSubstringsMatch"

#define VDIR_MATCHING_RULE_CASEEXATCH_IA5_MATCH             "caseExactIA5Match"
#define VDIR_MATCHING_RULE_CASEIGNORE_IA5_MATCH             "caseIgnoreIA5Match"
#define VDIR_MATCHING_RULE_CASEIGNORE_IA5_SUBSTR_MATCH      "caseIgnoreIA5SubstringsMatch"

#define VDIR_MATCHING_RULE_NUMERIC_STRING_MATCH             "numericStringMatch"
#define VDIR_MATCHING_RULE_NUMERIC_STRING_ORDERING_MATCH    "numericStringOrderingMatch"
#define VDIR_MATCHING_RULE_NUMERIC_STRING_SUBSTR_MATCH      "numericStringSubstringsMatch"

#define VDIR_MATCHING_RULE_INTEGER_MATCH                    "integerMatch"
#define VDIR_MATCHING_RULE_INTEGER_ORDERING_MATCH           "integerOrderingMatch"

// NOTE: order of fields MUST stay in sync with struct definition...
#define VDIR_MATCHING_RULE_INIT_TABLE_INITIALIZER                                  \
{                                                                                  \
    {                                                                              \
        VMDIR_SF_INIT(.pszName, VDIR_MATCHING_RULE_DN_MATCH),                      \
        VMDIR_SF_INIT(.pszOid, "2.5.13.1"),                                        \
        VMDIR_SF_INIT(.pszSyntaxOid, VDIR_OID_DN),                                 \
        VMDIR_SF_INIT(.pSyntax, NULL),                                             \
        VMDIR_SF_INIT(.pNormalizeFunc, VmDirNormalizeDNWrapper),                   \
        VMDIR_SF_INIT(.pCompareFunc, compareString)                                \
    },                                                                             \
    {                                                                              \
        VMDIR_SF_INIT(.pszName, VDIR_MATCHING_RULE_BITSTRING_MATCH),               \
        VMDIR_SF_INIT(.pszOid, "2.5.13.16"),                                       \
        VMDIR_SF_INIT(.pszSyntaxOid, VDIR_OID_BIT_STRING),                         \
        VMDIR_SF_INIT(.pSyntax, NULL),                                             \
        VMDIR_SF_INIT(.pNormalizeFunc, NULL),                                      \
        VMDIR_SF_INIT(.pCompareFunc, NULL)                                         \
    },                                                                             \
    {                                                                              \
        VMDIR_SF_INIT(.pszName, VDIR_MATCHING_RULE_OBJID_FIRST_COMPONENT_MATCH),   \
        VMDIR_SF_INIT(.pszOid, "2.5.13.30"),                                       \
        VMDIR_SF_INIT(.pszSyntaxOid, VDIR_OID_OID),                                \
        VMDIR_SF_INIT(.pSyntax, NULL),                                             \
        VMDIR_SF_INIT(.pNormalizeFunc, NULL),                                      \
        VMDIR_SF_INIT(.pCompareFunc, NULL)                                         \
    },                                                                             \
    {                                                                              \
        VMDIR_SF_INIT(.pszName, VDIR_MATCHING_RULE_OBJID_MATCH),                   \
        VMDIR_SF_INIT(.pszOid, "2.5.13.0"),                                        \
        VMDIR_SF_INIT(.pszSyntaxOid, VDIR_OID_OID),                                \
        VMDIR_SF_INIT(.pSyntax, NULL),                                             \
        VMDIR_SF_INIT(.pNormalizeFunc, NULL),                                      \
        VMDIR_SF_INIT(.pCompareFunc, NULL)                                         \
    },                                                                             \
    {                                                                              \
        VMDIR_SF_INIT(.pszName, VDIR_MATCHING_RULE_CASEEXACT_MATCH),               \
        VMDIR_SF_INIT(.pszOid, "2.5.13.5"),                                        \
        VMDIR_SF_INIT(.pszSyntaxOid, VDIR_OID_DIRECTORY_STRING),                   \
        VMDIR_SF_INIT(.pSyntax, NULL),                                             \
        VMDIR_SF_INIT(.pNormalizeFunc, normalizeCaseExact),                        \
        VMDIR_SF_INIT(.pCompareFunc, compareString)                                \
    },                                                                             \
    {                                                                              \
        VMDIR_SF_INIT(.pszName, VDIR_MATCHING_RULE_CASEEXACT_ORDERING_MATCH),      \
        VMDIR_SF_INIT(.pszOid, "2.5.13.6"),                                        \
        VMDIR_SF_INIT(.pszSyntaxOid, VDIR_OID_DIRECTORY_STRING),                   \
        VMDIR_SF_INIT(.pSyntax, NULL),                                             \
        VMDIR_SF_INIT(.pNormalizeFunc, normalizeCaseExact),                        \
        VMDIR_SF_INIT(.pCompareFunc, compareString)                                \
    },                                                                             \
    {                                                                              \
        VMDIR_SF_INIT(.pszName, VDIR_MATCHING_RULE_CASEEXACT_SUBSTR_MATCH),        \
        VMDIR_SF_INIT(.pszOid, "2.5.13.7"),                                        \
        VMDIR_SF_INIT(.pszSyntaxOid, VDIR_OID_DIRECTORY_STRING),                   \
        VMDIR_SF_INIT(.pSyntax, NULL),                                             \
        VMDIR_SF_INIT(.pNormalizeFunc, normalizeCaseExact),                        \
        VMDIR_SF_INIT(.pCompareFunc, compareSubString)                             \
    },                                                                             \
    {                                                                              \
        VMDIR_SF_INIT(.pszName, VDIR_MATCHING_RULE_CASEIGNORE_MATCH),              \
        VMDIR_SF_INIT(.pszOid, "2.5.13.2"),                                        \
        VMDIR_SF_INIT(.pszSyntaxOid, VDIR_OID_DIRECTORY_STRING),                   \
        VMDIR_SF_INIT(.pSyntax, NULL),                                             \
        VMDIR_SF_INIT(.pNormalizeFunc, normalizeCaseIgnore),                       \
        VMDIR_SF_INIT(.pCompareFunc, compareString)                                \
    },                                                                             \
    {                                                                              \
        VMDIR_SF_INIT(.pszName, VDIR_MATCHING_RULE_CASEIGNORE_ORDERING_MATCH),     \
        VMDIR_SF_INIT(.pszOid, "2.5.13.3"),                                        \
        VMDIR_SF_INIT(.pszSyntaxOid, VDIR_OID_DIRECTORY_STRING),                   \
        VMDIR_SF_INIT(.pSyntax, NULL),                                             \
        VMDIR_SF_INIT(.pNormalizeFunc, normalizeCaseIgnore),                       \
        VMDIR_SF_INIT(.pCompareFunc, compareString)                                \
    },                                                                             \
    {                                                                              \
        VMDIR_SF_INIT(.pszName, VDIR_MATCHING_RULE_CASEIGNORE_SUBSTR_MATCH),       \
        VMDIR_SF_INIT(.pszOid, "2.5.13.4"),                                        \
        VMDIR_SF_INIT(.pszSyntaxOid, VDIR_OID_DIRECTORY_STRING),                   \
        VMDIR_SF_INIT(.pSyntax, NULL),                                             \
        VMDIR_SF_INIT(.pNormalizeFunc, normalizeCaseIgnore),                       \
        VMDIR_SF_INIT(.pCompareFunc, compareSubString)                             \
    },                                                                             \
    {                                                                              \
        VMDIR_SF_INIT(.pszName, VDIR_MATCHING_RULE_CASEEXATCH_IA5_MATCH),          \
        VMDIR_SF_INIT(.pszOid, "1.3.6.1.4.1.1466.109.114.1"),                      \
        VMDIR_SF_INIT(.pszSyntaxOid, VDIR_OID_IA5_STRING),                         \
        VMDIR_SF_INIT(.pSyntax, NULL),                                             \
        VMDIR_SF_INIT(.pNormalizeFunc, normalizeCaseExact),                        \
        VMDIR_SF_INIT(.pCompareFunc, compareString)                                \
    },                                                                             \
    {                                                                              \
        VMDIR_SF_INIT(.pszName, VDIR_MATCHING_RULE_CASEIGNORE_IA5_MATCH),          \
        VMDIR_SF_INIT(.pszOid, "1.3.6.1.4.1.1466.109.114.2"),                      \
        VMDIR_SF_INIT(.pszSyntaxOid, VDIR_OID_IA5_STRING),                         \
        VMDIR_SF_INIT(.pSyntax, NULL),                                             \
        VMDIR_SF_INIT(.pNormalizeFunc, normalizeCaseIgnore),                       \
        VMDIR_SF_INIT(.pCompareFunc, compareString)                                \
    },                                                                             \
    {                                                                              \
        VMDIR_SF_INIT(.pszName, VDIR_MATCHING_RULE_CASEIGNORE_IA5_SUBSTR_MATCH),   \
        VMDIR_SF_INIT(.pszOid, "1.3.6.1.4.1.1466.109.114.3"),                      \
        VMDIR_SF_INIT(.pszSyntaxOid, VDIR_OID_IA5_STRING),                         \
        VMDIR_SF_INIT(.pSyntax, NULL),                                             \
        VMDIR_SF_INIT(.pNormalizeFunc, normalizeCaseIgnore),                       \
        VMDIR_SF_INIT(.pCompareFunc, compareSubString)                             \
    },                                                                             \
    {                                                                              \
        VMDIR_SF_INIT(.pszName, VDIR_MATCHING_RULE_NUMERIC_STRING_MATCH),          \
        VMDIR_SF_INIT(.pszOid, "2.5.13.8"),                                        \
        VMDIR_SF_INIT(.pszSyntaxOid, VDIR_OID_NUMERIC_STRING),                     \
        VMDIR_SF_INIT(.pSyntax, NULL),                                             \
        VMDIR_SF_INIT(.pNormalizeFunc, normalizeCaseExact),                        \
        VMDIR_SF_INIT(.pCompareFunc, compareString)                                \
    },                                                                             \
    {                                                                              \
        VMDIR_SF_INIT(.pszName, VDIR_MATCHING_RULE_NUMERIC_STRING_ORDERING_MATCH), \
        VMDIR_SF_INIT(.pszOid, "2.5.13.9"),                                        \
        VMDIR_SF_INIT(.pszSyntaxOid, VDIR_OID_NUMERIC_STRING),                     \
        VMDIR_SF_INIT(.pSyntax, NULL),                                             \
        VMDIR_SF_INIT(.pNormalizeFunc, normalizeCaseExact),                        \
        VMDIR_SF_INIT(.pCompareFunc, compareString)                                \
    },                                                                             \
    {                                                                              \
        VMDIR_SF_INIT(.pszName, VDIR_MATCHING_RULE_NUMERIC_STRING_SUBSTR_MATCH),   \
        VMDIR_SF_INIT(.pszOid, "2.5.13.10"),                                       \
        VMDIR_SF_INIT(.pszSyntaxOid, VDIR_OID_NUMERIC_STRING),                     \
        VMDIR_SF_INIT(.pSyntax, NULL),                                             \
        VMDIR_SF_INIT(.pNormalizeFunc, normalizeCaseExact),                        \
        VMDIR_SF_INIT(.pCompareFunc, compareSubString)                             \
    },                                                                             \
    {                                                                              \
        VMDIR_SF_INIT(.pszName, VDIR_MATCHING_RULE_INTEGER_MATCH),                 \
        VMDIR_SF_INIT(.pszOid, "2.5.13.14"),                                       \
        VMDIR_SF_INIT(.pszSyntaxOid, VDIR_OID_INTERGER),                           \
        VMDIR_SF_INIT(.pSyntax, NULL),                                             \
        VMDIR_SF_INIT(.pNormalizeFunc, NULL),                                      \
        VMDIR_SF_INIT(.pCompareFunc, compareIntegerString)                         \
    },                                                                             \
    {                                                                              \
        VMDIR_SF_INIT(.pszName, VDIR_MATCHING_RULE_INTEGER_ORDERING_MATCH),        \
        VMDIR_SF_INIT(.pszOid, "2.5.13.15"),                                       \
        VMDIR_SF_INIT(.pszSyntaxOid, VDIR_OID_INTERGER),                           \
        VMDIR_SF_INIT(.pSyntax, NULL),                                             \
        VMDIR_SF_INIT(.pNormalizeFunc, NULL),                                      \
        VMDIR_SF_INIT(.pCompareFunc, compareIntegerString)                         \
    },                                                                             \
};

// 0 is reserved for NO Id case
// 1~99 is reserved for bootstrap attrs
// auto generated Id starts from 100
#define NO_ATTR_ID_MAP               0
#define MAX_RESERVED_ATTR_ID_MAP    99

#define VDIR_SCHEMA_BOOTSTRAP_ENTRY_OBJECTCLASS_INITIALIZER  \
{                                                            \
    "subentry",                                              \
    "subschema",                                             \
};

// Supported attributetypes in bootstrap schema entry
#define VDIR_ATTRIBUTE_OBJECT_CLASSES        "objectClasses"
#define VDIR_ATTRIBUTE_ATTRIBUTE_TYPES       "attributeTypes"
#define VDIR_ATTRIBUTE_LADPSYNTAXES          "ldapSyntaxes"
#define VDIR_ATTRIBUTE_MATCHINGRULEUSE       "matchingRuleUse"
#define VDIR_ATTRIBUTE_MATCHINGRULES         "matchingRules"
#define VDIR_ATTRIBUTE_SUBSCHEMA_SUBENTRY    "subschemaSubentry"
#define VDIR_ATTRIBUTE_STRUCTURAL_OBJECTCLASS "structuralObjectClass"
#define VDIR_ATTRIBUTE_ENTRYDN                "entryDN"
#define VDIR_ATTRIBUTE_OBJECTCLASS            "objectClass"
#define VDIR_ATTRIBUTE_DIT_STRUCTURERULES     "dITStructureRules"
#define VDIR_ATTRIBUTE_DIT_CONTENTRULES       "dITContentRules"
#define VDIR_ATTRIBUTE_NAMEFORMS              "nameForms"
#define VDIR_ATTRIBUTE_EXTENSIBLE_OBJECT      "extensibleObject"
#define VDIR_ATTRIBUTE_CREATE_TIMESTAMP       "createTimestamp"
#define VDIR_ATTRIBUTE_MODIFY_TIMESTAMP       "modifyTimestamp"
#define VDIR_ATTRIBUTE_ATTRIBUTETOIDMAP       "vmwAttributeToIdMap"
#define VDIR_ATTRIBUTE_CN                     "cn"


#define VDIR_MUTABLE_SCHEMA_ELEMENT_INITIALIZER              \
{                                                            \
    VDIR_ATTRIBUTE_OBJECT_CLASSES,                           \
    VDIR_ATTRIBUTE_ATTRIBUTE_TYPES,                          \
    VDIR_ATTRIBUTE_DIT_STRUCTURERULES,                       \
    VDIR_ATTRIBUTE_DIT_CONTENTRULES,                         \
    VDIR_ATTRIBUTE_NAMEFORMS,                                \
}

#define VDIR_IMMUTABLE_SCHEMA_ELEMENT_INITIALIZER            \
{                                                            \
    VDIR_ATTRIBUTE_LADPSYNTAXES,                             \
    VDIR_ATTRIBUTE_MATCHINGRULEUSE,                          \
    VDIR_ATTRIBUTE_MATCHINGRULES,                            \
}

// index into fix map VDIR_SCHEMA_BOOTSTRP_ATTR_INITIALIZER below
#define VDIR_ATTRIBUTE_OBJECTCLASS_INDEX    8

// The only thing we need to bootstrap schema is id and name
// NOTE: order of fields MUST stay in sync with struct definition...
#define VDIR_SCHEMA_BOOTSTRP_ATTR_INITIALIZER                                                           \
{                                                                                                       \
    {                                                                                                   \
    VMDIR_SF_INIT(.usAttrID, 1),                                                                        \
    VMDIR_SF_INIT(.pszDesc, "attributeTypes: ( 2.5.21.6 NAME 'objectClasses' )")                        \
    },                                                                                                  \
    {                                                                                                   \
    VMDIR_SF_INIT(.usAttrID, 2),                                                                        \
    VMDIR_SF_INIT(.pszDesc, "attributeTypes: ( 2.5.21.5 NAME 'attributeTypes' )")                       \
    },                                                                                                  \
    {                                                                                                   \
    VMDIR_SF_INIT(.usAttrID, 3),                                                                        \
    VMDIR_SF_INIT(.pszDesc, "attributeTypes: ( 1.3.6.1.4.1.1466.101.120.16 NAME 'ldapSyntaxes' )")      \
    },                                                                                                  \
    {                                                                                                   \
    VMDIR_SF_INIT(.usAttrID, 4),                                                                        \
    VMDIR_SF_INIT(.pszDesc, "attributeTypes: ( 2.5.21.8 NAME 'matchingRuleUse' )")                      \
    },                                                                                                  \
    {                                                                                                   \
    VMDIR_SF_INIT(.usAttrID, 5),                                                                        \
    VMDIR_SF_INIT(.pszDesc, "attributeTypes: ( 2.5.21.4 NAME 'matchingRules' )")                        \
    },                                                                                                  \
    {                                                                                                   \
    VMDIR_SF_INIT(.usAttrID, 6),                                                                        \
    VMDIR_SF_INIT(.pszDesc, "attributeTypes: ( 2.5.18.10 NAME 'subschemaSubentry' )")                   \
    },                                                                                                  \
    {                                                                                                   \
    VMDIR_SF_INIT(.usAttrID, 7),                                                                        \
    VMDIR_SF_INIT(.pszDesc, "attributeTypes: ( 2.5.21.9 NAME 'structuralObjectClass' )")                \
    },                                                                                                  \
    {                                                                                                   \
    VMDIR_SF_INIT(.usAttrID, 8),                                                                        \
    VMDIR_SF_INIT(.pszDesc, "attributeTypes: ( 1.3.6.1.1.20 NAME 'entryDN' )")                          \
    },                                                                                                  \
    {                                                                                                   \
    VMDIR_SF_INIT(.usAttrID, 9),                                                                        \
    VMDIR_SF_INIT(.pszDesc, "attributeTypes: ( 2.5.4.0 NAME 'objectClass' )")                           \
    },                                                                                                  \
    {                                                                                                   \
    VMDIR_SF_INIT(.usAttrID, 10),                                                                       \
    VMDIR_SF_INIT(.pszDesc, "attributeTypes: ( 2.5.21.1 NAME 'dITStructureRules' )")                    \
    },                                                                                                  \
    {                                                                                                   \
    VMDIR_SF_INIT(.usAttrID, 11),                                                                       \
    VMDIR_SF_INIT(.pszDesc, "attributeTypes: ( 2.5.21.2 NAME 'dITContentRules' )")                      \
    },                                                                                                  \
    {                                                                                                   \
    VMDIR_SF_INIT(.usAttrID, 12),                                                                       \
    VMDIR_SF_INIT(.pszDesc, "attributeTypes: ( 2.5.21.7 NAME 'nameForms' )")                            \
    },                                                                                                  \
    {                                                                                                   \
    VMDIR_SF_INIT(.usAttrID, 13),                                                                       \
    VMDIR_SF_INIT(.pszDesc, "attributeTypes: ( 1.3.6.1.4.1.1466.101.120.111 NAME 'extensibleObject' )") \
    },                                                                                                  \
    {                                                                                                   \
    VMDIR_SF_INIT(.usAttrID, 14),                                                                       \
    VMDIR_SF_INIT(.pszDesc, "attributeTypes: ( 2.5.18.1 NAME 'createTimestamp' )")                      \
    },                                                                                                  \
    {                                                                                                   \
    VMDIR_SF_INIT(.usAttrID, 15),                                                                       \
    VMDIR_SF_INIT(.pszDesc, "attributeTypes: ( 2.5.18.2 NAME 'modifyTimestamp' )")                      \
    },                                                                                                  \
    {                                                                                                   \
    VMDIR_SF_INIT(.usAttrID, 16),                                                                       \
    VMDIR_SF_INIT(.pszDesc, "attributeTypes: ( 999.999.0.1 NAME 'vmwAttributeToIdMap' )")               \
    },                                                                                                  \
    {                                                                                                   \
    VMDIR_SF_INIT(.usAttrID, 17),                                                                       \
    VMDIR_SF_INIT(.pszDesc, "attributeTypes: ( 2.5.4.3 NAME 'cn' )")                                    \
    },                                                                                                  \
    {                                                                                                   \
    VMDIR_SF_INIT(.usAttrID, 18),                                                                       \
    VMDIR_SF_INIT(.pszDesc, "attributeTypes: ( 999.999.0.4 NAME 'uSNCreated' )")                        \
    },                                                                                                  \
    {                                                                                                   \
    VMDIR_SF_INIT(.usAttrID, 19),                                                                       \
    VMDIR_SF_INIT(.pszDesc, "attributeTypes: ( 999.999.0.5 NAME 'uSNChanged' )")                        \
    },                                                                                                  \
    {                                                                                                   \
    VMDIR_SF_INIT(.usAttrID, 20),                                                                       \
    VMDIR_SF_INIT(.pszDesc, "attributeTypes: ( 999.999.0.9 NAME 'dn' )")                                \
    },                                                                                                  \
    {                                                                                                   \
    VMDIR_SF_INIT(.usAttrID, 21),                                                                       \
    VMDIR_SF_INIT(.pszDesc, "attributeTypes: ( 999.999.0.29 NAME 'vmwSecurityDescriptor' )")            \
    },                                                                                                  \
    {                                                                                                   \
    VMDIR_SF_INIT(.usAttrID, SCHEMA_BOOTSTRAP_EID_SEQ_ATTRID_22),                                       \
    VMDIR_SF_INIT(.pszDesc, "attributeTypes: ( 999.999.0.32 NAME 'vmwEntryIdSequenceNumber' DESC 'ID NEEDS TO BE 22' )")  \
    },                                                                                                  \
    {                                                                                                   \
    VMDIR_SF_INIT(.usAttrID, SCHEMA_BOOTSTRAP_USN_SEQ_ATTRID_23),                                       \
    VMDIR_SF_INIT(.pszDesc, "attributeTypes: ( 999.999.0.33 NAME 'vmwUSNSequenceNumber' DESC 'ID NEEDS TO BE 23' )") \
    },                                                                                                  \
    {                                                                                                   \
    VMDIR_SF_INIT(.usAttrID, 24),                                                                       \
    VMDIR_SF_INIT(.pszDesc, "attributeTypes: ( 999.999.0.7 NAME 'objectGUID' DESC 'ID NEEDS TO BE 24' )")        \
    },\
}


#define VDIR_SCHEMA_BOOTSTRP_OC_INITIALIZER                      \
{                                                                \
    "objectClasses: ( 2.5.20.1 NAME 'subschema' DESC 'RFC4512: controlling subschema (sub)entry' " \
            "AUXILIARY MAY ( dITStructureRules $ nameForms $ ditContentRules $ ldapSyntaxes $ "\
            "objectClasses $ attributeTypes $ matchingRules $ matchingRuleUse $ vmwAttributeToIdMap ) )",    \
    "objectClasses: ( 2.5.17.0 NAME 'subentry' DESC 'RFC3672: subentry' SUP top STRUCTURAL MUST ( cn ) )" \
}
