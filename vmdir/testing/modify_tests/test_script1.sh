REL_DIR=`dirname $0`
cd $REL_DIR

source ../header.sh

echo "#########################################################"
echo "Create telephonenumber equality index"
echo "#########################################################"
echo

ldapmodify -c -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 <<EOM
dn: cn=indices,cn=config
changetype: modify
add: vmwAttrIndexDesc
vmwAttrIndexDesc: telephonenumber eq
vmwAttrIndexDesc: otherTelephone eq
EOM

ldapsearch -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 -b "cn=indices,cn=config" -s base "objectclass=*"

echo "#########################################################"
echo "Add a new attribute (otherTelephone: 223-556-8899)."
echo "#########################################################"
echo

ldapmodify -c -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 <<EOM
dn: cn=John-2,ou=eng,dc=vmware,dc=com
changetype: modify
add: otherTelephone
otherTelephone: 223-556-8899
EOM

ldapsearch -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 -b "dc=com" -s subtree "otherTelephone=223-556-8899" dn otherTelephone

echo "#########################################################"
echo "Add an attribute value (otherTelephone: 224-557-8800) to an existing indexed attribute."
echo "#########################################################"
echo

ldapmodify -c -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 <<EOM
dn: cn=John-2,ou=eng,dc=vmware,dc=com
changetype: modify
add: otherTelephone
otherTelephone: 224-557-8800
EOM

ldapsearch -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 -b "dc=com" -s subtree "otherTelephone=224-557-8800" dn otherTelephone

echo "#########################################################"
echo "Delete an attribute value (otherTelephone: 224-557-8800) for an indexed attribute."
echo "#########################################################"
echo

ldapmodify -c -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 <<EOM
dn: cn=John-2,ou=eng,dc=vmware,dc=com
changetype: modify
delete: otherTelephone
otherTelephone: 224-557-8800
EOM

ldapsearch -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 -b "dc=com" -s subtree "otherTelephone=224-557-8800"

ldapsearch -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 -b "dc=com" -s subtree "otherTelephone=223-556-8899" dn otherTelephone

echo "#########################################################"
echo "Delete a non-indexed attribute (st)"
echo "#########################################################"
echo

ldapmodify -c -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 <<EOM
dn: cn=John-2,ou=eng,dc=vmware,dc=com
changetype: modify
delete: st
EOM

ldapsearch -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 -b "dc=com" -s subtree "cn=John-2" dn st

echo "#########################################################"
echo "Add a non-indexed attribute (st)"
echo "#########################################################"
echo

ldapmodify -c -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 <<EOM
dn: cn=John-2,ou=eng,dc=vmware,dc=com
changetype: modify
add: st
st: washington
EOM

ldapsearch -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 -b "dc=com" -s subtree "cn=John-2" st

echo "#########################################################"
echo "Multiple Add mods (cn:david, st:california) should fail due to attribute st is a single value attribute"
echo "#########################################################"
echo

ldapmodify -c -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 <<EOM
dn: cn=John-2,ou=eng,dc=vmware,dc=com
changetype: modify
add: cn
cn: david
-
add: st
st: california
EOM

ldapsearch -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 -b "dc=com" -s subtree "cn=John-2" st

echo "#########################################################"
echo "Multiple Delete mods (cn:david, st:california)"
echo "#########################################################"
echo

ldapmodify -c -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 <<EOM
dn: cn=John-2,ou=eng,dc=vmware,dc=com
changetype: modify
delete: cn
cn: david
-
delete: st
st: california
EOM

ldapsearch -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 -b "dc=com" -s subtree "cn=John-2" st

echo "#########################################################"
echo "Try to delete a MUST attribute (cn)"
echo "#########################################################"
echo

ldapmodify -c -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 <<EOM
dn: cn=John-2,ou=eng,dc=vmware,dc=com
changetype: modify
delete: cn
EOM

ldapsearch -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 -b "dc=com" -s subtree "cn=John-2" dn cn

echo "#########################################################"
echo "Delete telephonenumber attribute by replacing it with no values"
echo "#########################################################"
echo

ldapmodify -c -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 <<EOM
dn: cn=John-2,ou=eng,dc=vmware,dc=com
changetype: modify
replace: telephonenumber
EOM

ldapsearch -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 -b "dc=com" -s subtree "cn=John-2" dn telephonenumber

echo "#########################################################"
echo "Add telephonenumber attribute by replacing it with values"
echo "#########################################################"
echo

ldapmodify -c -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 <<EOM
dn: cn=John-2,ou=eng,dc=vmware,dc=com
changetype: modify
replace: telephonenumber
telephonenumber: 425-123-456-2
EOM

ldapsearch -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 -b "dc=com" -s subtree "cn=John-2" dn telephonenumber

echo "#########################################################"
echo "Replace telephonenumber attribute values"
echo "#########################################################"
echo

ldapmodify -c -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 <<EOM
dn: cn=John-2,ou=eng,dc=vmware,dc=com
changetype: modify
replace: telephonenumber
telephonenumber: 0987654321-1
telephonenumber: 0987654321-2
telephonenumber: 0987654321-3
EOM

ldapsearch -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 -b "dc=com" -s subtree "telephonenumber=0987654321-2"


#echo "#########################################################"
#echo "Replace seeAlso attribute with same values - should fail 20"
#echo "#########################################################"
#echo

#ldapmodify -c -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 <<EOM
#dn: cn=John-2,ou=eng,dc=vmware,dc=com
#changetype: modify
#replace: seeAlso
#seeAlso: cn=VALUE1
#seeAlso: cn=value1
#EOM

#echo "#########################################################"
#echo "Add new user with same seeAlso attribute values - should fail 20"
#echo "#########################################################"
#echo

#ldapmodify -c -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 <<EOM
#dn: cn=Joe-2,ou=eng,dc=vmware,dc=com
#changetype: add
#cn: Joe-2
#sn: Joe-2-sn
#objectclass: person
#seeAlso: cn=VALUE1
#seeAlso: cn=value1
#EOM

#echo "#########################################################"
#echo "Add new user Joe-2"
#echo "#########################################################"
#echo

#ldapmodify -c -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 <<EOM
#dn: cn=Joe-2,ou=eng,dc=vmware,dc=com
#changetype: add
#cn: Joe-2
#sn: Joe-2-sn
#objectclass: person
#seeAlso: cn=VALUE1
#EOM

#echo "#########################################################"
#echo "Modify add attribute which already exists - should fail 20"
#echo "#########################################################"
#echo

#ldapmodify -c -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 <<EOM
#dn: cn=Joe-2,ou=eng,dc=vmware,dc=com
#changetype: modify
#add: seeAlso
#seeAlso: cn=valUE1
#EOM

#echo "#########################################################"
#echo "Delete user Joe-2"
#echo "#########################################################"
#echo

#ldapmodify -c -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 <<EOM
#dn: cn=Joe-2,ou=eng,dc=vmware,dc=com
#changetype: delete
#EOM
