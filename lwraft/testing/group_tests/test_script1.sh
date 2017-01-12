REL_DIR=`dirname $0`
cd $REL_DIR

source ../header.sh

echo "#########################################################"
echo "Create Group-1 with John-1 and John-2 as members"
echo "#########################################################"
echo

ldapmodify -c -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 <<EOM
dn: cn=Group-1,ou=eng,dc=vmware,dc=com
changetype: add
cn: Group-1
member: cn=John-1,ou=eng,dc=vmware,dc=com
member: cn=John-2,ou=eng,dc=vmware,dc=com
member: externalObjectId=S-1-2-3-4
member: externalObjectId=06783c58-4fdf-1031-8788-c37afe35b265
objectClass: groupOfNames
EOM

echo "#########################################################"
echo "Create Group-2 with John-2, John-3, John-4, and John-5 as members"
echo "#########################################################"
echo

ldapmodify -c -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 <<EOM
dn: cn=Group-2,ou=eng,dc=vmware,dc=com
changetype: add
cn: Group-2
member: cn=John-2,ou=eng,dc=vmware,dc=com
member: cn=John-3,ou=eng,dc=vmware,dc=com
member: cn=John-4,ou=eng,dc=vmware,dc=com
member: cn=John-5,ou=eng,dc=vmware,dc=com
member: externalObjectId=S-1-2-3-5
member: externalObjectId=06783c58-4fdf-1031-8778-c37afe35b265
objectClass: groupOfNames
EOM

echo "#########################################################"
echo "Try to add a non-existing member to Group-1"
echo "#########################################################"
echo

ldapmodify -c -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 <<EOM
dn: cn=Group-1,ou=eng,dc=vmware,dc=com
changetype: modify
add: member
member: cn=John-x,ou=eng,dc=vmware,dc=com
EOM

echo "#########################################################"
echo "Add Group-1 as member to Group-2"
echo "#########################################################"
echo

ldapmodify -c -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 <<EOM
dn: cn=Group-2,ou=eng,dc=vmware,dc=com
changetype: modify
add: member
member: cn=Group-1,ou=eng,dc=vmware,dc=com
EOM

echo "#########################################################"
echo "Search for John-1"
echo "#########################################################"
echo

ldapsearch -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 -b "" -s subtree "cn=John-1" memberOf

echo "#########################################################"
echo "Search for John-2"
echo "#########################################################"
echo

ldapsearch -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 -b "" -s subtree "cn=John-2" memberOf

echo "#########################################################"
echo "Search for John-3"
echo "#########################################################"
echo

ldapsearch -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 -b "" -s subtree "cn=John-3" memberOf

echo "#########################################################"
echo "Search for groups"
echo "#########################################################"
echo

ldapsearch -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 -b "" -s subtree "objectClass=groupOfNames" member

echo "#########################################################"
echo "Delete John-1"
echo "#########################################################"
echo

ldapmodify -c -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 <<EOM
dn: cn=John-1,ou=eng,dc=vmware,dc=com
changetype: delete
EOM

echo "#########################################################"
echo "Search for groups"
echo "#########################################################"
echo

ldapsearch -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 -b "" -s subtree "objectClass=groupOfNames" member

echo "#########################################################"
echo "Delete John-2"
echo "#########################################################"
echo

ldapmodify -c -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 <<EOM
dn: cn=John-2,ou=eng,dc=vmware,dc=com
changetype: delete
EOM

echo "#########################################################"
echo "Search for groups"
echo "#########################################################"
echo

ldapsearch -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 -b "" -s subtree "objectClass=groupOfNames" member

echo "#########################################################"
echo "Delete John-3"
echo "#########################################################"
echo

ldapmodify -c -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 <<EOM
dn: cn=John-3,ou=eng,dc=vmware,dc=com
changetype: delete
EOM

echo "#########################################################"
echo "Search for groups"
echo "#########################################################"
echo


ldapsearch -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 -b "" -s subtree "objectClass=groupOfNames" member

echo "#########################################################"
echo "Delete Group-1"
echo "#########################################################"
echo

ldapmodify -c -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 <<EOM
dn: cn=Group-1,ou=eng,dc=vmware,dc=com
changetype: delete
EOM

echo "#########################################################"
echo "Search for groups"
echo "#########################################################"
echo

ldapsearch -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 -b "" -s subtree "objectClass=groupOfNames" member

echo "#########################################################"
echo "Delete John-4 as member from Group-2"
echo "#########################################################"
echo

ldapmodify -c -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 <<EOM
dn: cn=Group-2,ou=eng,dc=vmware,dc=com
changetype: modify
delete: member
member: cn=John-4,ou=eng,dc=vmware,dc=com
EOM

echo "#########################################################"
echo "Search for groups"
echo "#########################################################"
echo

ldapsearch -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 -b "" -s subtree "objectClass=groupOfNames" member

echo "#########################################################"
echo "Delete John-5 as member from Group-2"
echo "#########################################################"
echo

ldapmodify -c -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 <<EOM
dn: cn=Group-2,ou=eng,dc=vmware,dc=com
changetype: modify
delete: member
member: cn=John-5,ou=eng,dc=vmware,dc=com
EOM

echo "#########################################################"
echo "Search for groups"
echo "#########################################################"
echo

ldapsearch -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 -b "" -s subtree "objectClass=groupOfNames" member

echo "#########################################################"
echo "Delete S-1-2-3-5 as member from Group-2"
echo "#########################################################"
echo

ldapmodify -c -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 <<EOM
dn: cn=Group-2,ou=eng,dc=vmware,dc=com
changetype: modify
delete: member
member: externalObjectId=S-1-2-3-5
EOM

echo "#########################################################"
echo "Search for groups"
echo "#########################################################"
echo

ldapsearch -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 -b "" -s subtree "objectClass=groupOfNames" member

echo "##################################################################"
echo "Delete 06783c58-4fdf-1031-8778-c37afe35b265 from Group-2"
echo "##################################################################"
echo

ldapmodify -c -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 <<EOM
dn: cn=Group-2,ou=eng,dc=vmware,dc=com
changetype: modify
delete: member
member: externalObjectId=06783c58-4fdf-1031-8778-c37afe35b265
EOM

echo "#########################################################"
echo "Search for groups"
echo "#########################################################"
echo

ldapsearch -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 -b "" -s subtree "objectClass=groupOfNames" member


echo "#########################################################"
echo "Delete Group-2"
echo "#########################################################"
echo

ldapmodify -c -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 <<EOM
dn: cn=Group-2,ou=eng,dc=vmware,dc=com
changetype: delete
EOM

echo "#########################################################"
echo "Search for ForeignSecurityPrincipals"
echo "#########################################################"
echo

ldapsearch -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 -b "" -s subtree "objectClass=foreignSecurityPrincipal" externalObjectId

