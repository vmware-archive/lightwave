#!/bin/bash

source ./header.sh

REL_DIR=`dirname $0`
cd $REL_DIR
TESTING_PATH=`pwd`
RESULT_PATH=$TESTING_PATH/results
RESULT_DIFF_PATH=$RESULT_PATH/diff

if  [ -d $RESULT_PATH ]; then
    rm -rf $RESULT_PATH
fi

mkdir $RESULT_PATH
mkdir $RESULT_DIFF_PATH

$TESTING_PATH/data/generate_data.sh > $TESTING_PATH/data/data.ldif
echo $TESTING_PATH/data/data.ldif

echo "Running modify_test"
# Create 100 test objects
ldapadd -c -h $host -p $port -x -D "cn=administrator,cn=users,dc=vmware,dc=com" -w 123 -f $TESTING_PATH/data/data.ldif > $RESULT_PATH/addObjects.out1 2>&1
# Modify test scripts
chmod +x $TESTING_PATH/modify_tests/test_script1.sh
$TESTING_PATH/modify_tests/test_script1.sh -h $host -p $port > $RESULT_PATH/modify_tests_output.txt 2>&1
diff $TESTING_PATH/modify_tests/good_output.txt  $RESULT_PATH/modify_tests_output.txt > $RESULT_DIFF_PATH/diff_modify.output
# Delete 100 objects
echo "Running delete_test after modify"
cd $TESTING_PATH/delete_tests
$TESTING_PATH/delete_tests/test_script1.sh -h $host -p $port> $RESULT_PATH/delete_tests_output_modify.txt 2>&1
diff $TESTING_PATH/delete_tests/good_output.txt  $RESULT_PATH/delete_tests_output_modify.txt > $RESULT_DIFF_PATH/diff_del_after_modify.output

echo "Running search_test"
# Re-Create 100 test objects
ldapadd -c -h $host -p $port -x -D "cn=administrator,cn=users,dc=vmware,dc=com" -w 123 -f $TESTING_PATH/data/data.ldif > $RESULT_PATH/addObjects.out2 2>&1
# Search test scripts
chmod +x $TESTING_PATH/search_tests/test_script1.sh
$TESTING_PATH/search_tests/test_script1.sh -h $host -p $port > $RESULT_PATH/search_tests_output.txt 2>&1
diff $TESTING_PATH/search_tests/good_output.txt  $RESULT_PATH/search_tests_output.txt > $RESULT_DIFF_PATH/diff_search.output
chmod +x $TESTING_PATH/search_tests/test_matching_rule.sh
$TESTING_PATH/search_tests/test_matching_rule.sh -h $host -p $port > $RESULT_PATH/search_matching_rule_tests_output.txt 2>&1
diff $TESTING_PATH/search_tests/good_output_matching_rule.txt  $RESULT_PATH/search_matching_rule_tests_output.txt > $RESULT_DIFF_PATH/diff_search_matching_rule.output
# Delete 100 objects
echo "Running delete_test after search"
cd $TESTING_PATH/delete_tests
$TESTING_PATH/delete_tests/test_script1.sh -h $host -p $port > $RESULT_PATH/delete_tests_output_search.txt 2>&1
diff $TESTING_PATH/delete_tests/good_output.txt  $RESULT_PATH/delete_tests_output_search.txt > $RESULT_DIFF_PATH/diff_del_after_search.output


echo "Running group_test"
# Re-Create 100 test objects
ldapadd -c -h $host -p $port -x -D "cn=administrator,cn=users,dc=vmware,dc=com" -w 123 -f $TESTING_PATH/data/data.ldif > $RESULT_PATH/addObjects.out3 2>&1
# Group members test scripts
chmod +x $TESTING_PATH/group_tests/test_script1.sh
$TESTING_PATH/group_tests/test_script1.sh -h $host -p $port > $RESULT_PATH/group_tests_output.txt 2>&1
diff $TESTING_PATH/group_tests/good_output.txt  $RESULT_PATH/group_tests_output.txt > $RESULT_DIFF_PATH/diff_group.output
# Delete 100 objects
echo "Running delete_test after group"
cd $TESTING_PATH/delete_tests
$TESTING_PATH/delete_tests/test_script1.sh -h $host -p $port > $RESULT_PATH/delete_tests_output_group.txt 2>&1
diff $TESTING_PATH/delete_tests/good_output_after_group.txt  $RESULT_PATH/delete_tests_output_group.txt > $RESULT_DIFF_PATH/diff_del_after_group.output


