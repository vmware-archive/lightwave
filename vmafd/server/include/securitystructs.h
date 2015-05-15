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




#define VECS_MAXIMUM_ALLOWED_MASK 0xC0000000

typedef enum
{
    VMAFD_UPDATE_STATUS_UNCHANGED = 0,
    VMAFD_UPDATE_STATUS_MODIFIED,
    VMAFD_UPDATE_STATUS_NEW
} VMAFD_UPDATE_STATUS;


typedef struct _VMAFD_ACE
{
    VMAFD_ACE_TYPE type;
    VMAFD_UPDATE_STATUS changeStatus;
    UINT32 accessMask;
    PVM_AFD_SECURITY_CONTEXT pSecurityContext;
}VMAFD_ACE, *PVMAFD_ACE;

typedef struct _VMAFD_ACE_LIST
{
    VMAFD_ACE Ace;
    struct _VMAFD_ACE_LIST *pNext;
}VMAFD_ACE_LIST, *PVMAFD_ACE_LIST;

typedef struct _VMAFD_ACL
{
    DWORD dwAceCount;
    PVMAFD_ACE_LIST pAceList;
}VMAFD_ACL, *PVMAFD_ACL;

typedef struct _VMAFD_SECURITY_DESCRIPTOR
{
    DWORD dwRevision;
    VMAFD_UPDATE_STATUS changeStatus;
    PVM_AFD_SECURITY_CONTEXT pOwnerSecurityContext;
    PVMAFD_ACL pAcl;
}VMAFD_SECURITY_DESCRIPTOR, *PVMAFD_SECURITY_DESCRIPTOR;

