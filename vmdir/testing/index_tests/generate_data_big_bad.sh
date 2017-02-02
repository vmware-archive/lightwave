#!/bin/sh

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
    
    echo "dn: cn=xxx-$i,ou=org-$o,dc=vmware,dc=com"
    echo "changetype: add"
    echo "cn: xxx-$i"
    echo "sn: yyy"
    echo "sAMAccountName: xxxyyy-$i"
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
