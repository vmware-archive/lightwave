#!/bin/sh

set -x

host="localhost"
port=389

while getopts h:p: opt;
do
    case $opt in
        h)
            host=$OPTARG
            ;;
        p)
            port=$OPTARG
            ;;
    esac
done

echo "Create host domain: vsphere.local"
/usr/lib/vmware-vmdir/bin/vdcpromo -u Administrator -w vmware -d vsphere.local

echo
echo "Create tenant domain: coke.com"
/usr/lib/vmware-vmdir/bin/vdcpromo -u Administrator -w vmware -d coke.com -t

echo
echo "Create tenant domain: pepsi.com"
/usr/lib/vmware-vmdir/bin/vdcpromo -u Administrator -w vmware -d pepsi.com -t

echo
echo "Creating objects as ** System ** Administrator"
echo
/opt/likewise/bin/ldapadd -c -h $host -p $port -x -D "cn=Administrator,cn=users,dc=vsphere,dc=local" -w vmware <<EOM
dn: cn=John-1,cn=Users,dc=vsphere,dc=local
changetype: add
cn: John-1
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
userPassword: Pasword@1

dn: cn=John-2,cn=Users,dc=vsphere,dc=local
changetype: add
cn: John-1
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
userPassword: Pasword@1

dn: cn=John-1,cn=Users,dc=coke,dc=com
changetype: add
cn: John-1
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
userPassword: Pasword@1

dn: cn=John-2,cn=Users,dc=coke,dc=com
changetype: add
cn: John-1
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
userPassword: Pasword@1

dn: cn=John-1,cn=Users,dc=pepsi,dc=com
changetype: add
cn: John-1
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
userPassword: Pasword@1

dn: cn=John-2,cn=Users,dc=pepsi,dc=com
changetype: add
cn: John-1
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
userPassword: Pasword@1
EOM

echo
echo "Creating objects as ** Coke ** Administrator"
echo
/opt/likewise/bin/ldapadd -c -h $host -p $port -x -D "cn=Administrator,cn=users,dc=coke,dc=com" -w vmware <<EOM
dn: cn=John-3,cn=Users,dc=vsphere,dc=local
changetype: add
cn: John-1
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
userPassword: Pasword@1

dn: cn=John-3,cn=Users,dc=coke,dc=com
changetype: add
cn: John-1
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
userPassword: Pasword@1

dn: cn=John-3,cn=Users,dc=pepsi,dc=com
changetype: add
cn: John-1
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
userPassword: Pasword@1
EOM

echo
echo "Creating objects as ** Pepsi ** Administrator"
echo
/opt/likewise/bin/ldapadd -c -h $host -p $port -x -D "cn=Administrator,cn=users,dc=pepsi,dc=com" -w vmware <<EOM
dn: cn=John-3,cn=Users,dc=vsphere,dc=local
changetype: add
cn: John-1
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
userPassword: Pasword@1

dn: cn=John-3,cn=Users,dc=coke,dc=com
changetype: add
cn: John-1
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
userPassword: Pasword@1

dn: cn=John-3,cn=Users,dc=pepsi,dc=com
changetype: add
cn: John-1
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
userPassword: Pasword@1
EOM

echo
echo "Creating objects as ** Vsphere ** Administrator"
echo
/opt/likewise/bin/ldapadd -c -h $host -p $port -x -D "cn=Administrator,cn=users,dc=vsphere,dc=local" -w vmware <<EOM
dn: cn=John-3,cn=Users,dc=vsphere,dc=local
changetype: add
cn: John-1
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
userPassword: Pasword@1

dn: cn=John-3,cn=Users,dc=coke,dc=com
changetype: add
cn: John-1
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
userPassword: Pasword@1

dn: cn=John-3,cn=Users,dc=pepsi,dc=com
changetype: add
cn: John-1
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
userPassword: Pasword@1
EOM

echo
echo "Creating objects as ** cn=John-1,cn=Users,dc=vsphere,dc=local **"
echo
/opt/likewise/bin/ldapadd -c -h $host -p $port -x -D "cn=John-1,cn=Users,dc=vsphere,dc=local" -w "Pasword@1" <<EOM
dn: cn=John-4,cn=Users,dc=vsphere,dc=local
changetype: add
cn: John-1
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
userPassword: Pasword@1

dn: cn=John-4,cn=Users,dc=coke,dc=com
changetype: add
cn: John-1
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
userPassword: Pasword@1

