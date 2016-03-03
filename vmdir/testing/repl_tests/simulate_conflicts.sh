#!/bin/bash

if [ $# -ne 2 ]; then
   echo "Usage: simulate_conflicts.sh <IP address of 1st replica> <IP address of 2nd replica>"
   exit 0
fi

echo "##### Add: Object already exists conflict test case setup"

/opt/likewise/bin/ldapadd -c -h $1 -p 11711 -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 <<EOM
dn: cn=John-101,ou=eng,dc=vmware,dc=com
changetype: add
cn: John-101
sn: Smith
objectClass: person
objectClass: organizationalPerson
telephoneNumber: 425-123-456-1
facsimileTelephoneNumber: 425-456-123-1
street: 20517 NE 1 th street
st: washington
l: redmond
postalCode: 98052
ou: Engineering
title: engineer
description: Employee of VMware
EOM

/opt/likewise/bin/ldapadd -c -h $2 -p 11711 -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 <<EOM
dn: cn=John-101,ou=eng,dc=vmware,dc=com
changetype: add
cn: John-101
sn: Smith
objectClass: person
objectClass: organizationalPerson
telephoneNumber: 425-123-456-1
facsimileTelephoneNumber: 425-456-123-1
street: 20517 NE 1 th street
st: washington
l: redmond
postalCode: 98052
ou: Engineering
title: engineer
description: Employee of VMware
EOM

echo "##### Sleeping for 40 secs ..."
sleep 40

echo "##### Add: Parent object does not exist conflict test case setup"

/opt/likewise/bin/ldapadd -c -h $1 -p 11711 -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 <<EOM
dn: ou=sales,dc=vmware,dc=com
changetype: add
ou: sales
objectclass: top
objectclass: organizationalUnit
EOM

echo "##### Sleeping for 40 secs ..."
sleep 40

/opt/likewise/bin/ldapadd -c -h $1 -p 11711 -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 <<EOM
dn: ou=sales,dc=vmware,dc=com
changetype: delete
EOM

/opt/likewise/bin/ldapadd -c -h $2 -p 11711 -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 <<EOM
dn: cn=John-102,ou=sales,dc=vmware,dc=com
changetype: add
cn: John-102
sn: Smith
objectClass: person
objectClass: organizationalPerson
telephoneNumber: 425-123-456-1
facsimileTelephoneNumber: 425-456-123-1
street: 20517 NE 1 th street
st: washington
l: redmond
postalCode: 98052
ou: Engineering
title: engineer
description: Employee of VMware
EOM

echo "##### Sleeping for 40 secs ..."
sleep 40

echo "##### Delete: Object does not exist conflict test case setup"

/opt/likewise/bin/ldapadd -c -h $1 -p 11711 -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 <<EOM
dn: ou=sales,dc=vmware,dc=com
changetype: add
ou: sales
objectclass: top
objectclass: organizationalUnit
EOM

echo "##### Sleeping for 40 secs ..."
sleep 40

/opt/likewise/bin/ldapadd -c -h $2 -p 11711 -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 <<EOM
dn: cn=John-102,ou=sales,dc=vmware,dc=com
changetype: delete
EOM

echo "##### Sleeping for 40 secs ..."
sleep 40

echo "##### Delete: Object not a leaf-node conflict test case setup"

/opt/likewise/bin/ldapadd -c -h $1 -p 11711 -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 <<EOM
dn: cn=John-103,ou=sales,dc=vmware,dc=com
changetype: add
cn: John-103
sn: Smith
objectClass: person
objectClass: organizationalPerson
telephoneNumber: 425-123-456-1
facsimileTelephoneNumber: 425-456-123-1
street: 20517 NE 1 th street
st: washington
l: redmond
postalCode: 98052
ou: Engineering
title: engineer
description: Employee of VMware
EOM

/opt/likewise/bin/ldapadd -c -h $2 -p 11711 -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 <<EOM
dn: ou=sales,dc=vmware,dc=com
changetype: delete
EOM

echo "##### Sleeping for 40 secs ..."
sleep 40

echo "##### Modify: Object does not exist conflict test case setup"

/opt/likewise/bin/ldapadd -c -h $1 -p 11711 -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 <<EOM
dn: cn=John-101,ou=eng,dc=vmware,dc=com
changetype: delete
EOM

/opt/likewise/bin/ldapadd -c -h $2 -p 11711 -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 <<EOM
dn: cn=John-101,ou=eng,dc=vmware,dc=com
changetype: modify
replace: description
description: new description 2
EOM
echo "##### Modify: For John-99: Replace for higher ServerId wins"

/opt/likewise/bin/ldapadd -c -h $1 -p 11711 -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 <<EOM
dn: cn=John-99,ou=eng,dc=vmware,dc=com
changetype: modify
replace: description
description: new description 192-2
EOM

/opt/likewise/bin/ldapadd -c -h $2 -p 11711 -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 <<EOM
dn: cn=John-99,ou=eng,dc=vmware,dc=com
changetype: modify
replace: description
description: new description 205-2
EOM

echo "##### Modify: For John-98: Replace beats Delete"

/opt/likewise/bin/ldapadd -c -h $1 -p 11711 -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 <<EOM
dn: cn=John-98,ou=eng,dc=vmware,dc=com
changetype: modify
delete: description
EOM

/opt/likewise/bin/ldapadd -c -h $2 -p 11711 -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 <<EOM
dn: cn=John-98,ou=eng,dc=vmware,dc=com
changetype: modify
replace: description
description: new description 205-2
EOM

echo "##### Modify: For John-97: Delete beats replace"

/opt/likewise/bin/ldapadd -c -h $1 -p 11711 -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 <<EOM
dn: cn=John-97,ou=eng,dc=vmware,dc=com
changetype: modify
replace: description
description: new description 192-2
EOM

/opt/likewise/bin/ldapadd -c -h $2 -p 11711 -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 <<EOM
dn: cn=John-97,ou=eng,dc=vmware,dc=com
changetype: modify
delete: description
EOM

echo "##### After simulating all the conflicts, add John-110, John-111 to test that replication continues."

/opt/likewise/bin/ldapadd -c -h $1 -p 11711 -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 <<EOM
dn: cn=John-110,ou=eng,dc=vmware,dc=com
changetype: add
cn: John-110
sn: Smith
objectClass: person
objectClass: organizationalPerson
telephoneNumber: 425-123-456-1
facsimileTelephoneNumber: 425-456-123-1
street: 20517 NE 1 th street
st: washington
l: redmond
postalCode: 98052
ou: Engineering
title: engineer
description: Employee of VMware
EOM

/opt/likewise/bin/ldapadd -c -h $1 -p 11711 -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 <<EOM
dn: cn=John-111,ou=eng,dc=vmware,dc=com
changetype: add
cn: John-111
sn: Smith
objectClass: person
objectClass: organizationalPerson
telephoneNumber: 425-123-456-1
facsimileTelephoneNumber: 425-456-123-1
street: 20517 NE 1 th street
st: washington
l: redmond
postalCode: 98052
ou: Engineering
title: engineer
description: Employee of VMware
EOM
