REL_DIR=`dirname $0`
cd $REL_DIR

source ../header.sh

host="localhost"
port=389
admindn="cn=administrator,cn=users,dc=vmware,dc=com"
adminpw='123'


# 0.
# any prep here

./generate_data.sh > ./data.ldif
ldapadd -c -h $host -p $port -x -D $admindn -w $adminpw -f ./data.ldif > ./addObjects.out5 2>&1



# 1.
# test uniq default index (objectguid)
echo
echo "***** 1 *****"
echo

echo "hit enter"
read

# write a bunch
# - done in step 0

# try modify their objectguid: 123456
echo "*** Should succeed"
ldapmodify -c -h $host -p $port -x -D $admindn -w $adminpw <<EOM
dn: cn=John-1,ou=org-A,dc=vmware,dc=com
changetype: modify
replace: objectguid
objectguid: 123456
EOM
echo

echo "hit enter"
read

echo "*** Should fail"
ldapmodify -c -h $host -p $port -x -D $admindn -w $adminpw <<EOM
dn: cn=John-2,ou=org-B,dc=vmware,dc=com
changetype: modify
replace: objectguid
objectguid: 123456
EOM
echo

echo "hit enter"
read

# search (objectguid=....)
echo "*** objectGuid should be 123456"
ldapsearch -c -h $host -p $port -x -D $admindn -w $adminpw -b "cn=John-1,ou=org-A,dc=vmware,dc=com" -s base objectguid
echo

echo "hit enter"
read

echo "*** objectGuid should NOT be 123456"
ldapsearch -c -h $host -p $port -x -D $admindn -w $adminpw -b "cn=John-2,ou=org-B,dc=vmware,dc=com" -s base objectguid
echo

echo "hit enter"
read

echo "*** dn should be cn=John-1,ou=org-A,dc=vmware,dc=com"
ldapsearch -c -h $host -p $port -x -D $admindn -w $adminpw -b "dc=vmware,dc=com" -s sub "(objectguid=123456)" dn
echo

echo "hit enter"
read

# try modify vmwAttrUniquenessScope
echo "*** Attempt to add vmwAttrUniquenessScope - Should fail"
ldapmodify -c -h $host -p $port -x -D $admindn -w $adminpw <<EOM
dn: cn=objectguid,cn=schemacontext
changetype: modify
add: vmwAttrUniquenessScope
vmwAttrUniquenessScope: cn=users,dc=vmware,dc=com
EOM
echo

echo "hit enter"
read

echo "*** Attempt to delete vmwAttrUniquenessScope - Should fail"
ldapmodify -c -h $host -p $port -x -D $admindn -w $adminpw <<EOM
dn: cn=objectguid,cn=schemacontext
changetype: modify
delete: vmwAttrUniquenessScope
EOM
echo



# 2.
# test non-uniq default index (samaccountname)
echo
echo "***** 2 *****"
echo

echo "hit enter"
read

# write a bunch
# - done in step 0

# add a few uniq scope and test them
#  against existing data
#  against new data
echo "*** New uniqueness scopes for samaccountname"
ldapmodify -c -h $host -p $port -x -D $admindn -w $adminpw <<EOM
dn: cn=samaccountname,cn=schemacontext
changetype: modify
add: vmwAttrUniquenessScope
vmwAttrUniquenessScope: ou=org-A,dc=vmware,dc=com
vmwAttrUniquenessScope: ou=org-B,dc=vmware,dc=vom
EOM
echo

echo "hit enter"
read

echo "*** Same uniqueness scopes - Should fail"
ldapmodify -c -h $host -p $port -x -D $admindn -w $adminpw <<EOM
dn: cn=samaccountname,cn=schemacontext
changetype: modify
add: vmwAttrUniquenessScope
vmwAttrUniquenessScope: ou=org-A,dc=vmware,dc=com
vmwAttrUniquenessScope: ou=org-B,dc=vmware,dc=vom
EOM
echo

echo "hit enter"
read

