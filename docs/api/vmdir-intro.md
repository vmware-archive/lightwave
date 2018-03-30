# VMware Directory Service (Vmdir)

## Overview

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

## API(s)
Vmdir offers two kinds of API(s) for directory data access: standard LDAP API and REST API.

### Standard LDAP API and Tools over LDAP protocols
1. LDAP bindings
* Simple Bind over SSL (Secure LDAP or LDAPS)

  Simple Bind is allowed over SSL only (port 636). Here is an example using command-line tool with LDAPS binding:

  ldapdelete -H ldaps://localhost:636 -D "cn=administrator,cn=users,dc=vmware,dc=com" -w 'mypasswd!' 'cn=testuser1,cn=users,dc=vmware,dc=com'

* SASL Bind

The following SASL binding mechanisms are supported through port 389:

  * Anonymous (RFC 4505)
  * Secure Remote Password (SRP)

Here is an example using the command-line tool with SRP binding:
ldapsearch -H "ldap://localhost" -Y SRP -U "adminustrator@cloud.stress" -w 'mypasswd!' -b "cn=cloud,cn=stress" -s base "objectclass=*"

  * Kerberos V5 (RFC 4120)

2. LDAP Operations

Vmdir supports standard base LDAP protocols V3 on the following operations:

* Bind
* Search
* Add
* Delete
* Modify
* Modify DN
* Unbind

LDAP command-line tools are provided with Likewise installation, or you can use other LDAP client packages such as OpenLdap to access vmdir server.
This Oracle document gives examples on using LDAP commands [LDAP Command-Line Tools](https://docs.oracle.com/cd/B10501_01/network.920/a96579/comtools.htm)

Please refer the following LDAP RFC(s) for the standards:

* RFC 4510 - LDAP: Technical Specification Road Map
* RFC 4511 - LDAP: The Protocol
* RFC 4512 - LDAP: Directory Information Models
* RFC 4513 - LDAP: Authentication Methods and Security Mechanisms
* RFC 4514 - LDAP: String Representation of Distinguished Names
* RFC 4515 - LDAP: String Representation of Search Filters
* RFC 4516 - LDAP: Uniform Resource Locator
* RFC 4517 - LDAP: Syntaxes and Matching Rules
* RFC 4518 - LDAP: Internationalized String Preparation
* RFC 4519 - LDAP: Schema for User Applications

3. LDAP Client Library

Vmdir is interoperable with all major LDAP client libraries and languages.
The Oracle document shows examples on using LDAP Client libraries [Oracle Internet Directory Software Developer's Kit](https://docs.oracle.com/cd/B10501_01/network.920/a96577/intro.htm)

The client Libraries that can be used to access the directory service include:

* OpenLDAP Client Libraries
* Novell [LDAP Libraries for C ](https://www.novell.com/developer/ndk/ldap_libraries_for_c.html)
* SUN (Oracle) LDAP C SDK
* Java LDAP SDK
* Other LDAP APIs supporting LDAP version 3

### VmDir REST API and Access Methods
1. HTTP Methods
* GET - Returns a representation of the resource (idempotent)
* PUT - Create a resource, and return a representation of that new resource (idempotent)
* PATCH - Update a resource, and return a representation of the newly modified resource
* DELETE - Delete a resource (idempotent)

2. REST API for LDAP operations

LDAP Operations (/vmdir/ldap):

|Method|Description|Request Representation|Response Representation|Parameters|
|------|-----------|----------------------|-----------------------|----------|
|GET|Search for DSE root entry|None|LDAP Entry Representation array|scope ...|
|PUT|Add an LDAP entry|LDAP Entry Representation|None||	 
|PATCH|Modify an LDAP entry|LDAP Mod Representation array|None|dn|
|DELETE|Delete an LDAP entry|None|None|dn|

3. JSON Representations

The following are JSON Representations of REST API for LDAP operations:

LDAP Entry Representation
```
{
    "dn": "DN",
    "attributes": [
        {
            "type": "attrA",
            "value": [
                "xxx"
            ]
        },
        {
            "type": "attrB",
            "value": [
                "yyy",
                "zzz"
            ]
        }
    ]
}
```
 
LDAP Mod Representation
```
{
    "operation": "add",
    "attribute": {
        "type": "description",
        "value": [
            "hello",
            "world"
        ]
    }
}
```

LDAP Search Response Representation
```
{
    "result_count": 100,
    "result": [
        ...LDAP entries...
    ]
    "paged_results_cookie": "c21d21nd12uhed128aaxx"
}
```

Error Response Representation
```
{
    "error-code": 49,
    "error-message": "invalid crendential"
}
```

4. Advanced

The REST API specification can be found from [Rest API](http://vmware.github.io/lightwave/docs/vmdir-rest-api.html)

## Deployment

Please refer [Lightwave Deployment](https://github.com/vmware/lightwave/tree/dev/docs/api#deployment).
