# VMware Lightwave

VMware Lightwave is a software stack geared towards providing identity services including authentication and authorization for traditional infrastructure, applications and containers.

VMware Lightwave consists of the following primary components.

1. VMware Directory Service (vmdir) 
2. VMware Certificate Authority (vmca)
3. VMware Authentication Framework Daemon/Service (vmafd)
4. VMware Secure Token Service (vmware-sts)

##Prerequisites
Lightwave uses several existing open source packages. These include
1. OpenLDAP - OpenLDAP is used for the LDAP server protocol head and the OpenLDAP Lightning MDB embedded database is used as the underlying LDAP store
2. Heimdal Kerberos - The Heimdal Kerberos stack is used as the Kerberos protocol head. 
3. DCE/RPC  - DCE/RPC is used as the control infrastructure for configuration of the Lightwave LDAP directory service
4. Likewise Open - The Likewise Open stack is used for its service control infrastructure, its registry infrastructure and its NT Security Descriptor support. Likewise Open also provides a easy mechanism to provide ssh support for Lightwave clients

Most of these packages are  co-located within the Lightwave project. The Likewise Open project however  is a separate project and needs to be built  from a separate git repository

##Source code

```
git clone ssh://git@github.com/vmware/lightwave.git
```

##Build

These build instructions are to build Lightwave on VMware's Photon Linux distribution.
 1. Clone lightwave git repository onto your Photon (Full) installation.
 2. Ensure likewise-open-devel-6.2.*.x86_64.rpm is installed on your Photon system. 
 3. Run *make* in [workspace root]
 4. As part of a successful build, the following RPMs should be created in the [workspace root]/stage folder
     1. vmware-directory-6.0.0-0.x86_64.rpm
     2. vmware-directory-client-6.0.0-0.x86_64.rpm
     3. vmware-directory-client-devel-6.0.0-0.x86_64.rpm
     4. vmware-afd-6.0.0-0.x86_64.rpm
     5. vmware-afd-client-2.0.0-0.x86_64.rpm
     6. vmware-afd-client-devel-2.0.0-0.x86_64.rpm
     7. vmware-ca-6.0.0-0.x86_64.rpm
     8. vmware-ca-client-6.0.0-0.x86_64.rpm
     9. vmware-ca-client-devel-6.0.0-0.x86_64.rpm
     10. vmware-ic-config-1.0.0-0.x86_64.rpm

##Deployment

A Lightwave platform comprises of Lightwave Domain Controllers and Lightwave Domain Clients.


### Setting up a Lightwave Domain Controller
You must first install the  following packages on your Photon instance

1. vmware-directory-client-6.0.0-0.x86_64.rpm
2. vmware-directory-6.0.0-0.x86_64.rpm
3. vmware-afd-client-6.0.0-0.x86_64.rpm
4. vmware-afd-6.0.0-0.x86_64.rpm
5. vmware-ca-client-6.0.0-0.x86_64.rpm
6. vmware-ca-6.0.0-0.x86_64.rpm
7. vmware-ic-config-1.0.0-0.x86_64.rpm

#### Instantiating a domain controller

##### Standalone mode (this is the first replica in a new domain)

```
/opt/vmware/bin/ic-promote
```

##### Partner mode (this is a new replica  in an existing domain)

```
/opt/vmware/bin/ic-promote --partner <hostname or ip-address of partner instance>
```

Notes:

1. The password specified for the domain administrator must be at least 8 characters, include an upper case letter, a lower case letter, a digit and a special character.
2. Make sure to assign a static ip address or a dhcp-address with a reservation to the system before promoting it to be a domain controller.

### Setting up a Lightwave Domain Client

The following packages are required to join the Photon system to the Directory Domain.

1. vmware-directory-client-6.0.0-0.x86_64.rpm
2. vmware-afd-client-6.0.0-0.x86_64.rpm
3. vmware-afd-6.0.0-0.x86_64.rpm
4. vmware-ca-client-6.0.0-0.x86_64.rpm
5. vmware-ic-config-1.0.0-0.x86_64.rpm

#### Joining a system to a Lightwave domain
```
/opt/vmware/bin/ic-join --domain-controller <hostname or ip-address of domain controller>
```
