# Lightwave DNS API Documentation
Lightwave DNS provides C api to manage resource records, forwarder and zones.

## Header file

Include file `vmdns.h` in your source file:

```C
#include <vmdns.h>
```

On your linker command, add vmdnsclient as a dependency library, e.g. `-vmdnsclient` for gcc.

## Connection Management APIs
Following APIs helps to establish and close secure sessions to a DNS server

### VmDnsOpenServerA
The VmDnsOpenServerA opens a handle to DNS server

```C
DWORD
VmDnsOpenServerW(
    PCWSTR                  pwszNetworkAddress,
    PCWSTR                  pwszUserName,
    PCWSTR                  pwszDomain,
    PCWSTR                  pwszPassword,
    DWORD                   dwFlags,
    PVOID                   pReserved,
    PVMDNS_SERVER_CONTEXT   *pServerContext
    );
```

* pwszNetworkAddress    - Server FQDN or IP
* pwszUserName          - Lightwave user name
* pwszDomain            - Lightwave domain name
* pwszPassword          - Lightwave password
* dwFlags               - Open Flags (not used)
* pReserved             - Not used
* pServerContext        - handle to the DNS server

### VmDnsCloseServer
VmDnsCloseServer closes the connection handle to a DNS server

```C
VOID
VmDnsCloseServer(
    PVMDNS_SERVER_CONTEXT   pServerContext
    );
```

* pServerContext        - Handle to the remote server.

## Zone Management APIs
DNS Zone management APIs

### VmDnsCreateZoneA
### VmDnsUpdateZoneA
### VmDnsListZoneA
### VmDnsDeleteZoneA
### VmDnsFreeZoneInfo
### VmDnsFreeZoneInfoArray

## Forwarder Management APIs

### VmDnsAddForwarderA
### VmDnsDeleteForwarderA
### VmDnsGetForwardersA
### VmDnsFreeForwarders

## Resource Record Management APIs

### VmDnsAddRecordA
### VmDnsDeleteRecordA
### VmDnsQueryRecordsA
### VmDnsListRecordsA
### VmDnsFreeRecords
### VmDnsFreeRecordArray
