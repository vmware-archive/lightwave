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

3.  *Run* ./build_photon.sh\* in [workspace]/build

4.  As part of a successful build, the following RPMs should be created in the
    [workspace]/build/stage directory

    1. lightwave-1.3.0-0.x86\_64.rpm

    2. lightwave-client-1.3.0-0.x86\_64.rpm

    3. lightwave-devel-1.3.0-0.x86\_64.rpm

    4. lightwave-post-1.3.0-0.x86\_64.rpm

    5. lightwave-server-1.3.0-0.x86\_64.rpm

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

#### For photon 1.0

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
[lightwave]
name=VMware Lightwave 1.0(x86_64)
baseurl=https://dl.bintray.com/vmware/lightwave
gpgkey=file:///etc/pki/rpm-gpg/RPM-GPG-KEY
gpgcheck=0
enabled=1
skip_if_unavailable=True
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#### For photon 2.0

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
[lightwave]
name=VMware Lightwave for Photon 2.0(x86_64)
baseurl=https://vmware.bintray.com/lightwave-dev/photon_2.0/master
gpgcheck=0
enabled=1
skip_if_unavailable=True
EOM
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

### Photon Extras YUM repository

Create the file "/etc/yum.repos.d/photon-extras.repo" with the following
contents.

#### For photon 1.0

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
[photon-extras]
name=VMware Photon Extras 1.0(x86_64)
baseurl=https://dl.bintray.com/vmware/photon_extras
gpgkey=file:///etc/pki/rpm-gpg/RPM-GPG-KEY
gpgcheck=0
enabled=1
skip_if_unavailable=True
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#### For photon 2.0

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
[photon-extras]
name=VMware Photon Extras 2.0(x86_64)
baseurl=https://dl.bintray.com/vmware/photon_extras_$releasever_$basearch
gpgkey=file:///etc/pki/rpm-gpg/VMWARE-RPM-GPG-KEY
gpgcheck=1
enabled=1
skip_if_unavailable=True
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

### Setting up a Lightwave Domain Controller

You must first install the following packages on your Photon instance

1. lightwave-client

2. lightwave-server

3. lightwave

For installing these simply execute the following tdnf commands:

#### For photon 1.0

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
tdnf makecache
tdnf install lightwave-client lightwave lightwave-server
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#### For photon 2.0

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
tdnf makecache
tdnf install lightwave-client-1.3.1 lightwave-1.3.1 lightwave-server-1.3.1
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This should pull all required depencies for lightwave.

#### Instantiating a domain controller

#### Set up the hostname for the instance

This is step is required for every lightwave node. Edit the /etc/hosts file
to look like given below:

echo -e "127.0.0.1\tlocalhost" > /etc/hosts
echo -e "${VM_IP}\t${VM_HOSTNAME}.${VM_DOMAIN_NAME} ${VM_HOSTNAME}" >> /etc/hosts
hostnamectl set-hostname --static ${VM_HOSTNAME}

Note that the VM_DOMAIN_NAME can be chosen for the first server and should be the
same for all partner nodes in a cluster.

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

1. lightwave-client-1.3.0-0.x86\_64.rpm

If using the YUM repositories for the pre-built binaries, install the Lightwave
Domain Client using "tdnf install vmware-lightwave-clients".

#### Joining a system to a Lightwave domain

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/opt/vmware/bin/domainjoin join <domain hostname>
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
