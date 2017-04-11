#!/bin/sh

echo "dn: ou=eng,dc=vmware,dc=com"
echo "changetype: delete"
echo ""

for i in {1..100}
do
   echo "dn: cn=John-$i,ou=eng,dc=vmware,dc=com"
   echo "changetype: delete"
   echo ""
done

echo "dn: ou=eng,dc=vmware,dc=com"
echo "changetype: delete"
echo ""

