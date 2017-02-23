# Test Script that performs testing for LDAP REST APIs and URI validation
# This script takes following input
# 1. New User DN prefix: which is path to parent node whose child entry you want to create
#	i.e. cn=Users,dc=vsphere,dc=local in "cn=newUser,cn=Users,dc=vsphere,dc=local"
# 2. New User CN: which is just "newUser" for above case
# 3. new user object class: name of object class for new add user
# 4. destination_file: File name where you want to store JSON file (File will be used for add operation)
# 5. Unauthorized User DN: This is a less priviliged user (should already be created)
# 6. Unauthorized User password: Password for above user
 
REL_DIR=`dirname $0`
cd $REL_DIR

## VARIABLES AND CONSTANTS ##

host="localhost"
port="7477"

# Admin DN
admindn="cn=administrator,cn=users,dc=vsphere,dc=local"
adminpw='Admin!23'

# Get DN and Password  of unauthorized(less priviliged) user
echo "Enter DN for less priviliged user"
read unauthorizeduserdn
echo "Enter Password for above user"
read userpw

# Target DN for add
add_target_dn="dc=vsphere,dc=local"

# Incorrect Password and Incorrect UserName
incorrectuser="cn=xxxx,dc=vmrest,dc=loxxx"
incorrectpw='Adminxxxxxxasfd'

# Invalid Target DN
invalid_target_dn="cn=useradabs,dc=sdfa,dc=local"

# Generate JSON file for ADD Operation
echo "Enter new user DN prefix"
read new_user_dn_prefix
echo "Enter new user CN"
read new_user_cn
echo "Enter new user object class"
read new_user_obj_class
echo "Enter destination filename"
read destination_file

sh gen_json_for_rest_add.sh $new_user_dn_prefix $new_user_cn $new_user_obj_class $destination_file

# Generate JSON to perform PATCH operation
echo "["> /tmp/patch.json
echo "{ \"op\": \"replace\", \"path\": \"description\", \"value\": [\"Description123\"] }" >> /tmp/patch.json
echo "]">> /tmp/patch.json
modify_user_json_file=/tmp/patch.json

# Patch and Delete Target DN based on added user
added_user_dn="cn=$new_user_cn,$new_user_dn_prefix"

# Generate Invalid JSON file
echo "{<>}" > /tmp/invalid.json
invalid_json_file="/tmp/invalid.json"

# Generate JSON with Invalid attribute to perform PATCH operation
echo "["> /tmp/invalid_attr_patch.json
echo "{ \"op\": \"replace123\", \"path\": \"descriptionxxx\", \"value\": [\"Description xxx\"] }" >> /tmp/invalid_attr_patch.json
echo "]">> /tmp/invalid_attr_patch.json
invalid_attr_patch_json=/tmp/invalid_attr_patch.json

## ADD OPERATION UNIT TESTS ##
echo
echo "Add Operation testing started"
echo

# Unit Test 1: Perform Add operation with Authorized user (Entry doesn't exists)
echo "Unit Test 1: Adding new Entry with valid authorization"
echo "Expected Result: Should Succeed"
echo "Hit Enter to continue"
read
curl -X PUT -d@$destination_file -v -u $admindn:$adminpw http://$host:$port/vmdir/ldap/$add_target_dn/
echo "Unit Test 1 Done. Hit Enter to continue"
read

# Unit Test 2: Perform Add operation with Authorized User (Entry already exists)
echo
echo "Unit Test 2: Adding an Entry which is already there (Authorized user)"
echo "Expected Result: Should Not Succeed"
echo "Hit Enter to continue"
read
curl -X PUT -d@$destination_file -v -u $admindn:$adminpw http://$host:$port/vmdir/ldap/$add_target_dn/
echo "Unit Test 2 Done. Hit Enter to continue"
read

# Unit Test 3: Perform Add operation with Unauthorized User (Entry already exists)
echo
echo "Unit Test 3: Adding an Entry which is already there (Unauthorized User)"
echo "Expected Result: Access Should Be Denied"
echo "Hit Enter to continue"
read
curl -X PUT -d@$destination_file -v -u $unauthorizeduserdn:$userpw http://$host:$port/vmdir/ldap/$add_target_dn/
echo "Unit Test 3 Done. Hit Enter to continue"
read

