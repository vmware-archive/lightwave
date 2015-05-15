#!/bin/sh

echo "dn: ou=eng,dc=tenant2,dc=com"
echo "changetype: add"
echo "ou: eng"
echo "objectclass: top"
echo "objectclass: organizationalUnit"
echo ""

for i in {1..10}
do
   echo "dn: cn=John-$i,ou=eng,dc=tenant2,dc=com"
   echo "changetype: add"
   echo "cn: John-$i"
   echo "sn: Smith"
#   echo "givenName: John-$i Smith"
   echo "objectClass: person"
   echo "objectClass: organizationalPerson"
   echo "userPassword: PaWorD@123"
#   echo "mail: john-$i@gmail.com"
   echo "telephoneNumber: 425-123-456-$i"
   echo "facsimileTelephoneNumber: 425-456-123-$i"
   echo "street: 20517 NE $i th street"
   echo "st: washington"
   echo "l: redmond"
   echo "postalCode: 98052"
   echo "ou: Engineering"
   echo "title: engineer"
   echo "description: Employee of VMware"
   echo ""
done

