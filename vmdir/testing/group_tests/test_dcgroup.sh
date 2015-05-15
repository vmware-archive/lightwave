#!/bin/sh

HOST=$1
ADMIN=$2
PASS=$3

####################################################################################
# 1. member of dcgroup should have system admin rights (per PROD2013 requirements).
####################################################################################

ldapmodify -H ldap://$HOST -x -D "cn=$ADMIN,cn=Users,dc=vsphere,dc=local" -w "$PASS" <<EOF
# add use
dn: CN=dcgroup_user,CN=Users,DC=vsphere,DC=local
changetype: add
objectClass: top
objectClass: person
objectClass: organizationalPerson
objectClass: user
cn: dcgroup_user
sAMAccountName: dcgroup_user
userPrincipalName: dcgroup_user@vsphere.local
sn: dcgroup_user_sn
userpassword: 1Ssn123456@
EOF

echo "should fail - no write permission"
ldapmodify -H ldap://$HOST -x -D "cn=dcgroup_user,cn=Users,dc=vsphere,dc=local" -w "1Ssn123456@" <<EOF
dn: CN=dcgroup_user1,CN=Users,DC=vsphere,DC=local
changetype: add
objectClass: top
objectClass: person
objectClass: organizationalPerson
objectClass: user
cn: dcgroup_user1
sAMAccountName: dcgroup_user1
userPrincipalName: dcgroup_user1@vsphere.local
sn: dcgroup_user1_sn
userpassword: 1Ssn123456@
EOF

echo "add dcgroup_user into dcgroup"
ldapmodify -H ldap://$HOST -x -D "cn=$ADMIN,cn=Users,dc=vsphere,dc=local" -w "$PASS" <<EOF
dn: CN=dcadmins,CN=builtin,DC=vsphere,DC=local
changetype: modify
add: member
member: CN=dcgroup_user,CN=Users,DC=vsphere,DC=local
EOF

echo "should pass"
ldapmodify -H ldap://$HOST -x -D "cn=dcgroup_user,cn=Users,dc=vsphere,dc=local" -w "1Ssn123456@" <<EOF
dn: CN=dcgroup_user1,CN=Users,DC=vsphere,DC=local
changetype: add
objectClass: top
objectClass: person
objectClass: organizationalPerson
objectClass: user
cn: dcgroup_user1
sAMAccountName: dcgroup_user1
userPrincipalName: dcgroup_user1@vsphere.local
sn: dcgroup_user1_sn
userpassword: 1Ssn123456@
EOF

echo "clean up"
ldapdelete -H ldap://$HOST -x -D "cn=$ADMIN,cn=Users,dc=vsphere,dc=local" -w "$PASS"  "CN=dcgroup_user,CN=Users,DC=vsphere,DC=local"
ldapdelete -H ldap://$HOST -x -D "cn=$ADMIN,cn=Users,dc=vsphere,dc=local" -w "$PASS"  "CN=dcgroup_user1,CN=Users,DC=vsphere,DC=local"