# Unit Test 4: Perform Add operation with Unauthorized User (Entry doesn't exists)
echo
echo "Unit Test 4: Adding new Entry (Unauthorized User)"
echo "Expected Result: Access Should Be Denied"
echo "Hit Enter to continue"
read
curl -X PUT -d@$destination_file -v -u $unauthorizeduserdn:$userpw http://$host:$port/vmdir/ldap/$add_target_dn/
echo "Unit Test 4 Done. Hit Enter to continue"
read

# Unit Test 5: Perform Add Operation, Invalid JSON file
echo
echo "Unit Test 5: Adding new Entry, Invalid JSON file (User Authorization doesn't matter)"
echo "Expected Result: Should Not Succeed"
echo "Hit Enter to continue"
read
curl -X PUT -d@$invalid_json_file -v -u $admindn:$adminpw http://$host:$port/vmdir/ldap/$add_target_dn/
echo "Unit Test 5 Done. Hit Enter to continue"
read

# Unit Test 6: Perform Add Operation, Invalid Target DN
echo
echo "Unit Test 6: Adding new Entry, Invalid Target DN (User Authorization doesn't matter)"
echo "Expected Result: Should Not Succeed"
echo "Hit Enter to continue"
read
curl -X PUT -d@$destination_file -v -u $admindn:$adminpw http://$host:$port/vmdir/ldap/$invalid_target_dn/
echo "Unit Test 6 Done"
echo
echo "Add Operation Testing Done"
echo "Hit Enter to continue"
read

## SEARCH OPERATION TESTING ##
echo
echo "Search Operation testing started"
echo

# Unit Test 7: Search Operation with Authorized User
echo "Unit Test 7: Searching entry we added in Add test case (Authorized User)"
echo "Expected Result: Should Succeed"
echo "Hit Enter to continue"
read
curl -v -u $admindn:$adminpw http://$host:$port/vmdir/ldap/$new_user_dn_prefix/subtree/ > /tmp/search.result
echo
if grep -Fiq "$added_user_dn" /tmp/search.result
then
  echo "*Succeeded*"
else
  echo "*Failed*"
fi
echo
echo "Unit Test 7 Done. Hit Enter to continue"
read

# Unit Test 8: Search operation with Authorized User
echo
echo "Unit Test 8: Searching Admin entry (Authorized User)"
echo "Expected Result: Should Succeed"
echo "Hit Enter to continue"
read
curl -v -u $admindn:$adminpw http://$host:$port/vmdir/ldap/$admindn/subtree/ > /tmp/search.result
echo
if grep -Fiq "$admindn" /tmp/search.result
then
  echo "*Succeeded*"
else
  echo "*Failed*"
fi
echo
echo "Unit Test 8 Done. Hit Enter to continue"
read

# Unit Test 9: Search operation with Unauthorized User
echo
echo "Unit Test 9: Searching Admin entry (Unauthorized User)"
echo "Expected Result: Should Not Succeed"
echo "Hit Enter to continue"
read
curl -v -u $unauthorizeduserdn:$userpw http://$host:$port/vmdir/ldap/$admindn/subtree/ > /tmp/search.result
echo
if grep -Fiq "$admindn" /tmp/search.result
then
  echo "*Succeeded*"
else
  echo "*Failed*"
fi
echo
echo "Unit Test 9 Done. Hit Enter to continue"
read

# Unit Test 10: Search operation with Inavlid Target DN
echo
echo "Unit Test 10: Searching under Invalid Target DN"
echo "Expected Result: Should Not Succeed"
echo "Hit Enter to continue"
read
curl -v -u $unauthorizeduserdn:$userpw http://$host:$port/vmdir/ldap/$invalid_target_dn/subtree/
echo
echo "Unit Test 10 Done."
echo
echo "Search Operation Testing Done"
echo "Hit Enter to continue"
read

