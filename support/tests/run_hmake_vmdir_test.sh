#!/bin/bash
#this is a continuation of run_hmake_sanity so the test container is the same
#Assumptions:
#1. env file is available at $LIGHTWAVE_ENV_FILE
#2. test setup with a primary and partner is available
#3. client rpms are installed

#source env variables
source $LIGHTWAVE_ENV_FILE

#check environment vars
if [ -z "$LIGHTWAVE_DOMAIN" -o -z "$LIGHTWAVE_PASS" ]; then
  echo "Please set LIGHTWAVE_DOMAIN and LIGHTWAVE_PASS in .env file"
  exit 1
fi

primary=server.$LIGHTWAVE_DOMAIN

echo "VmDir integration install test rpm"
rpm -i /src/build/rpmbuild/RPMS/x86_64/lightwave-test*.rpm

echo "VmDir integration test start" `date`
/opt/vmware/test/vmdir/bin/vmdir_test_runner \
	-H $primary \
	-u administrator \
	-w $LIGHTWAVE_PASS \
	-d $LIGHTWAVE_DOMAIN \
	-t /opt/vmware/test/vmdir/lib64

if [ $? -ne 0 ]; then
  # TODO, should exit 1
  echo "VmDir integration test failed" `date`
else
  echo "VmDir integration test passed" `date`
fi

