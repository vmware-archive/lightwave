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
 * Module Name: Directory Access Control
 *
 * Filename: defines.h
 *
 * Abstract:
 *
 * Directory Access Control module
 *
 * Definitions
 *
 */

#define MIN_NODE_SEQUENCE 0
#define MAX_NODE_SEQUENCE 255  // 2^8-1

#define MIN_RID_SEQUENCE 0
#define MAX_RID_SEQUENCE 16777215 // 2^24-1

#define MAX_COUNT_PRIOR_WRITE 100
#define VMDIR_DOMAIN_SID_GEN_HASH_TABLE_SIZE 1000

#define ORGANIZATION_SUB_AUTHORITY_NUMBER 5

#define SECURITY_VMWARE_AUTHORITY { 0, 0, 0, 0, 0, 7 }
#define SECURITY_SUBAUTHORITY_ORGANIZATION 21

#define VMDIR_ACL_MIN(m, n) (((m) < (n)) ? (m) : (n))
#define VMDIR_ACL_MAX(m, n) (((m) > (n)) ? (m) : (n))

