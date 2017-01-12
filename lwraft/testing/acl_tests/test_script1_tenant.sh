#!/bin/bash

REL_DIR=`dirname $0`
cd $REL_DIR

source ../header.sh

VDCPROMO_PATH=/usr/lib/vmware-vmdir/bin

# Setting up tenant named tenant1.com
$VDCPROMO_PATH/vdcpromo -d tenant1.com -u administrator -w 456 -t

# Setting up tenant named tenant2.com
$VDCPROMO_PATH/vdcpromo -d tenant2.com -u administrator -w 456 -t

echo "###############################################################################################"
echo "Adding 10 objects in each tenant (tenant1 and tenant2)"
echo

./generate_data_tenant1.sh > data_tenant1.ldif
./generate_data_tenant2.sh > data_tenant2.ldif

ldapadd -c -h $host -p $port -x -D "cn=administrator,cn=users,dc=tenant1,dc=com" -w 456 -f data_tenant1.ldif
ldapadd -c -h $host -p $port -x -D "cn=administrator,cn=users,dc=tenant2,dc=com" -w 456 -f data_tenant2.ldif


echo
echo "*********************************************************************************************"
echo "*************************Built-in Administrators ACL tests***********************************"
echo "*************************Memberof Built-in Administrators has AdminRole**********************"
echo "*********************************************************************************************"
echo

echo "##############################################################################################"
echo "Add new object in dc=tenant1,dc=com as tenant1\John-1 (should fail)"
echo "##############################################################################################"
echo

ldapadd -c -h $host -p $port -x -D "cn=John-1,ou=eng,dc=tenant1,dc=com" -w "PaWorD@123" <<EOM
dn: cn=John-add,ou=eng,dc=tenant1,dc=com
changetype: add
cn: John-add
sn: Smith
objectClass: person
objectClass: organizationalPerson
telephoneNumber: 425-123-456-91
facsimileTelephoneNumber: 425-456-123-91
street: 20517 NE 91 th street
st: washington
l: redmond
postalCode: 98052
ou: Engineering
title: engineer
description: Employee of VMware
userPassword: PaWorD@123
EOM

echo "Add tenant-1\John-1 to tenant1's built-in administrators group"

ldapmodify -c -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=tenant1,dc=com" -w 456 <<EOM
dn: cn=Administrators,cn=BuiltIn,dc=tenant1,dc=com
changetype: modify
add: member
member: cn=John-1,ou=eng,dc=tenant1,dc=com
EOM

echo "####################################################################################################################"
echo "Add new object in dc=tenant1,dc=com as tenant1\John-1 (John-1 is memberof BuiltIn Administrators, should succeed now"
echo "####################################################################################################################"
echo

ldapadd -c -h $host -p $port -x -D "cn=John-1,ou=eng,dc=tenant1,dc=com" -w "PaWorD@123" <<EOM
dn: cn=John-add,ou=eng,dc=tenant1,dc=com
changetype: add
cn: John-add
sn: Smith
objectClass: person
objectClass: organizationalPerson
telephoneNumber: 425-123-456-91
facsimileTelephoneNumber: 425-456-123-91
street: 20517 NE 91 th street
st: washington
l: redmond
postalCode: 98052
ou: Engineering
title: engineer
description: Employee of VMware
userPassword: PaWorD@123
EOM

ldapsearch -h $host -p $port -x -D "cn=administrator,cn=Users,dc=tenant1,dc=com" -w 456 -b "cn=John-add,ou=eng,dc=tenant1,dc=com" -s base "objectclass=*" dn

echo "################################################################################################"
echo "Delete tenant1 user as tenant1\John-1 -- this should pass (John-1 is memberof Admins)"
echo "################################################################################################"
ldapmodify -c -h $host -p $port -x -D "cn=John-1,ou=eng,dc=tenant1,dc=com" -w 'PaWorD@123'<<EOM
dn: cn=John-add,ou=eng,dc=tenant1,dc=com
changetype: delete
EOM

ldapsearch -h $host -p $port -x -D "cn=administrator,cn=Users,dc=tenant1,dc=com" -w 456 -b "cn=John-add,ou=eng,dc=tenant1,dc=com" -s base "objectclass=*" dn

echo "Remove tenant-1\John-1 from tenant1's built-in administrators group"

ldapmodify -c -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=tenant1,dc=com" -w 456 <<EOM
dn: cn=Administrators,cn=BuiltIn,dc=tenant1,dc=com
changetype: modify
delete: member
member: cn=John-1,ou=eng,dc=tenant1,dc=com
EOM



