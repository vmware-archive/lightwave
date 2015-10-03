![Lightwave](https://storage.googleapis.com/project-lightwave/vmw-logo-lightwave.svg "VMware Lightwave")

VMware Lightwave: Security for Container Ecosystem
==================================================

Project Lightwave is an open source project comprised of enterprise-grade, identity and access management services targeting critical security, governance, and compliance challenges for Cloud-Native Apps within the enterprise.

Through integration with [Project Photon](https://github.com/vmware/photon), Project Lightwave can provide security and governance for container workloads. Project Lightwave can also serve a variety of use cases such as single sign-on, authentication, authorization and certificate authority, as well as certificate key management services across the entire infrastructure and application stack.

Project Lightwave is based on production quality code and an enterprise-grade architecture that is multi-tenant, scalable, and highly available multi master replication topology.

Project Lightwave is made up of the following key identity infrastructure elements:

- **Lightwave Directory Service**: standards based, multi-tenant, multi-master, highly scalable LDAP v3 directory service enables an enterprise’s infrastructure to be used by the most-demanding applications as well as by multiple teams.
- **Lightwave Certificate Authority**: directory integrated certificate authority helps to simplify certificate-based operations and key management across the infrastructure.
- **Lightwave Certificate Store**: endpoint certificate store to store certificate credentials.
- **Lightwave Authentication Services**: cloud authentication services with support for Kerberos, OAuth 2.0/OpenID Connect, SAML and WSTrust enable interoperability with other standards-based technologies in the data center.

## Quickstart

On a running Photon VM, create the file `/etc/yum.repos.d/lightwave.repo` with the following content:

```
[lightwave]
name=VMware Lightwave 1.0(x86_64)
baseurl=https://dl.bintray.com/vmware/lightwave
gpgkey=file:///etc/pki/rpm-gpg/RPM-GPG-KEY
gpgcheck=0
enabled=1
skip_if_unavailable=True
```

Then install the Lightwave Domain Controller using `tdnf install vmware-lightwave-server`.

For up-to-date documentation, see the [Docs](docs/) folder.