echo "*** Samaccountname conflict - Should fail"
ldapmodify -c -h $host -p $port -x -D $admindn -w $adminpw <<EOM
dn: cn=xxxxx,ou=org-A,dc=vmware,dc=com
changetype: add
cn: xxxxx
sn: yyyyy
samaccountname: johnsmith-1
uid: 1
objectClass: user
objectClass: organizationalPerson
userPassword: PaWorD@123
telephoneNumber: 425-123-456-9999
facsimileTelephoneNumber: 425-456-123-9999
street: 20517 NE 9999 th street
st: washington
l: redmond
postalCode: 98052
ou: org-A
title: engineer
description: Employee of VMware
EOM
echo

echo "hit enter"
read

echo "*** Prepare for samaccountname conflict in org-C"
ldapmodify -c -h $host -p $port -x -D $admindn -w $adminpw <<EOM
dn: cn=xxxxx,ou=org-C,dc=vmware,dc=com
changetype: add
cn: xxxxx
sn: yyyyy
samaccountname: johnsmith-3
uid: 3
objectClass: user
objectClass: organizationalPerson
userPassword: PaWorD@123
telephoneNumber: 425-123-456-9999
facsimileTelephoneNumber: 425-456-123-9999
street: 20517 NE 9999 th street
st: washington
l: redmond
postalCode: 98052
ou: org-C
title: engineer
description: Employee of VMware
EOM
echo

echo "hit enter"
read

echo "*** Bad uniqueness scope (org-C) - Should fail (check log)"
ldapmodify -c -h $host -p $port -x -D $admindn -w $adminpw <<EOM
dn: cn=samaccountname,cn=schemacontext
changetype: modify
add: vmwAttrUniquenessScope
vmwAttrUniquenessScope: ou=org-C,dc=vmware,dc=com
EOM
echo

echo "hit enter"
read

echo "*** Search samaccountname vmwAttrUniquenessScope - Should be org-A and org-B only"
ldapsearch -c -h $host -p $port -x -D $admindn -w $adminpw -b "cn=samaccountname,cn=schemacontext" -s base vmwAttrUniquenessScope
echo

echo "hit enter"
read

echo "*** Delete samaacountname conflict in org-C"
ldapmodify -c -h $host -p $port -x -D $admindn -w $adminpw <<EOM
dn: cn=xxxxx,ou=org-C,dc=vmware,dc=com
changetype: delete
EOM
echo

echo "hit enter"
read

echo "*** New uniqueness scope (org-C) - Should succeed"
ldapmodify -c -h $host -p $port -x -D $admindn -w $adminpw <<EOM
dn: cn=samaccountname,cn=schemacontext
changetype: modify
add: vmwAttrUniquenessScope
vmwAttrUniquenessScope: ou=org-C,dc=vmware,dc=com
EOM
echo

echo "hit enter"
read

echo "*** Search samaccountname vmwAttrUniquenessScope - Should be org-A, org-B, and org-C"
ldapsearch -c -h $host -p $port -x -D $admindn -w $adminpw -b "cn=samaccountname,cn=schemacontext" -s base vmwAttrUniquenessScope
echo

echo "hit enter"
read

# search (samaccountname=...)
echo "*** Search (samaccountname=...)"
ldapsearch -c -h $host -p $port -x -D $admindn -w $adminpw -b "dc=vmware,dc=com" -s sub "(samaccountname=johnsmith-10*)" dn
echo

echo "hit enter"
read

# try delete index
echo "*** Attempt to delete default index - Should fail"
ldapmodify -c -h $host -p $port -x -D $admindn -w $adminpw <<EOM
dn: cn=samaccountname,cn=schemacontext
changetype: modify
replace: searchFlags
searchFlags: 2
EOM
echo

echo "hit enter"
read

echo "*** Delete uniqueness scopes - Should succeed"
ldapmodify -c -h $host -p $port -x -D $admindn -w $adminpw <<EOM
dn: cn=samaccountname,cn=schemacontext
changetype: modify
delete: vmwAttrUniquenessScope
EOM
echo



