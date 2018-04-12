#!/bin/sh

cat <<NNNN> /var/tmp/computers-container.ldif
version: 1
dn: CN=Computers,DC=lightwave,DC=local
changetype: add
objectClass: top
objectClass: container
distinguishedName: CN=Computers,DC=lightwave,DC=local
cn: Computers
description: Default container for upgraded computer accounts
instanceType: 4
name: Computers
systemFlags: -1946157056
isCriticalSystemObject: TRUE
NNNN

ldapmodify -Y GSSAPI -a -f /var/tmp/computers-container.ldif
