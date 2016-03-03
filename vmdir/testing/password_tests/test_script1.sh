REL_DIR=`dirname $0`
cd $REL_DIR

source ../header.sh

#
# test cases for password set/change w/o password and lockout policy
#
echo "#########################################################"
echo "set policy parameter"
echo "#########################################################"
echo
ldapmodify -c -h $host -p $port -x -D "cn=administrator,cn=Users,dc=vmware,dc=com" -w 123 <<EOM
dn: cn=password and lockout policy,dc=vmware,dc=com
changetype: modify
replace: vmwPasswordChangeFailedAttemptIntervalSec
vmwPasswordChangeFailedAttemptIntervalSec: 2
-
replace: vmwPasswordChangeAutoUnlockIntervalSec
vmwPasswordChangeAutoUnlockIntervalSec: 2
-
replace: vmwPasswordChangeMaxFailedAttempts
vmwPasswordChangeMaxFailedAttempts: 2
EOM

echo "#########################################################"
echo "Create password test user cn=password_test,cn=users,dc=vmware,dc=com"
echo "#########################################################"
echo
ldapmodify -c -h $host -p $port -x -D "cn=administrator,cn=Users,dc=vmware,dc=com" -w 123 <<EOM
dn: cn=password_test,cn=users,dc=vmware,dc=com
changetype: add
objectclass: User
cn: password_test
sn: password_test_sn
userpassword: mypaSsword9@
description: vmidentity account
EOM

echo "#########################################################"
echo " TEST CASE 1"
echo " BIND : administrator user"
echo " ACTION: set password"
echo " RESULT: should fail - more than one userpassword"
echo "#########################################################"
echo
ldapmodify -c -h $host -p $port -x -D "cn=administrator,cn=Users,dc=vmware,dc=com" -w 123 <<EOM
dn: cn=password_test,cn=users,dc=vmware,dc=com
changetype: modify
replace: userpassword
userpassword: newpaSsword
userpassword: anothernewpaSsword
EOM

echo "#########################################################"
echo " TEST CASE 2"
echo " BIND : administrator user"
echo " ACTION: set password"
echo " RESULT: should succeed"
echo "#########################################################"
echo
ldapmodify -c -h $host -p $port -x -D "cn=administrator,cn=Users,dc=vmware,dc=com" -w 123 <<EOM
dn: cn=password_test,cn=users,dc=vmware,dc=com
changetype: modify
replace: userpassword
userpassword: NEWpaSsword9@
EOM

echo "#########################################################"
echo " verify password change"
echo "#########################################################"
echo
ldapsearch -c -h $host -p $port -x -D "cn=password_test,cn=users,dc=vmware,dc=com" -w "NEWpaSsword9@" -b "cn=password_test,cn=users,dc=vmware,dc=com" -s base "objectclass=*" dn

echo "#########################################################"
echo " TEST CASE 3"
echo " BIND : self user"
echo " ACTION: change password"
echo " RESULT: should fail - combine password change with other attribute"
echo "#########################################################"
echo
ldapmodify -c -h $host -p $port -x -D "cn=password_test,cn=Users,dc=vmware,dc=com" -w "NEWpaSsword9@" <<EOM
dn: cn=password_test,cn=users,dc=vmware,dc=com
changetype: modify
add: userpassword
userpassword: selfchangepaSsword
-
delete: userpassword
userpassword: NEWpaSsword9@
-
replace: sn
sn: aNewSN
EOM

echo "#########################################################"
echo " TEST CASE 4"
echo " BIND : self user"
echo " ACTION: change password"
echo " RESULT: should fail - add and delete not in pair"
echo "#########################################################"
echo
ldapmodify -c -h $host -p $port -x -D "cn=password_test,cn=Users,dc=vmware,dc=com" -w "NEWpaSsword9@" <<EOM
dn: cn=password_test,cn=users,dc=vmware,dc=com
changetype: modify
add: userpassword
userpassword: selfchangepaSsword
EOM

echo "#########################################################"
echo " TEST CASE 5"
echo " BIND : self user"
echo " ACTION: change password"
echo " RESULT: should fail - add and delete not in pair"
echo "#########################################################"
echo
ldapmodify -c -h $host -p $port -x -D "cn=password_test,cn=Users,dc=vmware,dc=com" -w "NEWpaSsword9@" <<EOM
dn: cn=password_test,cn=users,dc=vmware,dc=com
changetype: modify
delete: userpassword
userpassword: selfchangepaSsword
EOM

