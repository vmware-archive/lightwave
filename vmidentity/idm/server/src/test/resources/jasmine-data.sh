#!/bin/bash
openldapHost=localhost
if [ $# -eq 1 ]; then
   openldapHost=$1
fi
ldapadd -c -h ${openldapHost} -x -D "cn=administrator,dc=ssolabs-openldap,dc=eng,dc=vmware,dc=com" -w 123 -f ut-ldapprovider.ldif