dn: cn=John-4,cn=Users,dc=pepsi,dc=com
changetype: add
cn: John-1
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
userPassword: Pasword@1
EOM

echo
echo "Creating objects as ** cn=John-1,cn=Users,dc=coke,dc=com **"
echo
/opt/likewise/bin/ldapadd -c -h $host -p $port -x -D "cn=John-1,cn=Users,dc=coke,dc=com" -w "Pasword@1" <<EOM
dn: cn=John-4,cn=Users,dc=vsphere,dc=local
changetype: add
cn: John-1
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
userPassword: Pasword@1

dn: cn=John-4,cn=Users,dc=coke,dc=com
changetype: add
cn: John-1
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
userPassword: Pasword@1

dn: cn=John-4,cn=Users,dc=pepsi,dc=com
changetype: add
cn: John-1
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
userPassword: Pasword@1
EOM

echo
echo "Creating objects as ** cn=John-1,cn=Users,dc=pepsi,dc=com **"
echo
/opt/likewise/bin/ldapadd -c -h $host -p $port -x -D "cn=John-1,cn=Users,dc=pepsi,dc=com" -w "Pasword@1" <<EOM
dn: cn=John-4,cn=Users,dc=vsphere,dc=local
changetype: add
cn: John-1
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
userPassword: Pasword@1

dn: cn=John-4,cn=Users,dc=coke,dc=com
changetype: add
cn: John-1
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
userPassword: Pasword@1

dn: cn=John-4,cn=Users,dc=pepsi,dc=com
changetype: add
cn: John-1
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
userPassword: Pasword@1
EOM

echo
echo "Modifying objects as ** System ** Administrator"
echo
/opt/likewise/bin/ldapadd -c -h $host -p $port -x -D "cn=Administrator,cn=users,dc=vsphere,dc=local" -w vmware <<EOM
dn: cn=John-1,cn=Users,dc=vsphere,dc=local
changetype: modify
replace: description
description: description changed by System

dn: cn=John-1,cn=Users,dc=coke,dc=com
changetype: modify
replace: description
description: description changed by System

dn: cn=John-1,cn=Users,dc=pepsi,dc=com
changetype: modify
replace: description
description: description changed by System
EOM

echo
echo "Modifying objects as ** Coke ** Administrator"
echo
/opt/likewise/bin/ldapadd -c -h $host -p $port -x -D "cn=Administrator,cn=users,dc=Coke,dc=com" -w vmware <<EOM
dn: cn=John-1,cn=Users,dc=vsphere,dc=local
changetype: modify
replace: description
description: description changed by Coke

dn: cn=John-1,cn=Users,dc=coke,dc=com
changetype: modify
replace: description
description: description changed by Coke

dn: cn=John-1,cn=Users,dc=pepsi,dc=com
changetype: modify
replace: description
description: description changed by Coke
EOM

echo
echo "Modifying objects as ** Pepsi ** Administrator"
echo
/opt/likewise/bin/ldapadd -c -h $host -p $port -x -D "cn=Administrator,cn=users,dc=pepsi,dc=com" -w vmware <<EOM
dn: cn=John-1,cn=Users,dc=vsphere,dc=local
changetype: modify
replace: description
description: description changed by pepsi

dn: cn=John-1,cn=Users,dc=coke,dc=com
changetype: modify
replace: description
description: description changed by pepsi

dn: cn=John-1,cn=Users,dc=pepsi,dc=com
changetype: modify
replace: description
description: description changed by pepsi
EOM

echo
echo "Modifying objects as ** cn=John-1,cn=Users,dc=vsphere,dc=local **"
echo
/opt/likewise/bin/ldapadd -c -h $host -p $port -x -D "cn=John-1,cn=Users,dc=vsphere,dc=local" -w "Pasword@1" <<EOM
dn: cn=John-1,cn=Users,dc=vsphere,dc=local
changetype: modify
replace: description
description: description changed by John-1,vsphere

dn: cn=John-1,cn=Users,dc=coke,dc=com
changetype: modify
replace: description
description: description changed by John-1,vsphere

dn: cn=John-1,cn=Users,dc=pepsi,dc=com
changetype: modify
replace: description
description: description changed by John-1,vsphere
EOM

