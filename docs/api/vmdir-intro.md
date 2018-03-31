# VMware Directory Service (Vmdir) Overview

VMware Directory Service is a multi-master directory service.
The multi-master replication protocol allows updates to be made on any servers in the directory topology, which eventually reach all the servers.
Vmdir uses several existing open source packages. These include

1. OpenLDAP

OpenLDAP is used for the LDAP server protocol head and the OpenLDAP Lightning
MDB embedded database is used as the underlying LDAP store

2. Heimdal Kerberos

The Heimdal Kerberos stack is used as the Kerberos protocol head.

3. DCE/RPC

DCE/RPC is used as the control infrastructure for configuration of the Lightwave
LDAP directory service

4.  Likewise Open

The Likewise Open stack is used for its service control infrastructure, its registry infrastructure and its NT Security Descriptor support. Likewise Open also provides a easy mechanism to provide ssh support for Lightwave clients