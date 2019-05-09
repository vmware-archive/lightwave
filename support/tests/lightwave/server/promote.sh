#!/bin/sh

wait_for_server()
{
  local lw_srv_node=$1

  local max_attempts=20
  local attempts=1
  local response='000'
  local http_ok='200'
  local wait_seconds=5

  while [ $response -ne $http_ok ] && [ $attempts -lt $max_attempts ]; do
    sleep $wait_seconds
    response=$(curl -k --write-out %{http_code} --silent --output /dev/null https://$lw_srv_node)
    echo "waiting for $lw_srv_node, response=$response [ $attempts/$max_attempts ]"
    attempts=$[attempts+1]
  done

  elapsed=$[$attempts*$wait_seconds]

  if [ $response -eq $http_ok ]; then
    echo "$lw_srv_node up in $elapsed seconds."
  else
    echo "Waited $elapsed seconds. Giving up. Expected $http_ok. Got $response"
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
rpm -Uvh --nodeps buildrpms/x86_64/lightwave-1*.rpm

# Need haveged for vmca
/usr/sbin/haveged -w 1024 -v 1

/opt/likewise/sbin/lwsmd --start-as-daemon

#set multiplesan option so localhost can be added to cert
#cannot use cli for this before promote
#if this is not done before promote, steps will get complicated
#and involve a set via cli, ca restart, regenerate cert to include localhost
/opt/likewise/bin/lwregshell add_value \
  '[HKEY_THIS_MACHINE\Services\vmca]' "ServerOption" REG_DWORD 0x1

/opt/likewise/bin/lwsm autostart
sleep 1

if [ $HOSTNAME = $LIGHTWAVE_NODE_1 ]
then
  /opt/vmware/bin/configure-lightwave-server \
    --domain $LIGHTWAVE_DOMAIN \
    --password $LIGHTWAVE_PASS \
    --ssl-subject-alt-name "${HOSTNAME},localhost"

  /opt/likewise/bin/lwsm restart vmdir
  /opt/likewise/bin/lwsm restart vmca
else

  wait_for_server $LIGHTWAVE_NODE_1

  /opt/vmware/bin/configure-lightwave-server \
    --domain $LIGHTWAVE_DOMAIN \
    --password $LIGHTWAVE_PASS \
    --server $LIGHTWAVE_NODE_1 \
    --ssl-subject-alt-name "${HOSTNAME},localhost"

  /opt/likewise/bin/lwsm restart vmdir
  /opt/likewise/bin/lwsm restart vmca

  LW_NODE_NUM=$(/opt/vmware/bin/vdcrepadmin  -f showservers -h localhost -u administrator -w $LIGHTWAVE_PASS | wc -l)
  if [ $LW_NODE_NUM -ne 2 ]
  then
    echo "Cluster size != 2"
    exit 4
  fi
fi

/opt/vmware/sbin/vmware-stsd.sh start

/bin/bash
