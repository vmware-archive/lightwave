What is Lightwave
================

Project Lightwave is an open source project comprised of enterprise-grade, identity and access management services targeting critical security, governance, and compliance challenges for Cloud-Native Apps within the enterprise. Through integration with Project Photon, Project Lightwave can provide security and governance for container workloads. Project Lightwave can also serve a variety of use cases such as single sign-on, authentication, authorization and certificate authority, as well as certificate key management services across the entire infrastructure and application stack. Project Lightwave is based on a production quality code and an enterprise-grade architecture that is multi-tenant, scalable, and highly available multi master replication topology.

Project Lightwave is made up of the following key identity infrastructure elements:

  * Lightwave Directory Service - standards based, multi-tenant, multi-master, highly scalable LDAP v3 directory service enables an enterprise’s infrastructure to be used by the most-demanding applications as well as by multiple teams.
  * Lightwave Certificate Authority - directory integrated certificate authority helps to simplify certificate-based operations and key management across the infrastructure.
  * Lightwave Certificate Store - endpoint certificate store to store certificate credentials.
  * Lightwave Authentication Services - cloud authentication services with support for Kerberos, OAuth 2.0/OpenID Connect, SAML and WSTrust enable interoperability with other standards-based technologies in the data center.
  * Lightwave Domain Name Services - directory integrated domain name service to ensure Kerberos Authentication to the Directory Service and Authentication Service (STS). It also support for site-affinity using SRV records as well as DNS Forwarders.

Dependencies
-------------

Lightwave uses following existing open source packages.

1.  OpenLDAP

OpenLDAP is used for the LDAP server protocol head and the OpenLDAP Lightning MDB embedded database is used as the underlying LDAP store

1.  Heimdal Kerberos

The Heimdal Kerberos stack is used as the Kerberos protocol head.

1.  DCE/RPC

DCE/RPC is used as the control infrastructure for configuration of the Lightwave LDAP directory service

1.  Likewise Open

The Likewise Open stack is used for its service control infrastructure, its registry infrastructure and its NT Security Descriptor support. Likewise Open
also provides a easy mechanism to provide ssh support for Lightwave clients The first three packages are co-located within the Lightwave project. The
Likewise Open project is a separate project and needs to be built from a separate git repository. A binary RPM is also available, please see instructions
below to add the repository.

Source code
-----------

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
git clone ssh://git@github.com/vmware/lightwave.git
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Build
-----

These build instructions are to build Lightwave on VMware's Photon Linux
distribution. (See wiki for building on other platforms)

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

    5. lightwave-mutentca-1.3.0-0.x86\_64.rpm

    6. lightwave-server-1.3.0-0.x86\_64.rpm

Deployment
----------

A Lightwave platform comprises of Lightwave Domain Controllers and Lightwave
Domain Clients.

Pre-built lightwave binaries
----------------------------

Pre-built binaries for Lightwave are available through the following YUM repositories that can be configured on your Photon deployment.

After the following YUM repositories have been configured, it should be possible to install the Lightwave Domain Controller and Lightwave Clients using "tdnf
install vmware-lightwave-server" and "tdnf install vmware-lightwave-clients" respectively.

Note : After configuring the following YUM repositories, please disable the photon-iso.repo; this is achieved by setting "enabled=0" in
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
*NOTE* You should replace the variables with appropriate values as they may not be set by default on a fresh PhotonOS install.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
echo -e "127.0.0.1\tlocalhost" > /etc/hosts
echo -e "${VM_IP}\t${VM_HOSTNAME}.${VM_DOMAIN_NAME} ${VM_HOSTNAME}" >> /etc/hosts
hostnamectl set-hostname --static ${VM_HOSTNAME}
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

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

## Contributing

You are invited to contribute new features, fixes, or updates, large or small; we are always thrilled to receive pull
requests, and do our best to process them as fast as we can. If you wish to contribute code and you have not signed our
contributor license agreement (CLA), our bot will update the issue when you open a [Pull
Request](https://help.github.com/articles/creating-a-pull-request). For any questions about the CLA process, please
refer to our [FAQ](https://cla.vmware.com/faq).

Before you start to code, we recommend discussing your plans through a  [GitHub
issue](https://github.com/vmware/lightwave/issues) or discuss it first with the official project
[maintainers](AUTHORS.md) via the [#Lightwave Slack Channel](https://vmwarecode.slack.com/messages/CCNLJNZ4M/), especially
for more ambitious contributions. This gives other contributors a chance to point you in the right direction, give you
feedback on your design, and help you find out if someone else is working on the same thing.

## License

Lightwave is available under the [Apache 2 license](LICENSE).

