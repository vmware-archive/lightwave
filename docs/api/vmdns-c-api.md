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
* ppZoneInfo - VMDNS_ZONE_INFO_ARRAY structure contains List of zones

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
Free Zone info obtained from list zones

```C
VOID
VmDnsFreeZoneInfo(
    PVMDNS_ZONE_INFO        pZoneInfo
    );
```

* pZoneInfo - Zone info obtained using VmDnsListZoneA

### VmDnsFreeZoneInfoArray
Free Zone info array obtained from list zones

```C
VOID
VmDnsFreeZoneInfoArray(
    PVMDNS_ZONE_INFO_ARRAY  pZoneInfoArray
    );
```

* pZoneInfoArray - Zone info array obtained using VmDnsListZoneA

## Forwarder Management APIs
Forwarder management API used to manageme add/remove/list external DNSs

If there is no authorized zone exists in VMware DNS, DNS forwards the requests to the configured extenrnal DNS one by one

### VmDnsAddForwarderA
Add new forwarder to VMware DNS.

```C
DWORD
VmDnsAddForwarderA(
    PVMDNS_SERVER_CONTEXT   pServerContext,
    PSTR                    pszForwarder
    );
```

* pServerContext - DNS server context obtained using VmDnsOpenServerA
* pszForwarder  - IP address of the external DNS

### VmDnsDeleteForwarderA
Deletes a configured forwarder form VMwareDNS

```C
DWORD
VmDnsDeleteForwarderA(
    PVMDNS_SERVER_CONTEXT   pServerContext,
    PSTR                    pszForwarder
    );
```
* pServerContext - DNS server context obtained using VmDnsOpenServerA
* pszForwarder  - IP address of the external DNS to delete

### VmDnsGetForwardersA
Get the list of forwarders from DNS server

```C
DWORD
VmDnsGetForwardersA(
    PVMDNS_SERVER_CONTEXT   pServerContext,
    PVMDNS_FORWARDERS*      ppForwarders
    );
```

* pServerContext - DNS server context obtained using VmDnsOpenServerA
* ppForwarders  - List of forwarders in the DNS server

### VmDnsFreeForwarders
Free list of forwarders returned from VmDnsGetForwardersA

```C
VOID
VmDnsFreeForwarders(
    PVMDNS_FORWARDERS       pForwarders
    );
```
* pForwarders  - List of forwarders in the DNS server

## Resource Record Management APIs
Add/Remove/Update resource records in the DNS

### VmDnsAddRecordA
Add new A record to a DNS zone

```C
DWORD
VmDnsAddRecordA(
    PVMDNS_SERVER_CONTEXT   pServerContext,
    PSTR                    pszZone,
    PVMDNS_RECORD           pRecord
    );
```

* pServerContext - DNS server context obtained using VmDnsOpenServerA
* pszZone - zone in which resource record needs to be added
* pRecord - Resource record needs to be added

### VmDnsDeleteRecordA
Delete reource record from a DNS zone

```C
DWORD
VmDnsDeleteRecordA(
    PVMDNS_SERVER_CONTEXT   pServerContext,
    PSTR                    pszZone,
    PVMDNS_RECORD           pRecord
    );
```
* pServerContext - DNS server context obtained using VmDnsOpenServerA
* pszZone - zone in which resource record needs to be added
* pRecord - Resource record needs to be added

### VmDnsQueryRecordsA
Query DNS server for records

```C
DWORD
VmDnsQueryRecordsA(
    PVMDNS_SERVER_CONTEXT   pServerContext,
    PSTR                    pszZone,
    PSTR                    pszName,
    VMDNS_RR_TYPE           dwType,
    DWORD                   dwOptions,
    PVMDNS_RECORD_ARRAY *   ppRecordArray
    );
```

* pServerContext - DNS server context obtained using VmDnsOpenServerA
* pszZone - zone in which resource record needs to be added
* pszName - Name needs to be queried
* dwType - RR type needs to queried (refer RFC)
* dwOptions - Not used
* ppRecordArray - Resource record list returned

### VmDnsListRecordsA
List DNS RR records in from a give zone

```C
DWORD
VmDnsListRecordsA(
    PVMDNS_SERVER_CONTEXT   pServerContext,
    PSTR                    pszZone,
    PVMDNS_RECORD_ARRAY *   ppRecordArray
    );
```

* pServerContext - DNS server context obtained using VmDnsOpenServerA
* pszZone - zone in which resource record needs to be added
* ppRecordArray - Resource record list returned

### VmDnsFreeRecordArray
Free RR record list obtained

```C
VOID
VmDnsFreeRecordArray(
    PVMDNS_RECORD_ARRAY     pRecordArray
    );
```
* ppRecordArray - Resource record list returned
