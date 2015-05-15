#pragma once

VOID
VmAfdFreeSecurityDescriptor (
        PVMAFD_SECURITY_DESCRIPTOR pSecurityDescriptor
        );

VOID
VmAfdFreeAceList (
    PVMAFD_ACE_LIST pAceList
    );

VOID
VmAfdFreeAce (
    PVMAFD_ACE pAce
    );

VOID
VmAfdFreeAcl (
    PVMAFD_ACL pAcl
    );
