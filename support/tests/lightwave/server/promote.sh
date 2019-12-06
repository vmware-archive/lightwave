#!/bin/sh

wait_for_server()
{
  local lw_srv_node=$1

  local max_attempts=20
  local attempts=1
  local response=1
  local nc_ok=0
  local wait_seconds=5

  while [ $response -ne 0 ] && [ $attempts -lt $max_attempts ]; do
    sleep $wait_seconds
    netcat -v -z $lw_srv_node 636
    response=$?
    echo "waiting for $lw_srv_node, response=$response [ $attempts/$max_attempts ]"
    attempts=$[attempts+1]
  done

  elapsed=$[$attempts*$wait_seconds]

  if [ $response -eq $nc_ok ]; then
    echo "$lw_srv_node up in $elapsed seconds."
  else
    echo "Waited $elapsed seconds. Giving up. Expected $nc_ok. Got $response"
    exit 3
  fi
}

#check environment vars
if [ -z "$LIGHTWAVE_DOMAIN" -o -z "$LIGHTWAVE_PASS" ]; then
  echo "Please set LIGHTWAVE_DOMAIN and LIGHTWAVE_PASS in .env file"
  exit 1
fi

LIGHTWAVE_NODE_1=server.$LIGHTWAVE_DOMAIN

if [ -z "$HOSTNAME" ]; then
  echo 'environemnt variable $HOSTNAME is not set'
  exit 2
fi

#prepare by installing rpms built in this build
rpm -Uvh --nodeps buildrpms/x86_64/lightwave-client*.rpm
rpm -Uvh --nodeps buildrpms/x86_64/lightwave-server*.rpm

# Need haveged for vmca
/usr/sbin/haveged -w 1024 -v 1

# set longer idle connection timeout to allow long running intergration test
/opt/vmware/bin/lwcommon-cli regcfg set-key /vmdir/parameters/LdapRecvTimeoutSec 1024

# set small tombstone reaping keys for testing
/opt/vmware/bin/lwcommon-cli regcfg set-key /vmdir/parameters/TombstoneExpirationPeriodInSec 5
/opt/vmware/bin/lwcommon-cli regcfg set-key /vmdir/parameters/TombstoneReapingThreadFreqInSec 10

/opt/vmware/bin/lwcommon-cli regcfg set-key /vmdir/parameters/EnableRename 1

# set vmdir key for serach test cases

# MaxIndexScan default 32/8192/512,  set to 1024
# Max number of index scan per table.
/opt/vmware/bin/lwcommon-cli regcfg set-key /vmdir/parameters/MaxIndexScan 1024

# SmallCandidateSet default 16/8192/32,  set to 128
# Stop index scan to build candiate list if we have a "Small" positive scan CL
/opt/vmware/bin/lwcommon-cli regcfg set-key /vmdir/parameters/SmallCandidateSet 128

# MaxIterationScan default 0/MAX/10000,  set to 512+128=640
# For non-admin users, limit the iteration allowed.
/opt/vmware/bin/lwcommon-cli regcfg set-key /vmdir/parameters/MaxSearchIterationScan 640

# MaxIterationScanTxn default 0/50000/2000, set to 64
# To avoid long lasting read transaction, vmdir refreshes transaction after X number of table iteration.
/opt/vmware/bin/lwcommon-cli regcfg set-key /vmdir/parameters/MaxSearchIterationScanTxn 64

#set multiplesan option so localhost can be added to cert
#cannot use cli for this before promote
#if this is not done before promote, steps will get complicated
#and involve a set via cli, ca restart, regenerate cert to include localhost
/opt/vmware/bin/lwcommon-cli regcfg set-key /vmca/ServerOption 1

if [ $HOSTNAME = $LIGHTWAVE_NODE_1 ]
then
  /opt/vmware/bin/configure-lightwave-server \
    --domain $LIGHTWAVE_DOMAIN \
    --password $LIGHTWAVE_PASS \
    --ssl-subject-alt-name "${HOSTNAME},localhost"  --disable-sts > /tmp/configureLW.out 2>&1

  # provision schema for integration_tests/search
  /opt/vmware/bin/vdcschema patch-schema-defs \
      --file /scripts/vmdir_search_test_schema.ldif \
      --domain $LIGHTWAVE_DOMAIN \
      --host localhost \
      --login administrator \
      --passwd $LIGHTWAVE_PASS

  # add default SD to objectclass vmwsearchtest, so authenticated user can read entries of this objectclass
  # TODO, should add this feature to vdcschema tool
  ldapmodify -h localhost -Y SRP -U administrator@$LIGHTWAVE_DOMAIN -w $LIGHTWAVE_PASS  <<EOF
dn: cn=vmwsearchtest,cn=schemacontext
changetype: modify
add: defaultsecuritydescriptor
defaultSecurityDescriptor: D:(A;;RC;;;S-1-0-0-545)(A;;RP;;;S-1-0-0-515)
EOF

  /opt/vmware/sbin/vmware-vmdird.sh stop
  /opt/vmware/sbin/vmware-vmdird.sh start

  /opt/vmware/sbin/vmware-vmcad.sh stop
  /opt/vmware/sbin/vmware-vmcad.sh start
else

  wait_for_server $LIGHTWAVE_NODE_1

  # safe buffer for node 1 vmdir to restart
  sleep 15

  /opt/vmware/bin/configure-lightwave-server \
    --domain $LIGHTWAVE_DOMAIN \
    --password $LIGHTWAVE_PASS \
    --server $LIGHTWAVE_NODE_1 \
    --ssl-subject-alt-name "${HOSTNAME},localhost"  --disable-sts > /tmp/configureLW.out 2>&1

  /opt/vmware/sbin/vmware-vmdird.sh stop
  /opt/vmware/sbin/vmware-vmdird.sh start

  /opt/vmware/sbin/vmware-vmcad.sh stop
  /opt/vmware/sbin/vmware-vmcad.sh start

  LW_NODE_NUM=$(/opt/vmware/bin/vdcrepadmin  -f showservers -h localhost -u administrator -w $LIGHTWAVE_PASS | wc -l)
  if [ $LW_NODE_NUM -ne 2 ]
  then
    echo "Cluster size != 2"
    exit 4
  fi
fi

/bin/bash
