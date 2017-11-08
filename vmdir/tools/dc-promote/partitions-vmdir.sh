#!/bin/sh
cat <<NNNN> /tmp/partitions-configuraions.ldif
version: 1
dn: CN=Partitions,cn=configuration,DOMAIN_DN
changetype: add
objectClass: top
objectClass: crossRefContainer
cn: Partitions
distinguishedName: CN=Partitions,CN=Configuration,DOMAIN_DN
dSCorePropagationData: 16010101000000.0Z
fSMORoleOwner: CN=DC_NAME.DC_DOMAIN,CN=Servers,CN=Default-First-Site-Name,CN=Sites,CN=Configuration,DOMAIN_DN
instanceType: 4
msDS-Behavior-Version: 4
name: Partitions
objectCategory: CN=Cross-Ref-Container,CN=Schema,CN=Configuration,DOMAIN_DN
showInAdvancedViewOnly: TRUE
systemFlags: -2147483648
whenChanged: 20170629191526.0Z
whenCreated: 20151207212342.0Z

dn: CN=DC_NAME,CN=Partitions,cn=configuration,DOMAIN_DN
changetype: add
objectClass: top
objectClass: crossRef
cn: DC_NAME
distinguishedName: CN=DC_NAME,CN=Partitions,CN=Configuration,DOMAIN_DN
dnsRoot: DC_NAME.DC_DOMAIN
dSCorePropagationData: 16010101000000.0Z
instanceType: 4
msDS-Behavior-Version: 4
name: DC_NAME
nCName: DOMAIN_DN
nETBIOSName: DC_NAME
nTMixedDomain: 0
objectCategory: CN=Cross-Ref,CN=Schema,CN=Configuration,DOMAIN_DN
showInAdvancedViewOnly: TRUE
systemFlags: 3
whenChanged: 20170629191526.0Z
whenCreated: 20151207212342.0Z
NNNN

ldapmodify -a -D cn=administrator,cn=users,DOMAIN_DN -h `hostname -i` -w `cat /var/tmp/promote-pwd.txt` -f /tmp/partitions-configuraions.ldif
