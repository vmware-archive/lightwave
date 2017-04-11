#!/bin/sh

echo "dn: ou=org-A,dc=vmware,dc=com"
echo "changetype: add"
echo "ou: org-A"
echo "objectclass: top"
echo "objectclass: organizationalUnit"
echo ""

echo "dn: ou=org-B,dc=vmware,dc=com"
echo "changetype: add"
echo "ou: org-B"
echo "objectclass: top"
echo "objectclass: organizationalUnit"
echo ""

echo "dn: ou=org-C,dc=vmware,dc=com"
echo "changetype: add"
echo "ou: org-C"
echo "objectclass: top"
echo "objectclass: organizationalUnit"
echo ""

for i in {1..100000}
do
    m=$((i%3))
    if [ $m -eq 1 ]
    then
        o="A"
    elif [ $m -eq 2 ]
    then
        o="B"
    else
        o="C"
    fi
    
    echo "dn: cn=John-$i,ou=org-$o,dc=vmware,dc=com"
    echo "changetype: add"
    echo "cn: John-$i"
    echo "sn: Smith"
    echo "sAMAccountName: johnsmith-$i"
    echo "uid: $i"
    echo "objectClass: user"
    echo "objectClass: organizationalPerson"
    echo "userPassword: PaWorD@123"
    echo "telephoneNumber: 425-123-456-$i"
    echo "facsimileTelephoneNumber: 425-456-123-$i"
    echo "street: 20517 NE $i th street"
    echo "st: washington"
    echo "l: redmond"
    echo "postalCode: 98052"
    echo "ou: org-$o"
    echo "title: engineer"
    echo "description: Employee of VMware"
    echo ""
done