echo
echo "*********************************************************************************************"
echo "**************************ADD ACL TESTS WITH TENANTS*****************************************"
echo "*********************************************************************************************"
echo

: '
TODO - Uncomment this test case when vmware\administrator privilege is fixed

echo "##############################################################################################"
echo "Add new object in dc=tenant1,dc=com as vmware\administrator"
echo "##############################################################################################"
echo

ldapadd -c -h $host -p $port -x -D "cn=administrator,cn=users,dc=vmware,dc=com" -w 123 <<EOM
dn: cn=John-add,ou=eng,dc=tenant1,dc=com
changetype: add
cn: John-add
sn: Smith
objectClass: person
objectClass: organizationalPerson
telephoneNumber: 425-123-456-91
facsimileTelephoneNumber: 425-456-123-91
street: 20517 NE 91 th street
st: washington
l: redmond
postalCode: 98052
ou: Engineering
title: engineer
description: Employee of VMware
userPassword: PaWorD@123
EOM
'

echo "##############################################################################################"
echo "Add new object in dc=tenant1,dc=com as tenant1\John-1 a non-admin user"
echo "##############################################################################################"
echo
ldapadd -c -h $host -p $port -x -D "cn=John-1,ou=eng,dc=tenant1,dc=com" -w "PaWorD@123" <<EOM
dn: cn=John-add,ou=eng,dc=tenant1,dc=com
changetype: add
cn: John-add
sn: Smith
objectClass: person
objectClass: organizationalPerson
telephoneNumber: 425-123-456-91
facsimileTelephoneNumber: 425-456-123-91
street: 20517 NE 91 th street
st: washington
l: redmond
postalCode: 98052
ou: Engineering
title: engineer
description: Employee of VMware
EOM

echo "##############################################################################################"
echo "Add new object in dc=tenant1,dc=com as tenant2\administrator an admin user in other tenant"
echo "##############################################################################################"
echo
ldapadd -c -h $host -p $port -x -D "cn=administrator,cn=users,dc=tenant2,dc=com" -w 456 <<EOM
dn: cn=John-add,ou=eng,dc=tenant1,dc=com
changetype: add
cn: John-add
sn: Smith
objectClass: person
objectClass: organizationalPerson
telephoneNumber: 425-123-456-91
facsimileTelephoneNumber: 425-456-123-91
street: 20517 NE 91 th street
st: washington
l: redmond
postalCode: 98052
ou: Engineering
title: engineer
description: Employee of VMware
EOM


echo "##############################################################################################"
echo "Add new object in dc=tenant1,dc=com as tenant2\John-1 a non-admin user in other tenant"
echo "##############################################################################################"
echo
ldapadd -c -h $host -p $port -x -D "cn=John-1,ou=eng,dc=tenant2,dc=com" -w "PaWorD@123" <<EOM
dn: cn=John-add,ou=eng,dc=tenant1,dc=com
changetype: add
cn: John-add
sn: Smith
objectClass: person
objectClass: organizationalPerson
telephoneNumber: 425-123-456-91
facsimileTelephoneNumber: 425-456-123-91
street: 20517 NE 91 th street
st: washington
l: redmond
postalCode: 98052
ou: Engineering
title: engineer
description: Employee of VMware
EOM


echo "##############################################################################################"
echo "Add new object in dc=tenant1,dc=com as tenant1\administrator"
echo "administrator in the same tenant are the only account that can create other users"
echo "##############################################################################################"
echo
ldapadd -c -h $host -p $port -x -D "cn=administrator,cn=users,dc=tenant1,dc=com" -w 456 <<EOM
dn: cn=John-add,ou=eng,dc=tenant1,dc=com
changetype: add
cn: John-add
sn: Smith
objectClass: person
objectClass: organizationalPerson
telephoneNumber: 425-123-456-91
facsimileTelephoneNumber: 425-456-123-91
street: 20517 NE 91 th street
st: washington
l: redmond
postalCode: 98052
ou: Engineering
title: engineer
description: Employee of VMware
userPassword: PaWorD@123
EOM

ldapsearch -h $host -p $port -x -D "cn=administrator,cn=Users,dc=tenant1,dc=com" -w 456 -b "cn=John-add,ou=eng,dc=tenant1,dc=com" -s base "objectclass=*" dn

echo
echo "*********************************************************************************************"
echo "**************************DELETE ACL TESTS WITH TENANTS**************************************"
echo "*********************************************************************************************"
echo

: '
TODO - Uncomment this test case when vmware\administrator privilege is fixed

