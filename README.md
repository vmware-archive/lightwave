VMware Lightwave
================

VMware Lightwave is a software stack geared towards providing identity services
including authentication and authorization for large-scale distributed
infrastructure, applications and containers.

VMware Lightwave consists of the following primary components.

1.  VMware Directory Service (vmdir)

2.  VMware Certificate Authority (vmca)

3.  VMware Authentication Framework Daemon/Service (vmafd)

4.  VMware Secure Token Service (vmware-sts)

Prerequisites
-------------

Lightwave uses several existing open source packages. These include

1.  OpenLDAP

OpenLDAP is used for the LDAP server protocol head and the OpenLDAP Lightning
MDB embedded database is used as the underlying LDAP store

1.  Heimdal Kerberos

The Heimdal Kerberos stack is used as the Kerberos protocol head.

1.  DCE/RPC

DCE/RPC is used as the control infrastructure for configuration of the Lightwave
LDAP directory service

1.  Likewise Open

The Likewise Open stack is used for its service control infrastructure, its
registry infrastructure and its NT Security Descriptor support. Likewise Open
also provides a easy mechanism to provide ssh support for Lightwave clients

The first three packages are co-located within the Lightwave project. The
Likewise Open project is a separate project and needs to be built from a
separate git repository. A binary RPM is also available, please see instructions
below to add the repository.

Source code
-----------

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
git clone ssh://git@github.com/vmware/lightwave.git
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Build
-----

These build instructions are to build Lightwave on VMware's Photon Linux
distribution.

1.  Clone lightwave git repository onto your Photon (Full) installation.

2.  Ensure likewise-open-devel-6.2.*.x86\_64.rpm is installed on your Photon
    system.*

3.  *Run* make\* in [workspace root]

4.  As part of a successful build, the following RPMs should be created in the
    [workspace root]/stage folder

    1.  vmware-event-devel-1.2.0-0.x86\_64.rpm

    2.  vmware-directory-client-1.2.0-0.x86\_64.rpm

    3.  vmware-directory-1.2.0-0.x86\_64.rpm

    4.  vmware-directory-client-devel-1.2.0-0.x86\_64.rpm

    5.  vmware-dns-client-1.2.0-0.x86\_64.rpm

    6.  vmware-dns-1.2.0-0.x86\_64.rpm

    7.  vmware-dns-client-devel-1.2.0-0.x86\_64.rpm

    8.  vmware-afd-client-1.2.0-0.x86\_64.rpm

    9.  vmware-afd-1.2.0-0.x86\_64.rpm

    10. vmware-afd-client-devel-1.2.0-0.x86\_64.rpm

    11. vmware-ca-1.2.0-0.x86\_64.rpm

    12. vmware-ca-client-1.2.0-0.x86\_64.rpm

    13. vmware-ca-client-devel-1.2.0-0.x86\_64.rpm

    14. vmware-sts-1.2.0-0.x86\_64.rpm

    15. vmware-sts-client-1.2.0-0.x86\_64.rpm

    16. vmware-ic-config-1.2.0-0.x86\_64.rpm

    17. vmware-lightwave-clients-1.2.0-0.x86\_64.rpm

    18. vmware-lightwave-server-1.2.0-0.x86\_64.rpm

    19. vmware-sts-1.2.0-0.x86\_64.rpm

    20. vmware-sts-client-1.2.0-0.x86\_64.rpm

Deployment
----------

A Lightwave platform comprises of Lightwave Domain Controllers and Lightwave
Domain Clients.

Pre-built lightwave binaries
----------------------------

Pre-built binaries for Lightwave are available through the following YUM
repositories that can be configured on your Photon deployment.

After the following YUM repositories have been configured, it should be possible
to install the Lightwave Domain Controller and Lightwave Clients using "tdnf
install vmware-lightwave-server" and "tdnf install vmware-lightwave-clients"
respectively.

Note : After configuring the following YUM repositories, please disable the
photon-iso.repo; this is achieved by setting "enabled=0" in
/etc/yum.repos.d/photon-iso.repo.

### Lightwave YUM repository

Create the file "/etc/yum.repos.d/lightwave.repo" with the following contents.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
[lightwave]
name=VMware Lightwave 1.0(x86_64)
baseurl=https://dl.bintray.com/vmware/lightwave
gpgkey=file:///etc/pki/rpm-gpg/RPM-GPG-KEY
gpgcheck=0
enabled=1
skip_if_unavailable=True
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

### Photon Extras YUM repository

Create the file "/etc/yum.repos.d/photon-extras.repo" with the following
contents.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
[photon-extras]
name=VMware Photon Extras 1.0(x86_64)
baseurl=https://dl.bintray.com/vmware/photon_extras
gpgkey=file:///etc/pki/rpm-gpg/RPM-GPG-KEY
gpgcheck=0
enabled=1
skip_if_unavailable=True
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

### Setting up a Lightwave Domain Controller

You must first install the following packages on your Photon instance

1.  vmware-directory-client-1.2.0-0.x86\_64.rpm

2.  vmware-directory-1.2.0-0.x86\_64.rpm

3.  vmware-dns-client-1.2.0-0.x86\_64.rpm

4.  vmware-dns-1.2.0-0.x86\_64.rpm

5.  vmware-afd-client-6.6.2-0.x86\_64.rpm

6.  vmware-afd-1.2.0-0.x86\_64.rpm

7.  vmware-ca-client-1.2.0-0.x86\_64.rpm

8.  vmware-ca-1.2.0-0.x86\_64.rpm

9.  vmware-ic-config-1.2.0-0.x86\_64.rpm

10. vmware-sts-client-1.2.0-0.x86\_64.rpm

11. vmware-sts-1.2.0-0.x86\_64.rpm

Alternately, you can install the vmware-lightwave-server-1.2.0-0.x86\_64.rpm
which is a meta RPM with dependencies on all the above RPMs.

If using the YUM repositories for the pre-built binaries, install the Lightwave
Domain Controller using "tdnf install vmware-lightwave-server".

#### Instantiating a domain controller

##### Standalone mode (this is the first replica in a new domain)

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/opt/vmware/bin/configure-lightwave-server --password <password>
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

##### Partner mode (this is a new replica in an existing domain)

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/opt/vmware/bin/configure-lightwave-server --password <password> \
 --server <hostname or ip-address of partner instance>
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Notes:

1.  The password specified for the domain administrator must be at least 8
    characters, include an upper case letter, a lower case letter, a digit and a
    special character.

2.  Make sure to assign a static ip address or a dhcp-address with a reservation
    to the system before promoting it to be a domain controller.

### Setting up a Lightwave Domain Client

The following packages are required to join the Photon system to the Lightwave
Domain.

1.  vmware-directory-client-1.2.0-0.x86\_64.rpm

2.  vmware-dns-client-1.2.0-0.x86\_64.rpm

3.  vmware-afd-client-1.2.0-0.x86\_64.rpm

4.  vmware-afd-1.2.0-0.x86\_64.rpm

5.  vmware-ca-client-1.2.0-0.x86\_64.rpm

6.  vmware-sts-client-1.2.0-0.x86\_64.rpm

7.  vmware-ic-config-1.2.0-0.x86\_64.rpm

Alternately, you can install the vmware-lightwave-clients-1.2.0-0.x86\_64.rpm
which is a meta RPM with dependencies on all the above RPMs.

If using the YUM repositories for the pre-built binaries, install the Lightwave
Domain Client using "tdnf install vmware-lightwave-clients".

#### Joining a system to a Lightwave domain

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/opt/vmware/bin/domainjoin join <domain hostname>
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
