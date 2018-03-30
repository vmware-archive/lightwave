# Lightwave DNS API Documentation
Lightwave DNS provides C api to manage resource records, forwarder and zones.

## Header file

Include file `vmdns.h` in your source file:

```C
#include <vmdns.h>
```

On your linker command, add vmdnsclient as a dependency library, e.g. `-vmdnsclient` for gcc.

## Connection Management APIs

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

* pwszNetworkAddress

### VmDnsCloseServer

## Zone Management APIs

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