echo "#########################################################"
echo " TEST CASE 6"
echo " BIND : self user"
echo " ACTION: change password"
echo " RESULT: should fail - two values of userpassword"
echo "#########################################################"
echo
ldapmodify -c -h $host -p $port -x -D "cn=password_test,cn=Users,dc=vmware,dc=com" -w "NEWpaSsword9@" <<EOM
dn: cn=password_test,cn=users,dc=vmware,dc=com
changetype: modify
add: userpassword
userpassword: selfchangepaSsword
userpassword: anotherselfchangepaSsword
-
delete: userpassword
userpassword: newpaSsword
EOM

echo "#########################################################"
echo " TEST CASE 7"
echo " BIND : self user"
echo " ACTION: change password"
echo " RESULT: should fail - bad old password"
echo "#########################################################"
echo
ldapmodify -c -h $host -p $port -x -D "cn=password_test,cn=Users,dc=vmware,dc=com" -w "NEWpaSsword9@" <<EOM
dn: cn=password_test,cn=users,dc=vmware,dc=com
changetype: modify
add: userpassword
userpassword: selfCHG9@
-
delete: userpassword
userpassword: badpaSsword
EOM

echo "#########################################################"
echo " TEST CASE 8"
echo " BIND : self user"
echo " ACTION: change password"
echo " RESULT: should fail - strength fail - len < min"
echo "#########################################################"
echo
ldapmodify -c -h $host -p $port -x -D "cn=password_test,cn=Users,dc=vmware,dc=com" -w "NEWpaSsword9@" <<EOM
dn: cn=password_test,cn=users,dc=vmware,dc=com
changetype: modify
add: userpassword
userpassword: self
-
delete: userpassword
userpassword: NEWpaSsword9@
EOM

echo "#########################################################"
echo " TEST CASE 9"
echo " BIND : self user"
echo " ACTION: change password"
echo " RESULT: should fail - strength fail - len > max"
echo "#########################################################"
echo
ldapmodify -c -h $host -p $port -x -D "cn=password_test,cn=Users,dc=vmware,dc=com" -w "NEWpaSsword9@" <<EOM
dn: cn=password_test,cn=users,dc=vmware,dc=com
changetype: modify
add: userpassword
userpassword: selfCaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
-
delete: userpassword
userpassword: NEWpaSsword9@
EOM

echo "#########################################################"
echo " TEST CASE 10"
echo " BIND : self user"
echo " ACTION: change password"
echo " RESULT: should fail - strength fail - no special char"
echo "#########################################################"
echo
ldapmodify -c -h $host -p $port -x -D "cn=password_test,cn=Users,dc=vmware,dc=com" -w "NEWpaSsword9@" <<EOM
dn: cn=password_test,cn=users,dc=vmware,dc=com
changetype: modify
add: userpassword
userpassword: selfCHG9234
-
delete: userpassword
userpassword: NEWpaSsword9@
EOM

echo "#########################################################"
echo " TEST CASE 11"
echo " BIND : self user"
echo " ACTION: change password"
echo " RESULT: should fail - strength fail - no upper case char"
echo "#########################################################"
echo
ldapmodify -c -h $host -p $port -x -D "cn=password_test,cn=Users,dc=vmware,dc=com" -w "NEWpaSsword9@" <<EOM
dn: cn=password_test,cn=users,dc=vmware,dc=com
changetype: modify
add: userpassword
userpassword: selfchg9@
-
delete: userpassword
userpassword: NEWpaSsword9@
EOM

echo "#########################################################"
echo " TEST CASE 12"
echo " BIND : self user"
echo " ACTION: change password"
echo " RESULT: should fail - strength fail - no numeric char"
echo "#########################################################"
echo
ldapmodify -c -h $host -p $port -x -D "cn=password_test,cn=Users,dc=vmware,dc=com" -w "NEWpaSsword9@" <<EOM
dn: cn=password_test,cn=users,dc=vmware,dc=com
changetype: modify
add: userpassword
userpassword: selfCHG@d
-
delete: userpassword
userpassword: NEWpaSsword9@
EOM

