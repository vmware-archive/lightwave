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
 * Module Name: Directory indexer
 *
 * Filename: defines.h
 *
 * Abstract:
 *
 * Directory indexer module
 *
 * Definitions
 *
 */

#define MAX_ATTR_INDEX_CACHE_INSTANCE   20
#define ENTRY_INDEXING_BATCH_SIZE       25

#define ATTR_INDEX_DESC                 "vmwAttrIndexDesc"
#define ATTR_INDEX_BUILDING_FLAG        "(B)"

// To add a new boot time index, just add another section in this table.
// IMPORTANT - The last section must has pszAttrName = NULL.
// NOTE: order of fields MUST stay in sync with struct definition...
#define VDIR_CFG_INDEX_INITIALIZER                                       \
{                                                                        \
    {                                                                    \
        VMDIR_SF_INIT(.status, VDIR_CFG_ATTR_INDEX_ENABLED),             \
        VMDIR_SF_INIT(.pszAttrName, ATTR_CN),                            \
        VMDIR_SF_INIT(.iTypes, INDEX_TYPE_EQUALITY | INDEX_TYPE_SUBSTR), \
        VMDIR_SF_INIT(.bIsUnique, FALSE),                                \
        VMDIR_SF_INIT(.bIsNumeric, FALSE),                               \
        VMDIR_SF_INIT(.iId, 0)                                           \
    },                                                                   \
    {                                                                    \
        VMDIR_SF_INIT(.status, VDIR_CFG_ATTR_INDEX_ENABLED),             \
        VMDIR_SF_INIT(.pszAttrName, ATTR_OBJECT_CLASS),                  \
        VMDIR_SF_INIT(.iTypes, INDEX_TYPE_EQUALITY),                     \
        VMDIR_SF_INIT(.bIsUnique, FALSE),                                \
        VMDIR_SF_INIT(.bIsNumeric, FALSE),                               \
        VMDIR_SF_INIT(.iId, 0)                                           \
    },                                                                   \
    {                                                                    \
        VMDIR_SF_INIT(.status, VDIR_CFG_ATTR_INDEX_ENABLED),             \
        VMDIR_SF_INIT(.pszAttrName, ATTR_DN),                            \
        VMDIR_SF_INIT(.iTypes, INDEX_TYPE_EQUALITY | INDEX_TYPE_SUBSTR), \
        VMDIR_SF_INIT(.bIsUnique, TRUE),                                 \
        VMDIR_SF_INIT(.bIsNumeric, FALSE),                               \
        VMDIR_SF_INIT(.iId, 0)                                           \
    },                                                                   \
    {                                                                    \
        VMDIR_SF_INIT(.status, VDIR_CFG_ATTR_INDEX_ENABLED),             \
        VMDIR_SF_INIT(.pszAttrName, ATTR_PARENT_ID), /* Not really an attribute, just identifies the index */ \
        VMDIR_SF_INIT(.iTypes, INDEX_TYPE_EQUALITY),                     \
        VMDIR_SF_INIT(.bIsUnique, FALSE),                                \
        VMDIR_SF_INIT(.bIsNumeric, FALSE), /* treat parentid as str because no need of ordering */   \
        VMDIR_SF_INIT(.iId, 0)                                           \
    },                                                                   \
    {                                                                    \
        VMDIR_SF_INIT(.status, VDIR_CFG_ATTR_INDEX_ENABLED),             \
        VMDIR_SF_INIT(.pszAttrName, ATTR_MEMBER),                        \
        VMDIR_SF_INIT(.iTypes, INDEX_TYPE_EQUALITY),                     \
        VMDIR_SF_INIT(.bIsUnique, FALSE),                                \
        VMDIR_SF_INIT(.bIsNumeric, FALSE),                               \
        VMDIR_SF_INIT(.iId, 0)                                           \
    },                                                                   \
    {                                                                    \
        VMDIR_SF_INIT(.status, VDIR_CFG_ATTR_INDEX_ENABLED),             \
        VMDIR_SF_INIT(.pszAttrName, ATTR_USN_CHANGED),                   \
        VMDIR_SF_INIT(.iTypes, INDEX_TYPE_EQUALITY),                     \
        VMDIR_SF_INIT(.bIsUnique, TRUE),                                 \
        VMDIR_SF_INIT(.bIsNumeric, TRUE),                                \
        VMDIR_SF_INIT(.iId, 0)                                           \
    },                                                                   \
    {                                                                    \
        VMDIR_SF_INIT(.status, VDIR_CFG_ATTR_INDEX_ENABLED),             \
        VMDIR_SF_INIT(.pszAttrName, ATTR_ATTR_META_DATA),                \
        VMDIR_SF_INIT(.iTypes, INDEX_TYPE_EQUALITY),                     \
        VMDIR_SF_INIT(.bIsUnique, TRUE),                                 \
        VMDIR_SF_INIT(.bIsNumeric, FALSE),                               \
        VMDIR_SF_INIT(.iId, 0)                                           \
    },                                                                   \
    {                                                                    \
        VMDIR_SF_INIT(.status, VDIR_CFG_ATTR_INDEX_ENABLED),             \
        VMDIR_SF_INIT(.pszAttrName,  ATTR_OBJECT_SID),                   \
        VMDIR_SF_INIT(.iTypes, INDEX_TYPE_EQUALITY),                     \
        VMDIR_SF_INIT(.bIsUnique, TRUE),                                 \
        VMDIR_SF_INIT(.bIsNumeric, FALSE),                               \
        VMDIR_SF_INIT(.iId, 0)                                           \
    },                                                                   \
    {                                                                    \
        VMDIR_SF_INIT(.status, VDIR_CFG_ATTR_INDEX_ENABLED),             \
        VMDIR_SF_INIT(.pszAttrName,  ATTR_KRB_UPN),                      \
        VMDIR_SF_INIT(.iTypes, INDEX_TYPE_EQUALITY),                     \
        VMDIR_SF_INIT(.bIsUnique, TRUE),                                 \
        VMDIR_SF_INIT(.bIsNumeric, FALSE),                               \
        VMDIR_SF_INIT(.iId, 0)                                           \
    },                                                                   \
    {                                                                    \
        VMDIR_SF_INIT(.status, VDIR_CFG_ATTR_INDEX_ENABLED),             \
        VMDIR_SF_INIT(.pszAttrName,  ATTR_KRB_SPN),                      \
        VMDIR_SF_INIT(.iTypes, INDEX_TYPE_EQUALITY),                     \
        VMDIR_SF_INIT(.bIsUnique, TRUE),                                 \
        VMDIR_SF_INIT(.bIsNumeric, FALSE),                               \
        VMDIR_SF_INIT(.iId, 0)                                           \
    },                                                                   \
    {                                                                    \
        VMDIR_SF_INIT(.status, VDIR_CFG_ATTR_INDEX_ENABLED),             \
        VMDIR_SF_INIT(.pszAttrName,  ATTR_SAM_ACCOUNT_NAME),             \
        VMDIR_SF_INIT(.iTypes, INDEX_TYPE_EQUALITY),                     \
        VMDIR_SF_INIT(.bIsUnique, FALSE),                                \
        VMDIR_SF_INIT(.bIsNumeric, FALSE),                               \
        VMDIR_SF_INIT(.iId, 0)                                           \
    },                                                                   \
    {                                                                    \
        VMDIR_SF_INIT(.status, VDIR_CFG_ATTR_INDEX_ENABLED),             \
        VMDIR_SF_INIT(.pszAttrName,  ATTR_VMWITCUSTOMERNUMBER),          \
        VMDIR_SF_INIT(.iTypes, INDEX_TYPE_EQUALITY),                     \
        VMDIR_SF_INIT(.bIsUnique, TRUE),                                 \
        VMDIR_SF_INIT(.bIsNumeric, FALSE),                               \
        VMDIR_SF_INIT(.iId, 0)                                           \
    },                                                                   \
    {                                                                    \
        VMDIR_SF_INIT(.status, VDIR_CFG_ATTR_INDEX_ENABLED),             \
        VMDIR_SF_INIT(.pszAttrName,  ATTR_UID),                          \
        VMDIR_SF_INIT(.iTypes, INDEX_TYPE_EQUALITY),                     \
        VMDIR_SF_INIT(.bIsUnique, TRUE),                                 \
        VMDIR_SF_INIT(.bIsNumeric, FALSE),                               \
        VMDIR_SF_INIT(.iId, 0)                                           \
    },                                                                   \
    {                                                                    \
        VMDIR_SF_INIT(.status, VDIR_CFG_ATTR_INDEX_ENABLED),             \
        VMDIR_SF_INIT(.pszAttrName,  ATTR_VMWITUSERGUID),                \
        VMDIR_SF_INIT(.iTypes, INDEX_TYPE_EQUALITY),                     \
        VMDIR_SF_INIT(.bIsUnique, TRUE),                                 \
        VMDIR_SF_INIT(.bIsNumeric, FALSE),                               \
        VMDIR_SF_INIT(.iId, 0)                                           \
    },                                                                   \
    {                                                                    \
        VMDIR_SF_INIT(.status, VDIR_CFG_ATTR_INDEX_ENABLED),             \
        VMDIR_SF_INIT(.pszAttrName, ATTR_OBJECT_GUID),                   \
        VMDIR_SF_INIT(.iTypes, INDEX_TYPE_EQUALITY),                     \
        VMDIR_SF_INIT(.bIsUnique, TRUE),                                 \
        VMDIR_SF_INIT(.bIsNumeric, FALSE),                               \
        VMDIR_SF_INIT(.iId, 0)                                           \
    },                                                                   \
    {                                                                    \
        VMDIR_SF_INIT(.status, VDIR_CFG_ATTR_INDEX_ENABLED),             \
        VMDIR_SF_INIT(.pszAttrName, NULL),                               \
        VMDIR_SF_INIT(.iTypes, INDEX_TYPE_EQUALITY),                     \
        VMDIR_SF_INIT(.bIsUnique, FALSE),                                \
        VMDIR_SF_INIT(.bIsNumeric, FALSE),                               \
        VMDIR_SF_INIT(.iId, 0)                                           \
    }                                                                    \
}