## PATCH OPERATION TESTING ##
echo
echo "Patch Operation testing started"
echo

# Unit Test 11: Patch operation with Authorized User (Entry exists)
echo "Unit Test 11: Modify entry which exists (Authorized User)"
echo "Expected Result: Should Succeed"
echo "Hit Enter to continue"
read
curl -X PATCH -d@$modify_user_json_file -v -u $admindn:$adminpw http://$host:$port/vmdir/ldap/$added_user_dn/
echo "Unit Test 11 Done. Hit Enter to continue"
read

# Unit Test 12: Patch operation with Authorized User (Entry doesn't exists)
echo
echo "Unit Test 12: Modify an Entry which doesn't exists (Authorized user)"
echo "Expected Result: Should Not Succeed"
echo "Hit Enter to continue"
read
curl -X PATCH -d@$modify_user_json_file -v -u $admindn:$adminpw http://$host:$port/vmdir/ldap/cn=randommm111212123,cn=Users,dc=vsphere,dc=local/
echo "Unit Test 12 Done. Hit Enter to continue"
read

# Unit Test 13: Perform PATCH operation with Unauthorized User (Entry already exists)
echo
echo "Unit Test 13: Modify an Entry which exists (Unauthorized User)"
echo "Expected Result: Access Should Be Denied"
echo "Hit Enter to continue"
read
curl -X PATCH -d@$modify_user_json_file -v -u $unauthorizeduserdn:$userpw http://$host:$port/vmdir/ldap/$added_user_dn/
echo "Unit Test 13 Done. Hit Enter to continue"
read

# Unit Test 14: Perform PATCH operation with Unauthorized User (Entry doesn't exists)
echo
echo "Unit Test 14: Modify new Entry (Unauthorized User)"
echo "Expected Result: Should Not Succeed"
echo "Hit Enter to continue"
read
curl -X PATCH -d@$modify_user_json_file -v -u $unauthorizeduserdn:$userpw http://$host:$port/vmdir/ldap/cn=randommm111212123,cn=Users,dc=vsphere,dc=local/
echo "Unit Test 14 Done. Hit Enter to continue"
read

# Unit Test 15: Perform PATCH operation with Invalid JSON (Entry existence doesn't matter)
echo
echo "Unit Test 15: Modify Entry with Invalid JSON (User authorization doesn't matter)"
echo "Expected Result: Should Not Succeed"
echo "Hit Enter to continue"
read
curl -X PATCH -d@$invalid_json_file -v -u $unauthorizeduserdn:$userpw http://$host:$port/vmdir/ldap/cn=randommm111212123,cn=Users,dc=vsphere,dc=local/
echo "Unit Test 15 Done. Hit Enter to continue"
read

# Unit Test 16: Perform PATCH operation on invalid attr (Authorized User)
echo
echo "Unit Test 16: Modify invalid Entry Attribute (Authorized User)"
echo "Expected Result: Should Not Succeed"
echo "Hit Enter to continue"
read
curl -X PATCH -d@$invalid_attr_patch_json -v -u $admindn:$adminpw http://$host:$port/vmdir/ldap/$added_user_dn/
echo "Unit Test 16 Done"
echo
echo "PATCH Operation Testing Done"
echo "Hit Enter to continue"
read

## DELETE OPERATION TESTING ##
echo
echo "Delete Operation testing started"
echo

# Unit Test 17: Delete operation with Unauthorized User (Entry exists)
echo "Unit Test 17: Delete entry which exists (Unauthorized User)"
echo "Expected Result: Access Should Be Denied"
echo "Hit Enter to continue"
read
curl -X DELETE -v -u $unauthorizeduserdn:$userpw http://$host:$port/vmdir/ldap/$added_user_dn/
echo "Unit Test 17 Done. Hit Enter to continue"
read