echo "#########################################################"
echo " TEST CASE 13"
echo " BIND : self user"
echo " ACTION: change password"
echo " RESULT: should succeed - selfCHG9@"
echo "#########################################################"
echo
ldapmodify -c -h $host -p $port -x -D "cn=password_test,cn=Users,dc=vmware,dc=com" -w "NEWpaSsword9@" <<EOM
dn: cn=password_test,cn=users,dc=vmware,dc=com
changetype: modify
add: userpassword
userpassword: selfCHG9@
-
delete: userpassword
userpassword: NEWpaSsword9@
EOM

echo "#########################################################"
echo " verify password change"
echo "#########################################################"
echo
ldapsearch -c -h $host -p $port -x -D "cn=password_test,cn=users,dc=vmware,dc=com" -w "selfCHG9@" -b "cn=password_test,cn=users,dc=vmware,dc=com" -s base "objectclass=*" dn

echo "#########################################################"
echo " TEST CASE 14 - PREPARE"
echo "#########################################################"
echo
ldapmodify -c -h $host -p $port -x -D "cn=password_test,cn=Users,dc=vmware,dc=com" -w "selfCHG9@" <<EOM
dn: cn=password_test,cn=users,dc=vmware,dc=com
changetype: modify
add: userpassword
userpassword: selfCHG9#
-
delete: userpassword
userpassword: selfCHG9@
EOM

ldapsearch -c -h $host -p $port -x -D "cn=password_test,cn=users,dc=vmware,dc=com" -w "selfCHG9#" -b "cn=password_test,cn=users,dc=vmware,dc=com" -s base "objectclass=*" dn

echo "#########################################################"
echo " TEST CASE 14"
echo " BIND : self user"
echo " ACTION: set password but use prior value"
echo " RESULT: should fail - recycle password"
echo "#########################################################"
ldapmodify -c -h $host -p $port -x -D "cn=password_test,cn=Users,dc=vmware,dc=com" -w "selfCHG9#" <<EOM
dn: cn=password_test,cn=users,dc=vmware,dc=com
changetype: modify
add: userpassword
userpassword: selfCHG9@
-
delete: userpassword
userpassword: selfCHG9#
EOM

echo "#########################################################"
echo " TEST CASE 15 - PREPARE"
echo " useraccountcontrol lockout flag is clear 0x00000000"
echo "#########################################################"
echo
ldapsearch -c -h $host -p $port -x -D "cn=administrator,cn=users,dc=vmware,dc=com" -w "123" -b "cn=password_test,cn=users,dc=vmware,dc=com" -s base "objectclass=*" useraccountcontrol
echo
echo
echo "#########################################################"
echo " max fail attempt to trigger lockout - all search should fail"
echo " NOTE, we only need two more failures to exceed Max (2) as recycle"
echo " check above count one already."
echo "#########################################################"
echo
ldapsearch -c -h $host -p $port -x -D "cn=password_test,cn=users,dc=vmware,dc=com" -w "1selfCHG9#" -b "cn=password_test,cn=users,dc=vmware,dc=com" -s base "objectclass=*" dn
ldapsearch -c -h $host -p $port -x -D "cn=password_test,cn=users,dc=vmware,dc=com" -w "1selfCHG9#" -b "cn=password_test,cn=users,dc=vmware,dc=com" -s base "objectclass=*" dn

echo "#########################################################"
echo " TEST CASE 15"
echo " BIND : sefl user"
echo " ACTION: fail attempt > max allowed to trigger lockout"
echo " RESULT: should fail - account lockout"
echo "#########################################################"
ldapsearch -c -h $host -p $port -x -D "cn=password_test,cn=users,dc=vmware,dc=com" -w "selfCHG9#" -b "cn=password_test,cn=users,dc=vmware,dc=com" -s base "objectclass=*" useraccountcontrol

echo "#########################################################"
echo " TEST CASE 15-1"
echo " BIND : administrator user"
echo " ACTION: fail attempt > max allowed to trigger lockout"
echo " RESULT:  useraccountcontrol lockout flag is set 0x00000010"
echo "#########################################################"
echo
ldapsearch -c -h $host -p $port -x -D "cn=administrator,cn=users,dc=vmware,dc=com" -w "123" -b "cn=password_test,cn=users,dc=vmware,dc=com" -s base "objectclass=*" useraccountcontrol
echo
echo "#########################################################"
echo " sleep 3 secs for auto unlock interval"
echo "#########################################################"
echo
sleep 3

