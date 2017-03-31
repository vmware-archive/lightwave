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

#define ATTRIBUTETYPS_TAG           "attributetypes:"
#define ATTRIBUTETYPS_TAG_LEN       sizeof(ATTRIBUTETYPS_TAG) - 1
#define IS_ATTRIBUTETYPES_TAG(tag)  \
    (VmDirStringNCompareA(tag, ATTRIBUTETYPS_TAG,  ATTRIBUTETYPS_TAG_LEN, FALSE) == 0)

#define OBJECTCLASSES_TAG           "objectclasses:"
#define OBJECTCLASSES_TAG_LEN       sizeof(OBJECTCLASSES_TAG) - 1
#define IS_OBJECTCLASSES_TAG(tag)   \
    (VmDirStringNCompareA(tag, OBJECTCLASSES_TAG, OBJECTCLASSES_TAG_LEN, FALSE) == 0)

#define CONTENTRULES_TAG            "ditcontentrules:"
#define CONTENTRULES_TAG_LEN        sizeof(CONTENTRULES_TAG) - 1
#define IS_CONTENTRULES_TAG(tag)    \
    (VmDirStringNCompareA(tag, CONTENTRULES_TAG, CONTENTRULES_TAG_LEN, FALSE) == 0)

#define STRUCTURERULES_TAG          "ditstructurerules:"
#define STRUCTURERULES_TAG_LEN      sizeof(STRUCTURERULES_TAG) - 1
#define IS_STRUCTURERULES_TAG(tag)  \
    (VmDirStringNCompareA(tag, STRUCTURERULES_TAG, STRUCTURERULES_TAG_LEN, FALSE) == 0)

#define NAMEFORM_TAG                "nameforms:"
#define NAMEFORM_TAG_LEN            sizeof(NAMEFORM_TAG) - 1
#define IS_NAMEFORM_TAG(tag)        \
    (VmDirStringNCompareA(tag, NAMEFORM_TAG, NAMEFORM_TAG_LEN, FALSE) == 0)