echo "################################################################################################"
echo "Delete tenant1 user as vmware\administrator -- this should fail"
echo "################################################################################################"
ldapmodify -c -h $host -p $port -x -D "cn=administrator,cn=Users,dc=vmware,dc=com" -w '123'<<EOM
dn: cn=John-add,ou=eng,dc=tenant1,dc=com
changetype: delete
EOM
'

echo "################################################################################################"
echo "Delete tenant1 user as tenant2\administrator -- this should fail"
echo "################################################################################################"
ldapmodify -c -h $host -p $port -x -D "cn=administrator,cn=Users,dc=tenant2,dc=com" -w '456'<<EOM
dn: cn=John-add,ou=eng,dc=tenant1,dc=com
changetype: delete
EOM


echo "################################################################################################"
echo "Delete tenant1 user as tenant1\administrator"
echo "################################################################################################"
ldapmodify -c -h $host -p $port -x -D "cn=administrator,cn=Users,dc=tenant1,dc=com" -w '456'<<EOM
dn: cn=John-add,ou=eng,dc=tenant1,dc=com
changetype: delete
EOM

ldapsearch -h $host -p $port -x -D "cn=administrator,cn=Users,dc=tenant1,dc=com" -w 456 -b "cn=John-add,ou=eng,dc=tenant1,dc=com" -s base "objectclass=*"


echo "*********************************************************************************************"
echo "**************************MODIFY ACL TESTS WITH TENANTS*****************************************"
echo "*********************************************************************************************"

echo "######################################################################"
	echo "Add an attribute value (cn: david) to an existing indexed attribute."
echo "######################################################################"
echo

echo "Bind as tenant1\John-1 to modify tenant1\John-2"
echo "This should be access denied"
ldapmodify -c -h $host -p $port -x -D "cn=John-1,ou=eng,dc=tenant1,dc=com" -w "PaWorD@123" <<EOM
dn: cn=John-2,ou=eng,dc=tenant1,dc=com
changetype: modify
add: displayName
displayName: wfu
EOM

echo "Bind as tenant2\John-1 to modify tenant1\John-1"
echo "This should be access denied"
ldapmodify -c -h $host -p $port -x -D "cn=John-1,ou=eng,dc=tenant2,dc=com" -w "PaWorD@123" <<EOM
dn: cn=John-1,ou=eng,dc=tenant1,dc=com
changetype: modify
add: displayName
displayName: wfu
EOM

echo "Bind as tenant2\John-1 to modify tenant1\John-2"
echo "This should be access denied"
ldapmodify -c -h $host -p $port -x -D "cn=John-1,ou=eng,dc=tenant2,dc=com" -w "PaWorD@123" <<EOM
dn: cn=John-2,ou=eng,dc=tenant1,dc=com
changetype: modify
add: displayName
displayName: wfu
EOM

echo "Bind as tenant2\administrator to modify tenant1\John-1"
echo "This should be access denied"
ldapmodify -c -h $host -p $port -x -D "cn=administrator,cn=users,dc=tenant2,dc=com" -w 456 <<EOM
dn: cn=John-1,ou=eng,dc=tenant1,dc=com
changetype: modify
add: displayName
displayName: wfu
EOM

: '
TODO - Uncomment this test case when vmware\administrator privilege is fixed

echo "Bind as vmware\administrator to modify tenant1\John-1"
echo "This should be access denied"
ldapmodify -c -h $host -p $port -x -D "cn=administrator,cn=users,dc=vmware,dc=com" -w 123 <<EOM
dn: cn=John-1,ou=eng,dc=tenant1,dc=com
changetype: modify
add: displayName
displayName: wfu
EOM
'

echo "Bind as tenant1\administrator to modify tenant1\John-1"
echo "This should succeed"
ldapmodify -c -h $host -p $port -x -D "cn=administrator,cn=users,dc=tenant1,dc=com" -w 456 <<EOM
dn: cn=John-1,ou=eng,dc=tenant1,dc=com
changetype: modify
add: displayName
displayName: wfu
EOM

ldapsearch -h $host -p $port -x -D "cn=administrator,cn=Users,dc=tenant1,dc=com" -w 456 -b "cn=John-1,ou=eng,dc=tenant1,dc=com" -s base "objectclass=*" dn

echo "Bind as tenant1\John-1 to modify tenant1\John-1"
echo "This should succeed"
ldapmodify -c -h $host -p $port -x -D "cn=John-1,ou=eng,dc=tenant1,dc=com" -w "PaWorD@123" <<EOM
dn: cn=John-1,ou=eng,dc=tenant1,dc=com
changetype: modify
add: department
department: cloud
EOM