echo
echo "#########################################################"
echo " TEST CASE 16"
echo " BIND : self user"
echo " ACTION: lockout with autounlock time pass"
echo " RESULT:  useraccountcontrol lockout flag is set 0x00000000"
echo "#########################################################"
echo
ldapsearch -c -h $host -p $port -x -D "cn=password_test,cn=users,dc=vmware,dc=com" -w "selfCHG9#" -b "cn=password_test,cn=users,dc=vmware,dc=com" -s base "objectclass=*" useraccountcontrol

echo "#########################################################"
echo " TEST CASE 17"
echo " BIND : administrator user"
echo " ACTION: set user password that violate strength policy"
echo " RESULT:  should fail"
echo "#########################################################"
echo
ldapmodify -c -h $host -p $port -x -D "cn=administrator,cn=Users,dc=vmware,dc=com" -w 123 <<EOM
dn: cn=password_test,cn=users,dc=vmware,dc=com
changetype: modify
replace: userpassword
userpassword: badpass
EOM

echo
echo "#########################################################"
echo " self verify administrator set password fail in TEST CASE 18 - search should succceed"
echo "#########################################################"
echo
ldapsearch -c -h $host -p $port -x -D "cn=password_test,cn=users,dc=vmware,dc=com" -w "selfCHG9#" -b "cn=password_test,cn=users,dc=vmware,dc=com" -s base "objectclass=*" useraccountcontrol

echo
echo "#########################################################"
echo " TEST CASE 18 - PREPARE"
echo "#########################################################"
echo
ldapsearch -c -h $host -p $port -x -D "cn=administrator,cn=users,dc=vmware,dc=com" -w "bad123" -b "cn=password_test,cn=users,dc=vmware,dc=com" -s base "objectclass=*" dn
ldapsearch -c -h $host -p $port -x -D "cn=administrator,cn=users,dc=vmware,dc=com" -w "bad123" -b "cn=password_test,cn=users,dc=vmware,dc=com" -s base "objectclass=*" dn
ldapsearch -c -h $host -p $port -x -D "cn=administrator,cn=users,dc=vmware,dc=com" -w "bad123" -b "cn=password_test,cn=users,dc=vmware,dc=com" -s base "objectclass=*" dn
ldapsearch -c -h $host -p $port -x -D "cn=administrator,cn=users,dc=vmware,dc=com" -w "bad123" -b "cn=password_test,cn=users,dc=vmware,dc=com" -s base "objectclass=*" dn

echo
echo "#########################################################"
echo " TEST CASE 18"
echo " BIND : administrator user"
echo " ACTION: fail login exceed max allowed in policy"
echo " RESULT: should succeed - administrator account is NOT locked out"
echo "#########################################################"
echo
ldapsearch -c -h $host -p $port -x -D "cn=administrator,cn=users,dc=vmware,dc=com" -w "123" -b "cn=administrator,cn=users,dc=vmware,dc=com" -s base "objectclass=*" useraccountcontrol

echo "#########################################################"
echo " TEST CASE 19"
echo " BIND : administrator user"
echo " ACTION: create user with password strength violoation"
echo " RESULT: should fail"
echo "#########################################################"
echo
ldapmodify -c -h $host -p $port -x -D "cn=administrator,cn=Users,dc=vmware,dc=com" -w 123 <<EOM
dn: cn=password_test_next,cn=users,dc=vmware,dc=com
changetype: add
objectclass: vmIdentity-User
cn: password_test_next
sn: password_test_sn
userpassword: mypaSs
description: vmidentity account
EOM

echo
echo "#########################################################"
echo "Delete password test user cn=password_test,cn=users,dc=vmware,dc=com"
echo "#########################################################"
echo

ldapmodify -c -h $host -p $port -x -D "cn=administrator,cn=Users,dc=vmware,dc=com" -w 123 <<EOM
dn: cn=password_test,cn=users,dc=vmware,dc=com
changetype: delete
EOM