echo
echo "Deleteing objects as ** Pepsi ** Administrator"
echo
/opt/likewise/bin/ldapadd -c -h $host -p $port -x -D "cn=Administrator,cn=users,dc=pepsi,dc=com" -w vmware <<EOM
dn: cn=John-1,cn=Users,dc=vsphere,dc=local
changetype: delete

dn: cn=John-2,cn=Users,dc=vsphere,dc=local
changetype: delete

dn: cn=John-3,cn=Users,dc=vsphere,dc=local
changetype: delete

dn: cn=John-1,cn=Users,dc=coke,dc=com
changetype: delete

dn: cn=John-2,cn=Users,dc=coke,dc=com
changetype: delete

dn: cn=John-3,cn=Users,dc=coke,dc=com
changetype: delete

dn: cn=John-1,cn=Users,dc=pepsi,dc=com
changetype: delete

dn: cn=John-2,cn=Users,dc=pepsi,dc=com
changetype: delete

dn: cn=John-3,cn=Users,dc=pepsi,dc=com
changetype: delete

EOM

echo
echo "Deleteing objects as ** Coke ** Administrator"
echo
/opt/likewise/bin/ldapadd -c -h $host -p $port -x -D "cn=Administrator,cn=users,dc=coke,dc=com" -w vmware <<EOM
dn: cn=John-1,cn=Users,dc=vsphere,dc=local
changetype: delete

dn: cn=John-2,cn=Users,dc=vsphere,dc=local
changetype: delete

dn: cn=John-3,cn=Users,dc=vsphere,dc=local
changetype: delete

dn: cn=John-1,cn=Users,dc=coke,dc=com
changetype: delete

dn: cn=John-2,cn=Users,dc=coke,dc=com
changetype: delete

dn: cn=John-3,cn=Users,dc=coke,dc=com
changetype: delete

dn: cn=John-1,cn=Users,dc=pepsi,dc=com
changetype: delete

dn: cn=John-2,cn=Users,dc=pepsi,dc=com
changetype: delete

dn: cn=John-3,cn=Users,dc=pepsi,dc=com
changetype: delete

EOM

echo
echo "Deleteing objects as ** System ** Administrator"
echo
/opt/likewise/bin/ldapadd -c -h $host -p $port -x -D "cn=Administrator,cn=users,dc=vsphere,dc=local" -w vmware <<EOM
dn: cn=John-1,cn=Users,dc=vsphere,dc=local
changetype: delete

dn: cn=John-2,cn=Users,dc=vsphere,dc=local
changetype: delete

dn: cn=John-3,cn=Users,dc=vsphere,dc=local
changetype: delete

dn: cn=John-1,cn=Users,dc=coke,dc=com
changetype: delete

dn: cn=John-2,cn=Users,dc=coke,dc=com
changetype: delete

dn: cn=John-3,cn=Users,dc=coke,dc=com
changetype: delete

dn: cn=John-1,cn=Users,dc=pepsi,dc=com
changetype: delete

dn: cn=John-2,cn=Users,dc=pepsi,dc=com
changetype: delete

dn: cn=John-3,cn=Users,dc=pepsi,dc=com
changetype: delete

EOM

echo
echo "Search for Admins as ** system administrator ** ..."
echo
/opt/likewise/bin/ldapsearch -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vsphere,dc=local" -w 'vmware' -b "" -s subtree "cn=Admin*" dn

echo
echo "Search for Admins as ** coke administrator ** ..."
echo
/opt/likewise/bin/ldapsearch -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=coke,dc=com" -w 'vmware' -b "" -s subtree "cn=Admin*" dn

echo
echo "Search for Admins as ** pepsi administrator ** ..."
echo
/opt/likewise/bin/ldapsearch -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=pepsi,dc=com" -w 'vmware' -b "" -s subtree "cn=Admin*" dn

echo
echo "Search for Admins as ** John-1,pepsi ** ..."
echo
/opt/likewise/bin/ldapsearch -h $host -p $port -x -D "cn=John-1,cn=Users,dc=pepsi,dc=com" -w 'vmware' -b "" -s subtree "cn=Admin*" dn

echo
echo "Search for Admins as ** Anonymous ** ..."
echo
/opt/likewise/bin/ldapsearch -h $host -p $port -x -b "" -s subtree "cn=Admin*" dn

echo
echo "** Anoymous ** DSE Root entry search ..."
/opt/likewise/bin/ldapsearch -h $host -p $port -x -b "" -s base "objectclass=*"
