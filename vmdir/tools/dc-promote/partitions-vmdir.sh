#!/bin/sh
cat <<NNNN> /tmp/partitions-configuraions.ldif
version: 1
dn: CN=Partitions,cn=configuration,dc=lightwave,dc=local
changetype: add
objectClass: top
objectClass: crossRefContainer
cn: Partitions
distinguishedName: CN=Partitions,CN=Configuration,dc=lightwave,dc=local
dSCorePropagationData: 16010101000000.0Z
fSMORoleOwner: CN=photon-102-test.lightwave.local,CN=Servers,CN=Default-First-Site-Name,CN=Sites,CN=Configuration,dc=lightwave,dc=local
instanceType: 4
msDS-Behavior-Version: 4
name: Partitions
objectCategory: CN=Cross-Ref-Container,CN=Schema,CN=Configuration,dc=lightwave,dc=local
showInAdvancedViewOnly: TRUE
systemFlags: -2147483648
whenChanged: 20170629191526.0Z
whenCreated: 20151207212342.0Z

dn: CN=photon-102-test,CN=Partitions,cn=configuration,dc=lightwave,dc=local
changetype: add
objectClass: top
objectClass: crossRef
cn: PHOTON-102-TEST
distinguishedName: CN=PHOTON-102-TEST,CN=Partitions,CN=Configuration,dc=lightwave,dc=local
dnsRoot: photon-102-test.lightwave.local
dSCorePropagationData: 16010101000000.0Z
instanceType: 4
msDS-Behavior-Version: 4
name: PHOTON-102-TEST
nCName: dc=lightwave,dc=local
nETBIOSName: PHOTON-102-TEST
nTMixedDomain: 0
objectCategory: CN=Cross-Ref,CN=Schema,CN=Configuration,dc=lightwave,dc=local
showInAdvancedViewOnly: TRUE
systemFlags: 3
whenChanged: 20170629191526.0Z
whenCreated: 20151207212342.0Z
NNNN

ldapmodify -a -D cn=administrator,cn=users,dc=lightwave,dc=local -h `hostname -i` -w `cat /tmp/promote-pwd.txt` -f /tmp/partitions-configuraions.ldif
