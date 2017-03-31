#!/bin/sh
HOST=$1
ADMIN=$2
PASS=$3

ldapmodify -H ldap://$HOST -x -D "cn=$ADMIN,cn=Users,dc=vsphere,dc=local" -w "$PASS" <<EOF
dn: cn=test_lockout_user1,cn=users,dc=vsphere,dc=local
changetype: add
objectClass: top
objectClass: person
objectClass: organizationalPerson
objectClass: user
cn: test_lockout_user1
sAMAccountName: test_lockout_user1
userPrincipalName: test_lockout_user1@vsphere.local
sn: test_lockout_user1
userpassword: VMware12#
EOF

echo " --------------------------------------------- login normal user - should pass"
ldapsearch -H ldap://$HOST -x -D "cn=test_lockout_user1,cn=Users,dc=vsphere,dc=local" -w "VMware12#" -b "dc=vsphere,dc=local" -s base "objectclass=*" entryDN

echo " --------------------------------------------- set disable flag"
ldapmodify -H ldap://$HOST -x -D "cn=$ADMIN,cn=Users,dc=vsphere,dc=local" -w "$PASS" <<EOF
# change password
dn: cn=test_lockout_user1,cn=users,dc=vsphere,dc=local
changetype: modify
replace: useraccountcontrol
useraccountcontrol: 2
EOF

echo " --------------------------------------------- login normal user with disable flag set - should fail"
ldapsearch -H ldap://$HOST -x -D "cn=test_lockout_user1,cn=Users,dc=vsphere,dc=local" -w "VMware12#" -b "dc=vsphere,dc=local" -s base "objectclass=*" entryDN

echo " --------------------------------------------- add user into builtin administrators group"
ldapmodify -H ldap://$HOST -x -D "cn=$ADMIN,cn=Users,dc=vsphere,dc=local" -w "$PASS" <<EOF
dn: CN=administrators,CN=builtin,DC=vsphere,DC=local
changetype: modify
add: member
member: cn=test_lockout_user1,cn=users,dc=vsphere,dc=local
EOF

echo " --------------------------------------------- login normal user in builtin admin group with disable flag set - should fail"
ldapsearch -H ldap://$HOST -x -D "cn=test_lockout_user1,cn=Users,dc=vsphere,dc=local" -w "VMware12#" -b "dc=vsphere,dc=local" -s base "objectclass=*" entryDN

echo " --------------------------------------------- set disable flag for default administrator"
ldapmodify -H ldap://$HOST -x -D "cn=$ADMIN,cn=Users,dc=vsphere,dc=local" -w "$PASS" <<EOF
# change password
dn: cn=$ADMIN,cn=users,dc=vsphere,dc=local
changetype: modify
replace: useraccountcontrol
useraccountcontrol: 2
EOF

echo " --------------------------------------------- login as default administrator - should pass"
ldapsearch -H ldap://$HOST -x -D "cn=$ADMIN,cn=Users,dc=vsphere,dc=local" -w "$PASS" -b "dc=vsphere,dc=local" -s base "objectclass=*" entryDN

echo " --------------------------------------------- clean up"
ldapmodify -H ldap://$HOST -x -D "cn=$ADMIN,cn=Users,dc=vsphere,dc=local" -w "$PASS" <<EOF
# change password
dn: cn=$ADMIN,cn=users,dc=vsphere,dc=local
changetype: modify
replace: useraccountcontrol
useraccountcontrol: 0
EOF

ldapmodify -H ldap://$HOST -x -D "cn=$ADMIN,cn=Users,dc=vsphere,dc=local" -w "$PASS" <<EOF
dn: CN=administrators,CN=builtin,DC=vsphere,DC=local
changetype: modify
delete: member
member: cn=test_lockout_user1,cn=users,dc=vsphere,dc=local
EOF

ldapdelete -H ldap://$HOST -x -D "cn=$ADMIN,cn=Users,dc=vsphere,dc=local" -w "$PASS"  "CN=test_lockout_user1,CN=Users,DC=vsphere,DC=local"
