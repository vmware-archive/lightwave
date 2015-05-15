#!/bin/sh
HOST=$1
ADMIN=$2
PASS=$3

ldapmodify -H ldap://$HOST -x -D "cn=$ADMIN,cn=Users,dc=vsphere,dc=local" -w "$PASS" <<EOF
dn: cn=password and lockout policy, dc=vsphere,dc=local
changetype: modify
replace: vmwPasswordprohibitedPreviousCount
vmwPasswordprohibitedPreviousCount: 3
-
replace: vmwPasswordMaxLength
vmwPasswordMaxLength: 20
-
replace: vmwPasswordMinLength
vmwPasswordMinLength: 8
-
replace: vmwPasswordMinAlphabeticCount
vmwPasswordMinAlphabeticCount: 3
-
replace: vmwPasswordMinUpperCaseCount
vmwPasswordMinUpperCaseCount: 1
-
replace: vmwPasswordMinLowerCaseCount
vmwPasswordMinLowerCaseCount: 1
-
replace: vmwPasswordMinNumericCount
vmwPasswordMinNumericCount: 1
-
replace: vmwPasswordMinSpecialCharCount
vmwPasswordMinSpecialCharCount: 1
-
replace: vmwPasswordMaxIdenticalAdjacentChars
vmwPasswordMaxIdenticalAdjacentChars: 2
-
replace: vmwPasswordChangeMaxFailedAttempts
vmwPasswordChangeMaxFailedAttempts: 5


# add use
dn: CN=passwd_recycle_test,CN=Users,DC=vsphere,DC=local
changetype: add
objectClass: top
objectClass: person
objectClass: organizationalPerson
objectClass: user
cn: passwd_recycle_test
sAMAccountName: sruo
userPrincipalName: passwd_recycle_test@vsphere.local
sn: ruo
userpassword: 1Ssn123456@
EOF

echo "should fail"
ldapmodify -H ldap://$HOST -x -D "cn=passwd_recycle_test,cn=Users,dc=vsphere,dc=local" -w "1Ssn123456@" <<EOF
# change password
dn: CN=passwd_recycle_test,CN=Users,DC=vsphere,DC=local
changetype: modify
delete: userpassword
userpassword: 1Ssn123456@
-
add: userpassword
userpassword: 1Ssn123456@
EOF

echo "should pass"
ldapmodify -H ldap://$HOST -x -D "cn=passwd_recycle_test,cn=Users,dc=vsphere,dc=local" -w "1Ssn123456@" <<EOF
# change password
dn: CN=passwd_recycle_test,CN=Users,DC=vsphere,DC=local
changetype: modify
delete: userpassword
userpassword: 1Ssn123456@
-
add: userpassword
userpassword: 2Ssn123456@
EOF

echo "should fail"
ldapmodify -H ldap://$HOST -x -D "cn=passwd_recycle_test,cn=Users,dc=vsphere,dc=local" -w "2Ssn123456@" <<EOF
# change password
dn: CN=passwd_recycle_test,CN=Users,DC=vsphere,DC=local
changetype: modify
delete: userpassword
userpassword: 2Ssn123456@
-
add: userpassword
userpassword: 2Ssn123456@
EOF

echo "should pass"
ldapmodify -H ldap://$HOST -x -D "cn=passwd_recycle_test,cn=Users,dc=vsphere,dc=local" -w "2Ssn123456@" <<EOF
# change password
dn: CN=passwd_recycle_test,CN=Users,DC=vsphere,DC=local
changetype: modify
delete: userpassword
userpassword: 2Ssn123456@
-
add: userpassword
userpassword: 3Ssn123456@
EOF

echo "should fail - pasword recycle"
ldapmodify -H ldap://$HOST -x -D "cn=passwd_recycle_test,cn=Users,dc=vsphere,dc=local" -w "3Ssn123456@" <<EOF
# change password
dn: CN=passwd_recycle_test,CN=Users,DC=vsphere,DC=local
changetype: modify
delete: userpassword
userpassword: 3Ssn123456@
-
add: userpassword
userpassword: 1Ssn123456@
EOF

echo "should pass"
ldapmodify -H ldap://$HOST -x -D "cn=passwd_recycle_test,cn=Users,dc=vsphere,dc=local" -w "3Ssn123456@" <<EOF
# change password
dn: CN=passwd_recycle_test,CN=Users,DC=vsphere,DC=local
changetype: modify
delete: userpassword
userpassword: 3Ssn123456@
-
add: userpassword
userpassword: 4Ssn123456@
EOF

echo "should pass - over recycle count limit"
ldapmodify -H ldap://$HOST -x -D "cn=passwd_recycle_test,cn=Users,dc=vsphere,dc=local" -w "4Ssn123456@" <<EOF
# change password
dn: CN=passwd_recycle_test,CN=Users,DC=vsphere,DC=local
changetype: modify
delete: userpassword
userpassword: 4Ssn123456@
-
add: userpassword
userpassword: 1Ssn123456@
EOF

echo "should return one entry"
ldapsearch -H ldap://$HOST -x -D "cn=passwd_recycle_test,cn=Users,dc=vsphere,dc=local" -w "1Ssn123456@" -b "cn=passwd_recycle_test,cn=users,dc=vsphere,dc=local" -s base "objectclass=*" entryDN

echo "clean up"
ldapdelete -H ldap://$HOST -x -D "cn=$ADMIN,cn=Users,dc=vsphere,dc=local" -w "$PASS"  "CN=passwd_recycle_test,CN=Users,DC=vsphere,DC=local"