# Unit Test 18: Delete operation with Unauthorized User (Entry doesn't exists)
echo
echo "Unit Test 18: Delete an Entry which doesn't exists (Unauthorized user)"
echo "Expected Result: Should Not Succeed"
echo "Hit Enter to continue"
read
curl -X DELETE -v -u $unauthorizeduserdn:$userpw http://$host:$port/vmdir/ldap/cn=randommm111212123,cn=Users,dc=vsphere,dc=local/
echo "Unit Test 18 Done. Hit Enter to continue"
read

# Unit Test 19: Delete operation with Authorized User (Entry doesn't exists)
echo
echo "Unit Test 19: Delete an Entry which doesn't exists (Authorized User)"
echo "Expected Result: Should Not Succeed"
echo "Hit Enter to continue"
read
curl -X DELETE -v -u $admindn:$adminpw http://$host:$port/vmdir/ldap/cn=randommm111212123,cn=Users,dc=vsphere,dc=local/
echo "Unit Test 19 Done. Hit Enter to continue"
read

# Unit Test 20: Perform Delete operation with Authorized User (Entry exists)
echo
echo "Unit Test 20: Delete existing Entry (Authorized User)"
echo "Expected Result: Should Succeed"
echo "Hit Enter to continue"
read
curl -X DELETE -v -u $admindn:$adminpw http://$host:$port/vmdir/ldap/$added_user_dn/
echo "Unit Test 20 Done"
echo
echo "Delete Operation Testing Done"
echo "Hit Enter to continue"
read

## GET TOPOLOGY OPERATION TESTING ##
echo
echo "GetTopology Operation testing started"
echo

# Unit Test 21: GetTopology Operation with Authorized User
echo "Unit Test 21: GetTopology (Authorized User)"
echo "Expected Result: Should be able to see all entries for which user has access"
echo "Hit Enter to continue"
read
curl -v -u $admindn:$adminpw http://$host:$port/vmdir/manage/topology
echo
echo "Unit Test 21 Done. Hit Enter to continue"
read

# Unit Test 22: GetTopology operation with Unauthorized User
echo
echo "Unit Test 22: GetTopology (Unauthorized User)"
echo "Expected Result: Should not be able to see entries for which user are not authorized"
echo "Hit Enter to continue"
read
curl -v -u $unauthorizeduserdn:$userpw http://$host:$port/vmdir/manage/topology
echo
echo "Unit Test 22 Done"
echo
echo "GetTopology Operation Testing Done"
echo "Hit Enter to continue"
read

## GET COMPUTERS OPERATION TESTING ##
echo
echo "GetComputers Operation testing started"
echo

# Unit Test 23: GetComputers Operation with Authorized User
echo "Unit Test 23: GetComputers (Authorized User)"
echo "Expected Result: Should be able to see all entries for which user has access"
echo "Hit Enter to continue"
read
curl -v -u $admindn:$adminpw http://$host:$port/vmdir/manage/computers
echo
echo "Unit Test 23 Done. Hit Enter to continue"
read

# Unit Test 24: GetComputers operation with Unauthorized User
echo
echo "Unit Test 24: GetComputers (Unauthorized User)"
echo "Expected Result: Should not be able to see entries for which user are not authorized"
echo "Hit Enter to continue"
read
curl -v -u $unauthorizeduserdn:$userpw http://$host:$port/vmdir/manage/computers
echo
echo "Unit Test 24 Done"
echo
echo "GetComputers Operation Testing Done"
echo "Hit Enter to continue"
read

## GET DCINFO OPERATION TESTING ##
echo
echo "GetDCInfo Operation testing started"
echo

# Unit Test 25: GetDCInfo Operation with Authorized User
echo "Unit Test 25: GetDCInfo (Authorized User)"
echo "Expected Result: Should be able to see all entries for which user has access"
echo "Hit Enter to continue"
read
curl -v -u $admindn:$adminpw http://$host:$port/vmdir/manage/dc
echo
echo "Unit Test 25 Done. Hit Enter to continue"
read