ldapsearch -h $host -p $port -x -D "cn=administrator,cn=Users,dc=tenant1,dc=com" -w 456 -b "cn=John-1,ou=eng,dc=tenant1,dc=com" -s base "objectclass=*" dn


echo "*********************************************************************************************"
echo "**************************SEARCH ACL TESTS WITH TENANTS*****************************************"
echo "*********************************************************************************************"

echo "Bind as tenant1\John-1 to search tenant1\John-1"
echo "This should succeed, return 1 search result."
ldapsearch -h $host -p $port -x -D "cn=John-1,ou=eng,dc=tenant1,dc=com" -w "PaWorD@123" -b "cn=John-1,ou=eng,dc=tenant1,dc=com" -s base "objectclass=*" dn

echo
echo "Bind as tenant1\John-1 to search tenant2\John-1"
echo "This should fail, return 0 search result."
ldapsearch -h $host -p $port -x -D "cn=John-1,ou=eng,dc=tenant1,dc=com" -w "PaWorD@123" -b "cn=John-1,ou=eng,dc=tenant2,dc=com" -s base "objectclass=*" dn

echo
echo "Bind as tenant1\John-1 to search tenant1\John-2"
echo "This should fail, return 0 search result."
ldapsearch -h $host -p $port -x -D "cn=John-1,ou=eng,dc=tenant1,dc=com" -w "PaWorD@123" -b "cn=John-2,ou=eng,dc=tenant1,dc=com" -s base "objectclass=*" dn

echo
echo "Bind as tenant1\administrator to search tenant1\John-*"
echo "This should succeed, return 10 search result."
ldapsearch -h $host -p $port -x -D "cn=administrator,cn=users,dc=tenant1,dc=com" -w 456 -b "dc=tenant1,dc=com" -s subtree "(&(objectclass=*)(cn=John*))" dn

echo
echo "Bind as tenant1\administrator to search tenant2\John-*"
echo "This should fail, return 0 search result."
ldapsearch -h $host -p $port -x -D "cn=administrator,cn=users,dc=tenant1,dc=com" -w 456 -b "dc=tenant2,dc=com" -s subtree "(&(objectclass=*)(cn=John*))" dn

: '
TODO - Uncomment this test case when vmware\administrator privilege is fixed

echo
echo "Bind as vmware\administrator to search tenant1\John-*"
echo "This should fail, return 0 search result."
ldapsearch -h $host -p $port -x -D "cn=administrator,cn=users,dc=vmware,dc=com" -w 123 -b "dc=tenant1,dc=com" -s subtree "(&(objectclass=*)(cn=John*))" dn
'

echo
echo "*********************************************************************************************"
echo "**************************CLEANUP PHASE *****************************************************"
echo "*********************************************************************************************"
echo

./generate_data_tenant1_del.sh > data_tenant1_del.ldif
./generate_data_tenant2_del.sh > data_tenant2_del.ldif

: '
TODO - Uncomment this test case when vmware\administrator privilege is fixed

echo "################################################################################################"
echo "Remove tenant content as vmware\administrator -- this should fail"
echo "################################################################################################"

ldapmodify -c -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w '123' -f data_tenant1_del.ldif
'

echo "################################################################################################"
echo "Remove tenant2 content as tenant1\administrator -- this should fail"
echo "################################################################################################"

ldapmodify -c -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=tenant2,dc=com" -w '456' -f data_tenant1_del.ldif

echo "################################################################################################"
echo "Remove tenant1 content as tenant1\administrator"
echo "################################################################################################"

ldapmodify -c -h $host -p $port -x -D "cn=administrator,cn=Users,dc=tenant1,dc=com" -w '456' -f data_tenant1_del.ldif

echo "################################################################################################"
echo "Remove tenant2 content as tenant2\administrator"
echo "################################################################################################"

ldapmodify -c -h $host -p $port -x -D "cn=administrator,cn=Users,dc=tenant2,dc=com" -w '456' -f data_tenant2_del.ldif


echo "################################################################################################"
echo "Remove tenants root as vmware\administrator"
echo "################################################################################################"

ldapmodify -c -h $host -p $port -x -D "cn=administrator,cn=Users,dc=vmware,dc=com" -w '123'<<EOM
dn: dc=tenant1,dc=com
changetype: delete
EOM

ldapmodify -c -h $host -p $port -x -D "cn=administrator,cn=Users,dc=vmware,dc=com" -w '123'<<EOM
dn: dc=tenant2,dc=com
changetype: delete
EOM


# clean up data file
rm data_tenant1.ldif
rm data_tenant2.ldif
rm data_tenant1_del.ldif
rm data_tenant2_del.ldif



