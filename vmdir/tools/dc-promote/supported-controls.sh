#!/bin/sh
#
# Add supportedControls present in Samba4, not present in lightwave
#
cat <<NNNN> /var/tmp/supported-control.ldif
version: 1
dn: cn=DSE Root
changetype: modify
add: supportedControl
supportedControl: 1.2.840.113556.1.4.1338
-
add: supportedControl
supportedControl: 1.2.840.113556.1.4.1339
-
add: supportedControl
supportedControl: 1.2.840.113556.1.4.1340
-
add: supportedControl
supportedControl: 1.2.840.113556.1.4.1341
-
add: supportedControl
supportedControl: 1.2.840.113556.1.4.1413
-
add: supportedControl
supportedControl: 1.2.840.113556.1.4.1504
-
add: supportedControl
supportedControl: 1.2.840.113556.1.4.2064
-
add: supportedControl
supportedControl: 1.2.840.113556.1.4.473
-
add: supportedControl
supportedControl: 1.2.840.113556.1.4.528
-
add: supportedControl
supportedControl: 1.2.840.113556.1.4.529
-
add: supportedControl
supportedControl: 1.2.840.113556.1.4.801
-
add: supportedControl
supportedControl: 1.2.840.113556.1.4.805
-
add: supportedControl
supportedControl: 1.2.840.113556.1.4.841
-
add: supportedControl
supportedControl: 2.16.840.1.113730.3.4.9
-
delete: supportedControl
supportedControl: 1.3.6.1.4.1.4203.1.9.1.1
-
delete: supportedControl
supportedControl: 1.3.6.1.4.1.4203.1.9.1.2
-
delete: supportedControl
supportedControl: 1.3.6.1.4.1.4203.1.9.1.3
-
add: supportedCapabilities
supportedCapabilities: 1.2.840.113556.1.4.2080
-
add: supportedCapabilities
supportedCapabilities: 1.2.840.113556.1.4.1935
-
add: supportedCapabilities
supportedCapabilities: 1.2.840.113556.1.4.1791
-
add: supportedCapabilities
supportedCapabilities: 1.2.840.113556.1.4.1670
-
add: supportedCapabilities
supportedCapabilities: 1.2.840.113556.1.4.800
-
add: dnsHostName
dnsHostName: photon-addc2.lightwave.local
-
add: ldapServiceName
ldapServiceName: lightwave.local:adam-win2k8r2-d\$@LIGHTWAVE.LOCAL
-
add: dsServiceName
dsServiceName: CN=NTDS Settings,CN=PHOTON-ADDC2,CN=Servers,CN=Default-First-Site-Name,CN=Sites,CN=Configuration,DC=lightwave,DC=local
NNNN
ldapmodify -Y GSSAPI -a -f /var/tmp/supported-control.ldif