# 3.
# create a custom index (uid)
echo
echo "***** 3 *****"
echo

echo "hit enter"
read

echo "*** Start indexing for uid"
ldapmodify -c -h $host -p $port -x -D $admindn -w $adminpw <<EOM
dn: cn=uid,cn=schemacontext
changetype: modify
replace: vmwAttrUniquenessScope
vmwAttrUniquenessScope: ou=org-A,dc=vmware,dc=com
EOM
echo

echo "hit enter"
read

# add a uniq scope
echo "*** Add uid index a new uniq scope (org-B)"
ldapmodify -c -h $host -p $port -x -D $admindn -w $adminpw <<EOM
dn: cn=uid,cn=schemacontext
changetype: modify
add: vmwAttrUniquenessScope
vmwAttrUniquenessScope: ou=org-B,dc=vmware,dc=com
EOM
echo

echo "hit enter"
read

# add a uniq scope
echo "*** Add uid index a new uniq scope (org-C)"
ldapmodify -c -h $host -p $port -x -D $admindn -w $adminpw <<EOM
dn: cn=uid,cn=schemacontext
changetype: modify
add: vmwAttrUniquenessScope
vmwAttrUniquenessScope: ou=org-C,dc=vmware,dc=com
EOM
echo

echo "hit enter"
read

# test dup attr val (conflict)
echo "*** Dup uid (conflict) - Should fail"
ldapmodify -c -h $host -p $port -x -D $admindn -w $adminpw <<EOM
dn: cn=xxxxx,ou=org-A,dc=vmware,dc=com
changetype: add
cn: xxxxx
sn: yyyyy
samaccountname: johnsmith-1
uid: 1
objectClass: user
objectClass: organizationalPerson
userPassword: PaWorD@123
telephoneNumber: 425-123-456-9999
facsimileTelephoneNumber: 425-456-123-9999
street: 20517 NE 9999 th street
st: washington
l: redmond
postalCode: 98052
ou: org-A
title: engineer
description: Employee of VMware
EOM
echo

echo "hit enter"
read

# test dup attr val (conflict)
echo "*** Dup uid (conflict) - Should fail"
ldapmodify -c -h $host -p $port -x -D $admindn -w $adminpw <<EOM
dn: cn=xxxxx,ou=org-B,dc=vmware,dc=com
changetype: add
cn: xxxxx
sn: yyyyy
samaccountname: johnsmith-2
uid: 2
objectClass: user
objectClass: organizationalPerson
userPassword: PaWorD@123
telephoneNumber: 425-123-456-9999
facsimileTelephoneNumber: 425-456-123-9999
street: 20517 NE 9999 th street
st: washington
l: redmond
postalCode: 98052
ou: org-B
title: engineer
description: Employee of VMware
EOM
echo

echo "hit enter"
read

# test dup attr val (conflict)
echo "*** Dup uid (conflict) - Should fail"
ldapmodify -c -h $host -p $port -x -D $admindn -w $adminpw <<EOM
dn: cn=xxxxx,ou=org-C,dc=vmware,dc=com
changetype: add
cn: xxxxx
sn: yyyyy
samaccountname: johnsmith-3
uid: 3
objectClass: user
objectClass: organizationalPerson
userPassword: PaWorD@123
telephoneNumber: 425-123-456-9999
facsimileTelephoneNumber: 425-456-123-9999
street: 20517 NE 9999 th street
st: washington
l: redmond
postalCode: 98052
ou: org-C
title: engineer
description: Employee of VMware
EOM
echo

echo "hit enter"
read

# delete a uniq scope
echo "*** Delete a uniq scope (org-C) from uid index"
ldapmodify -c -h $host -p $port -x -D $admindn -w $adminpw <<EOM
dn: cn=uid,cn=schemacontext
changetype: modify
delete: vmwAttrUniquenessScope
vmwAttrUniquenessScope: ou=org-C,dc=vmware,dc=com
EOM
echo

echo "hit enter"
read

