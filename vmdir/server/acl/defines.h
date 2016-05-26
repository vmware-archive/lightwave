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
#define MAX_RID_SEQUENCE 0xFFFFFFFE

#define MAX_COUNT_PRIOR_WRITE 100
#define VMDIR_DOMAIN_SID_GEN_HASH_TABLE_SIZE 1000

#define ORGANIZATION_SUB_AUTHORITY_NUMBER 5

#define SECURITY_VMWARE_AUTHORITY { 0, 0, 0, 0, 0, 7 }
#define SECURITY_SUBAUTHORITY_ORGANIZATION 21

/*
 * Default starting allocation size for security descriptors (will grow as
 * needed).
 */
#define VMDIR_DEFAULT_SD_RELATIVE_SIZE     512

/*
 * RIDs below this are for well-known / pre-defined users.
 */
#define VMDIR_ACL_RID_BASE 1000
