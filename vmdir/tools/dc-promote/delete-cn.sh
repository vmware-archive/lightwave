#!/bin/sh

if [ -z "$1" ]; then
  echo "usage $0 cn=host,..."
  exit 1
fi
DN="$1"
shift

# Delete machine account for WIN2K8 system
cat <<NNNN> /var/tmp/machacct-delete.ldif
version: 1
dn: $DN
changetype: delete
NNNN
ldapmodify -a -f /var/tmp/machacct-delete.ldif
