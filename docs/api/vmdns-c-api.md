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
VmDnsOpenServerA(
    PCSTR                   pszNetworkAddress,
    PCSTR                   pszUserName,
    PCSTR                   pszDomain,
    PCSTR                   pszPassword,
    DWORD                   dwFlags,
    PVOID                   pReserved,
    PVMDNS_SERVER_CONTEXT   *pServerContext
    );
```

* pszNetworkAddress    - Server FQDN or IP
* pszUserName          - Lightwave user name
* pszDomain            - Lightwave domain name
* pszPassword          - Lightwave password
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
DNS Zone management APIs help to create/delete/update and list DNS zones.

### VmDnsCreateZoneA
Create a new DNS zone

```C
DWORD
VmDnsCreateZoneA(
    PVMDNS_SERVER_CONTEXT   pServerContext,
    PVMDNS_ZONE_INFO        pZoneInfo
    );
```

* pServerContext - DNS server context obtained using VmDnsOpenServerA
* pZoneInfo - Zone informartion

### VmDnsUpdateZoneA
Updates the DNS zone

```C
DWORD
VmDnsUpdateZoneA(
    PVMDNS_SERVER_CONTEXT   pServerContext,
    PVMDNS_ZONE_INFO        pZoneInfo
    );
```

* pServerContext - DNS server context obtained using VmDnsOpenServerA
* pZoneInfo - Zone informartion

### VmDnsListZoneA
List configured zones in the DNS

```C
DWORD
VmDnsListZoneA(
    PVMDNS_SERVER_CONTEXT   pServerContext,
    PVMDNS_ZONE_INFO_ARRAY *ppZoneInfo
    );
```

* pServerContext - DNS server context obtained using VmDnsOpenServerA
* ppZoneInfo - List of zones

### VmDnsDeleteZoneA
Delete zone from the DNS server

```C
DWORD
VmDnsDeleteZoneA(
    PVMDNS_SERVER_CONTEXT   pServerContext,
    PSTR pszZone
    );
```

* pServerContext - DNS server context obtained using VmDnsOpenServerA
* pszZone - Zone name to delete

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