# test dup attr val (no conflict)
echo "*** Dup uid (no conflict) - Should Succeed"
ldapmodify -c -h $host -p $port -x -D $admindn -w $adminpw <<EOM
dn: cn=xxxxx,ou=org-C,dc=vmware,dc=com
changetype: add
cn: xxxxx
sn: yyyyy
samaccountname: johnsmith-3
uid: 3
objectClass: user
objectClass: organizationalPerson
userPassword: PaWorD@123
telephoneNumber: 425-123-456-9999
facsimileTelephoneNumber: 425-456-123-9999
street: 20517 NE 9999 th street
st: washington
l: redmond
postalCode: 98052
ou: org-C
title: engineer
description: Employee of VMware
EOM
echo

echo "hit enter"
read

# test search (uid=99*)
echo "*** Search uid=99* - Should succeed"
ldapsearch -c -h $host -p $port -x -D $admindn -w $adminpw -b "dc=vmware,dc=com" -s sub "(uid=99*)" uid
echo

echo "hit enter"
read

# try delete index
echo "*** Attempt to delete custom index - should succeed"
ldapmodify -c -h $host -p $port -x -D $admindn -w $adminpw <<EOM
dn: cn=uid,cn=schemacontext
changetype: modify
delete: searchFlags
EOM
echo

echo "hit enter"
read

# search uid
echo "*** Search uid - Should be NO searchFlags and vmwAttrUniquenessScope"
ldapsearch -c -h $host -p $port -x -D $admindn -w $adminpw -b "cn=uid,cn=schemacontext" -s base
echo



# 4.
# create an attribute schema entry with searchFlags=1 and scope=global
echo
echo "***** 4 *****"
echo

echo "hit enter"
read

echo "*** Create a new attribute type (testAttr) and add it to user maycontain list"
ldapmodify -c -h $host -p $port -x -D $admindn -w $adminpw <<EOM
dn: cn=testAttr,cn=schemacontext
changetype: add
cn: testAttr
objectclass: attributeschema
attributeid: 111.111.0.001
attributesyntax: 1.3.6.1.4.1.1466.115.121.1.27
issinglevalued: TRUE
searchFlags: 2
vmwAttrUniquenessScope: cn=DSE Root
EOM
echo

echo "hit enter"
read

ldapmodify -c -h $host -p $port -x -D $admindn -w $adminpw <<EOM
dn: cn=user,cn=schemacontext
changetype: modify
add: maycontain
maycontain: testAttr
EOM
echo

echo "hit enter when index is complete"
read

echo "*** testAttr=1 - Should succeed"
ldapmodify -c -h $host -p $port -x -D $admindn -w $adminpw <<EOM
dn: cn=John-1,ou=org-A,dc=vmware,dc=com
changetype: modify
add: testAttr
testAttr: 1
EOM
echo

echo "hit enter"
read

echo "*** testAttr=1 - Should fail"
ldapmodify -c -h $host -p $port -x -D $admindn -w $adminpw <<EOM
dn: cn=John-2,ou=org-B,dc=vmware,dc=com
changetype: modify
add: testAttr
testAttr: 1
EOM
echo

echo "hit enter"
read

# add uniq scopes and see if it's still global
echo "*** Add uniq scopes - should still be global"
ldapmodify -c -h $host -p $port -x -D $admindn -w $adminpw <<EOM
dn: cn=testAttr,cn=schemacontext
changetype: modify
add: vmwAttrUniquenessScope
vmwAttrUniquenessScope: ou=org-A,dc=vmware,dc=com
vmwAttrUniquenessScope: ou=org-B,dc=vmware,dc=com
vmwAttrUniquenessScope: ou=org-C,dc=vmware,dc=com
EOM
echo

echo "hit enter"
read

echo "*** testAttr=1 - Should fail"
ldapmodify -c -h $host -p $port -x -D $admindn -w $adminpw <<EOM
dn: cn=John-3,ou=org-C,dc=vmware,dc=com
changetype: modify
add: testAttr
testAttr: 1
EOM
echo



