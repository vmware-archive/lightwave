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

#define ATTR_VMW_ATTRIBUTE_TO_ID_MAP    "vmwAttributeToIdMap"
#define SCHEMA_ATTR_ID_MAP_TOKEN_SEP    ":"

// NOTE: order of fields MUST stay in sync with struct definition...
#define VDIR_LEGACY_SCHEMA_BOOTSTRP_ATTR_INITIALIZER                    \
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
    VMDIR_SF_INIT(.usAttrID, 99),                                       \
    VMDIR_SF_INIT(.pszDesc, "attributeTypes: ("                         \
            " VMWare.DIR.attribute.0.59"                                \
            " NAME 'attributeValueMetaData'"                            \
            " SYNTAX 1.3.6.1.4.1.1466.115.121.1.40"                     \
            " NO-USER-MODIFICATION"                                     \
            " USAGE directoryOperation )")                              \
    },                                                                  \
    {                                                                   \
    VMDIR_SF_INIT(.usAttrID, 0),                                        \
    VMDIR_SF_INIT(.pszDesc, NULL)                                       \
    },                                                                  \
}
