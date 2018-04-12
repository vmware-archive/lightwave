#!/bin/sh
if [ -n "$1" ]; then
  SUFFIX="$1"
fi
cat <<NNNN> /var/tmp/adam-wintest-computer.ldif
version: 1
dn: cn=TEST-WINTEST${SUFFIX},cn=Computers,dc=lightwave,dc=local
changetype: add
objectClass: Computer
userPassword: Testing123@
sAMAccountName: TEST-WINTEST${SUFFIX}
userPrincipalName: TEST-WINTEST${SUFFIX}@LIGHTWAVE.LOCAL
NNNN
ldapmodify -Y GSSAPI -a -f /var/tmp/adam-wintest-computer.ldif
