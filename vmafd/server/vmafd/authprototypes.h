/*
 * Copyright (C) 2014 VMware, Inc. All rights reserved.
 *
 * Module   : authprototypes.h
 *
 * Abstract :
 *
 */
#pragma once

typedef enum{

  VMW_IPC_MODIFY_PERMISSIONS_UNKNOWN = 0,
  VMW_IPC_MODIFY_PERMISSIONS_SET,
  VMW_IPC_MODIFY_PERMISSIONS_ADD,
  VMW_IPC_MODIFY_PERMISSIONS_REVOKE

} VMW_IPC_MODIFY_PERMISSIONS;

DWORD
VmAfdInitializeSecurityDescriptor (
        PVM_AFD_SECURITY_CONTEXT pSecurityContext,
        DWORD dwRevision,
        PVMAFD_SECURITY_DESCRIPTOR *ppSecurityDescriptor
        );

DWORD
VmAfdAccessCheckWithHandle (
       PVECS_SRV_STORE_HANDLE pStore,
       PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
       DWORD dwDesiredAccess
        );

DWORD
VmAfdAccessCheck (
       PVECS_SERV_STORE pStore,
       PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
       DWORD dwDesiredAccess
        );

DWORD
VmAfdCheckOwnerShip (
      PVECS_SERV_STORE pStore,
      PVM_AFD_SECURITY_CONTEXT pSecurityContext
     );


DWORD
VmAfdCheckOwnerShipWithHandle (
      PVECS_SRV_STORE_HANDLE pStore,
      PVM_AFD_CONNECTION_CONTEXT pConnectionContext
     );


DWORD
VmAfdSetSecurityDescriptor (
       PVECS_SERV_STORE pStore,
       PVMAFD_SECURITY_DESCRIPTOR pSecurityDescriptor
       );

DWORD
VmAfdGetSecurityDescriptor (
      PVECS_SERV_STORE pStore,
      PVMAFD_SECURITY_DESCRIPTOR *ppSecurityDescriptor
      );

DWORD
VmAfdModifyPermissions (
      PVECS_SERV_STORE pStore,
      PCWSTR pszServiceName,
      DWORD accessMask,
      VMAFD_ACE_TYPE aceType,
      PVMAFD_SECURITY_DESCRIPTOR pSecurityDescriptor,
      VMW_IPC_MODIFY_PERMISSIONS dwModifyType
      );

DWORD
VmAfdModifyOwner (
      PVECS_SERV_STORE pStore,
      PCWSTR pszServiceName,
      PVMAFD_SECURITY_DESCRIPTOR pSecurityDescriptor
      );

DWORD
VmAfdCopySecurityDescriptor (
                             PVMAFD_SECURITY_DESCRIPTOR pSecurityDescriptorSrc,
                             PVMAFD_SECURITY_DESCRIPTOR *ppSecurityDescriptorDest
                            );
