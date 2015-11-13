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
 * Module Name:
 *
 *        structs.h
 *
 * Abstract:
 *
 *        Identity Manager - Local O/S Identity Provider
 *
 *        Structure definitions 
 *
 * Authors: Sriram Nambakam (snambakam@vmware.com)
 *
 */

typedef enum
{
    ENUMERATION_TYPE_UNKNOWN = 0,
    ENUMERATION_TYPE_USER,
    ENUMERATION_TYPE_GROUP
} ENUMERATION_TYPE;

typedef struct _AUTHCONTEXT
{
    ENUMERATION_TYPE enumType;

} AUTHCONTEXT;

typedef struct _MEMBERSHIP_INFO
{
    PGROUPINFO pGroupInfo;

    struct _MEMBERSHIP_INFO* pNext;

} MEMBERSHIP_INFO, *PMEMBERSHIP_INFO;
