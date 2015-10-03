# Building Lightwave

VMware Lightwave is a software stack geared towards providing identity services including authentication and authorization for large-scale distributed infrastructure, applications and containers.

VMware Lightwave consists of the following primary components.

1. VMware Directory Service (vmdir)
2. VMware Certificate Authority (vmca)
3. VMware Authentication Framework Daemon/Service (vmafd)
4. VMware Secure Token Service (vmware-sts)

## Prerequisites
Lightwave uses several existing open source packages. These include

1. OpenLDAP

   <pre-wrap>
   OpenLDAP is used for the LDAP server protocol head and the OpenLDAP Lightning MDB embedded database is used as the underlying LDAP store
   </pre-wrap>

2. Heimdal Kerberos

   <pre-wrap>
   The Heimdal Kerberos stack is used as the Kerberos protocol head.
   </pre-wrap>

3. DCE/RPC

   <pre-wrap>
   DCE/RPC is used as the control infrastructure for configuration of the Lightwave LDAP directory service
   </pre-wrap>

4. Likewise Open

   <pre-wrap>
   The Likewise Open stack is used for its service control infrastructure, its registry infrastructure and its NT Security Descriptor support. Likewise Open also provides a easy mechanism to provide ssh support for Lightwave clients
   </pre-wrap>

The first three packages are co-located within the Lightwave project. The Likewise Open project is a separate project and needs to be built from a [separate git repository](https://github.com/vmware/likewise-open).  A binary RPM is also available, please see instructions below to add the repository.

## Source code

```
git clone ssh://git@github.com/vmware/lightwave.git
```

## Build

These build instructions are to build Lightwave on VMware's [Photon](htttps://github.com/vmware/photon) Linux distribution.
 1. Clone lightwave git repository onto your Photon (Full) installation.
 2. Ensure `likewise-open-devel-6.2.*.x86_64.rpm` is installed on your Photon system.
 3. Run `make` in `[workspace root]`
 4. As part of a successful build, the following RPMs should be created in the `[workspace root]/stage` folder
```
vmware-directory-6.0.0-0.x86_64.rpm
vmware-directory-client-6.0.0-0.x86_64.rpm
vmware-directory-client-devel-6.0.0-0.x86_64.rpm
vmware-afd-6.0.0-0.x86_64.rpm
vmware-afd-client-2.0.0-0.x86_64.rpm
vmware-afd-client-devel-2.0.0-0.x86_64.rpm
vmware-ca-6.0.0-0.x86_64.rpm
vmware-ca-client-6.0.0-0.x86_64.rpm
vmware-ca-client-devel-6.0.0-0.x86_64.rpm
vmware-ic-config-1.0.0-0.x86_64.rpm
vmware-lightwave-server-6.0.0-0.x86_64.rpm
vmware-lightwave-clients-6.0.0-0.x86_64.rpm
```

## Deployment

A Lightwave platform comprises of Lightwave Domain Controllers and Lightwave Domain Clients.

## Pre-built lightwave binaries

Pre-built binaries for Lightwave are available through the following YUM repositories that can be configured on your Photon deployment.

After the following YUM repositories have been configured, it should be possible to install the Lightwave Domain Controller and Lightwave Clients using `tdnf install vmware-lightwave-server` and `tdnf install vmware-lightwave-clients` respectively.

Note : After configuring the following YUM repositories, please disable the photon-iso.repo; this is achieved by setting `enabled=0` in `/etc/yum.repos.d/photon-iso.repo`.

### Lightwave YUM repository

Create the file `/etc/yum.repos.d/lightwave.repo` with the following contents.

```
[lightwave]
name=VMware Lightwave 1.0(x86_64)
baseurl=https://dl.bintray.com/vmware/lightwave
gpgkey=file:///etc/pki/rpm-gpg/RPM-GPG-KEY
gpgcheck=0
enabled=1
skip_if_unavailable=True
```

### Photon Extras YUM repository

Create the file `/etc/yum.repos.d/photon-extras.repo` with the following contents.

```
[photon-extras]
name=VMware Photon Extras 1.0(x86_64)
baseurl=https://dl.bintray.com/vmware/photon_extras
gpgkey=file:///etc/pki/rpm-gpg/RPM-GPG-KEY
gpgcheck=0
enabled=1
skip_if_unavailable=True
```

### Setting up a Lightwave Domain Controller
You must first install the following packages on your Photon instance

1. `vmware-directory-client-6.0.0-0.x86_64.rpm`
2. `vmware-directory-6.0.0-0.x86_64.rpm`
3. `vmware-afd-client-6.0.0-0.x86_64.rpm`
4. `vmware-afd-6.0.0-0.x86_64.rpm`
5. `vmware-ca-client-6.0.0-0.x86_64.rpm`
6. `vmware-ca-6.0.0-0.x86_64.rpm`
7. `vmware-ic-config-1.0.0-0.x86_64.rpm`

Alternately, you can install the `vmware-lightwave-server-6.0.0-0.x86_64.rpm` which is a meta RPM with dependencies on all the above RPMs.

If using the YUM repositories for the pre-built binaries, install the Lightwave Domain Controller using `tdnf install vmware-lightwave-server`.