echo "Running acl_test"
# special acl objects with password information
$TESTING_PATH/acl_tests/generate_data.sh > $TESTING_PATH/acl_tests/data.ldif
# Re-Create 100 test objects
ldapadd -c -h $host -p $port -x -D "cn=administrator,cn=users,dc=vmware,dc=com" -w 123 -f $TESTING_PATH/acl_tests/data.ldif > $RESULT_PATH/addObjects.out4 2>&1
# ACL test scripts
chmod +x $TESTING_PATH/acl_tests/test_script1.sh
$TESTING_PATH/acl_tests/test_script1.sh -h $host -p $port > $RESULT_PATH/acl_tests_output.txt 2>&1
diff $TESTING_PATH/acl_tests/good_output.txt  $RESULT_PATH/acl_tests_output.txt > $RESULT_DIFF_PATH/diff_acl.output
# ACL tenant related test
chmod +x $TESTING_PATH/acl_tests/test_script1_tenant.sh
cd $TESTING_PATH/acl_tests
$TESTING_PATH/acl_tests/test_script1_tenant.sh -h $host -p $port > $RESULT_PATH/acl_tenant_tests_output.txt 2>&1
diff $TESTING_PATH/acl_tests/good_output_tenant.txt  $RESULT_PATH/acl_tenant_tests_output.txt > $RESULT_DIFF_PATH/diff_acl_tenant.output
# Delete 100 objects
echo "Running delete_test after acl"
cd $TESTING_PATH/delete_tests
$TESTING_PATH/delete_tests/test_script1.sh -h $host -p $port > $RESULT_PATH/delete_tests_output_acl.txt 2>&1
diff $TESTING_PATH/delete_tests/good_output.txt  $RESULT_PATH/delete_tests_output_acl.txt > $RESULT_DIFF_PATH/diff_del_after_acl.output


echo "Running passwords_test"
# PASSWORDS test scripts
chmod +x $TESTING_PATH/password_tests/test_script1.sh
$TESTING_PATH/password_tests/test_script1.sh -h $host -p $port > $RESULT_PATH/password_tests_output.txt 2>&1
diff $TESTING_PATH/password_tests/good_output.txt  $RESULT_PATH/password_tests_output.txt > $RESULT_DIFF_PATH/diff_password.output


echo "Running schema_test (first boot)"
# Schema test scripts (first boot)
chmod +x $TESTING_PATH/schema_tests/test_script1.sh
$TESTING_PATH/schema_tests/test_script1.sh -h $host -p $port > $RESULT_PATH/schema_firstboot_tests_output.txt 2>&1
diff $TESTING_PATH/schema_tests/good_output_firstboot.txt  $RESULT_PATH/schema_firstboot_tests_output.txt > $RESULT_DIFF_PATH/diff_schema_firstboot.output
# Restart vmdir
/opt/likewise/bin/lwsm restart vmdir
echo "Running schema_test (subsequent boot)"
# Schema test scripts (subsequent boot)
chmod +x $TESTING_PATH/schema_tests/test_script1.sh
$TESTING_PATH/schema_tests/test_script1.sh -h $host -p $port > $RESULT_PATH/schema_subsequentboot_tests_output.txt 2>&1
diff $TESTING_PATH/schema_tests/good_output_subsequentboot.txt  $RESULT_PATH/schema_subsequentboot_tests_output.txt > $RESULT_DIFF_PATH/diff_schema_subsequentboot.output


# Clean up
rm $TESTING_PATH/data/data.ldif
rm $TESTING_PATH/acl_tests/data.ldif

echo "Please check output in 'results' directory."
ls -l $RESULT_DIFF_PATH/diff_*.output
