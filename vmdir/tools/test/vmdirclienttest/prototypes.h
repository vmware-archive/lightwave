#ifndef _VMDIR_CLIENT_TEST_PROTOTYPE_H_
#define _VMDIR_CLIENT_TEST_PROTOTYPE_H_

void
TestVmDirSASLClient(
    void
    );

DWORD
VmDirConnectLDAPServer(
    LDAP**      pLd,
    PCSTR       pszHostName,
    PCSTR       pszDomain,
    PCSTR       pszUserName,
    PCSTR       pszPassword
    );

DWORD
VmDirLdapGetMasterKey(
    LDAP* pLd,
    PCSTR pszDomainDN,
    PBYTE* ppMasterKey,
    DWORD* pLen
    );

#endif