# Unit Test 26: GetDCInfo operation with Unauthorized User
echo
echo "Unit Test 26: GetDCInfo (Unauthorized User)"
echo "Expected Result: Should not be able to see entries for which user are not authorized"
echo "Hit Enter to continue"
read
curl -v -u $unauthorizeduserdn:$userpw http://$host:$port/vmdir/manage/dc
echo
echo "Unit Test 26 Done"
echo
echo "GetDCInfo Operation Testing Done"
echo "Hit Enter to continue"
read

## GENERAL TEST CASES REALTED TO USER INPUT ##
echo
echo "Starting General testing related to received input data/request"
echo

# Unit Test 27: Wrong Method Name in URI
echo "Unit Test 27: Wrong method name in URI"
echo "Expected Result: Should Not Succeed"
echo "Hit Enter to continue"
read
curl -X XXXX -v -u $admindn:$adminpw http://$host:$port/vmdir/ldap/cn=randommm111212123,cn=Users,dc=vsphere,dc=local/
echo
echo "Unit Test 27 Done. Hit Enter to continue"
read

# Unit Test 28: Incorrect URI
echo
echo "Unit Test 28: Incorrect URI (Requesting resource which is not there/exposed)"
echo "Expected Result: Should Not Succeed"
echo "Hit Enter to continue"
read
curl -X DELETE -v -u $admindn:$adminpw http://$host:$port/vmdir/xxxx/ldap/$added_user_dn/
echo
echo "Unit Test 28 Done. Hit Enter to continue"
read

# Unit Test 29: Incorrect Password
echo
echo "Unit Test 29: Incorrect Password"
echo "Expected Result: Should Not Succeed"
echo "Hit Enter to continue"
read
curl -v -u $admindn:$incorrectpw http://$host:$port/vmdir/ldap/$admindn/subtree/
echo
echo "Unit Test 29 Done. Hit Enter to continue"
read

# Unit Test 30: Incorrect UserName
echo
echo "Unit Test 30: Incorrect UserName"
echo "Expected Result: Should Not Succeed"
echo "Hit Enter to continue"
read
curl -v -u $incorrectuser:$adminpw http://$host:$port/vmdir/ldap/$admindn/subtree/
echo
echo "Unit Test 30 Done."
echo
echo "General Testing Done"
echo "Hit Enter to continue"
read

## ReplNow OPERATION TESTING ##
echo
echo "ReplNow Operation testing started"
echo

# Unit Test 31: ReplNow Operation with Authorized User
echo "Unit Test 31: ReplNow (Authorized User, Just to make sure that operation succeed, requires manual check for replication)"
echo "Expected Result: Should Succeed"
echo "Hit Enter to continue"
read
curl -X POST -v -u $admindn:$adminpw http://$host:$port/vmdir/manage/replication
echo
echo "Unit Test 31 Done. Hit Enter to continue"
read

# Unit Test 32: ReplNow Operation with Unauthorized User
echo "Unit Test 32: ReplNow (Unauthorized User)"
echo "Expected Result: Shouldn't Succeed"
echo "Hit Enter to continue"
read
curl -X POST -v -u $unauthorizeduserdn:$userpw http://$host:$port/vmdir/manage/replication
echo
echo "Unit Test 32 Done. Done with ReplNow Testing. Hit Enter to continue"
read

# Unit Test 33: Add Replication Agreement With invalid input JSON
echo "Unit Test 32: Add RA with invalid JSON"
echo "Expected Result: Shouldn't Succeed"
echo "Hit Enter to continue"
read
curl -X PUT -d@$invalid_json_file -v -u $admindn:$adminpw http://$host:$port/vmdir/manage/replication
echo
echo "Unit Test 33 Done. Done with Add RA testing.  Hit Enter to continue"
read

# Unit Test 34: Remove RA Operation with invalid JSON
echo "Unit Test 34: Remove RA with Invalid JSON"
echo "Expected Result: Shouldn't Succeed"
echo "Hit Enter to continue"
read
curl -X DELETE -d@$invalid_json_file -v -u $admindn:$adminpw http://$host:$port/vmdir/manage/replication
echo
echo "Unit Test 34 Done. Done with Remove RA Testing. Hit Enter to continue"
read

echo
echo "Done with All testing"
exit 0
