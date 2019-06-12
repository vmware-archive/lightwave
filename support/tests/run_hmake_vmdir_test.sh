#!/bin/bash
#this is a continuation of run_hmake_sanity so the test container is the same
#Assumptions:
#1. env file is available at $LIGHTWAVE_ENV_FILE
#2. test setup with a primary and partner is available
#3. client rpms are installed

start_integrity_check()
{
  local lw_srv_node=$1

  ldapsearch -h $lw_srv_node -p 389 -Y SRP -U administrator@$LIGHTWAVE_DOMAIN -w $LIGHTWAVE_PASS \
      -b "cn=integritycheckstatus" -s base "objectclass=*" start > /dev/null 2>&1
}

waitfor_integrity_check()
{
  local lw_srv_node=$1

  while [ 1 ]; do
    ldapsearch -o ldif-wrap=no -h $lw_srv_node -p 389 -Y SRP \
        -U administrator@$LIGHTWAVE_DOMAIN -w $LIGHTWAVE_PASS \
        -b "cn=integritycheckstatus" -s one "objectclass=*"  vmwServerRunTimeStatus 2>&1 \
        | grep -i "^vmwServerRunTimeStatus" > /tmp/$lw_srv_node.integrityCheck.log

    LW_DONE=$(grep ": end time:" /tmp/$lw_srv_node.integrityCheck.log | wc -l)
    if [ $LW_DONE -eq 1 ]; then
      break;
    fi

    sleep 2;
  done
}

validate_integrity_check()
{
  local lw_srv_node=$1
  local lw_partner_node=$2

  LW_RESULT=$(grep "digest mismatch (0), missing entry (0)" /tmp/$lw_srv_node.integrityCheck.log | grep -i $lw_partner_node | wc -l)

  if [ $LW_RESULT -ne 1 ]; then
    echo FALSE
  else
    echo ""
  fi
}


#source env variables
source $LIGHTWAVE_ENV_FILE

#check environment vars
if [ -z "$LIGHTWAVE_DOMAIN" -o -z "$LIGHTWAVE_PASS" ]; then
  echo "Please set LIGHTWAVE_DOMAIN and LIGHTWAVE_PASS in .env file"
  exit 1
fi

LW_N1=server.$LIGHTWAVE_DOMAIN
LW_N2=server-n2.$LIGHTWAVE_DOMAIN

echo "VmDir integration install test rpm"
rpm -i /src/build/rpmbuild/RPMS/x86_64/lightwave-test*.rpm

echo "VmDir integration test start" `date`
/opt/vmware/test/vmdir/bin/vmdir_test_runner \
	-H $LW_N1 \
	-u administrator \
	-w $LIGHTWAVE_PASS \
	-d $LIGHTWAVE_DOMAIN \
	-k \
	-r \
	-t /opt/vmware/test/vmdir/lib64

if [ $? -ne 0 ]; then
  # TODO, should exit 2
  echo "VmDir integration test failed" `date`
else
  echo "VmDir integration test passed" `date`
fi

# let replication sync
sleep 5

echo "start integrity check" `date`
start_integrity_check $LW_N1
start_integrity_check $LW_N2

echo "wait for integrity check" `date`
waitfor_integrity_check $LW_N1
waitfor_integrity_check $LW_N2

echo "validate integrity check" `date`
NODE_1_INTEGRITY="$(validate_integrity_check $LW_N1 $LW_N2)"
NODE_2_INTEGRITY="$(validate_integrity_check $LW_N2 $LW_N1)"

echo ========== $LW_N1 integrity report ==========
cat /tmp/$LW_N1.integrityCheck.log
echo ========== $LW_N2 integrity report ==========
cat /tmp/$LW_N2.integrityCheck.log

if [ -n "${NODE_1_INTEGRITY}" -o -n "${NODE_2_INTEGRITY}" ]; then
  # TODO, should exit 3
  echo "VmDir integrity check failed"
else
  echo "VmDir integrity check passed"
fi
