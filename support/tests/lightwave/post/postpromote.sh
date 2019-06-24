#!/bin/sh

promote_second_node()
{
  local max_attempts=20
  local attempts=1
  local wait_seconds=5

  while [ $attempts -lt $max_attempts ]; do
    netcat -z -v ${POST_NODE_1} 38900
    if [ $? -ne 0 ]; then
      echo "waiting for $POST_NODE_1 to start"
    else
      break
    fi

    sleep $wait_seconds
    attempts=$[attempts+1]
  done

  elapsed=$[$attempts*$wait_seconds]

  if [ $attempts -eq $max_attempts ]; then
    echo "Waited $elapsed seconds for $POST_NODE_1 to start. Giving up"
    exit 1
  else
    sleep 3
    /opt/vmware/bin/post-cli node promote \
      --partner-name ${POST_NODE_1} \
      --password ${POST_PASS} \
      --host-name ${HOSTNAME}
  fi
}

#check environment vars
if [ -z "$POST_DOMAIN" -o -z "$POST_PASS" ]; then
  echo "Please set POST_DOMAIN and LIGHTWAVE_PASS in .env file"
  exit 1
fi

POST_NODE_1=server.$POST_DOMAIN

#if a hostname variable is not already set, set one
if [ -z "$HOSTNAME" ]; then
  echo "environment variable HOSTNAME is not set"
  exit 1
fi

#prepare by installing rpms built in this build
rpm -Uvh --nodeps buildrpms/x86_64/lightwave-client*.rpm
rpm -Uvh --nodeps buildrpms/x86_64/lightwave-post*.rpm

/usr/sbin/haveged -w 1024 -v 1

/opt/likewise/sbin/lwsmd --start-as-daemon

/opt/likewise/bin/lwsm autostart
sleep 1

# TODO, join post nodes to lightwave

if [ $HOSTNAME = $POST_NODE_1 ]
then
  /opt/vmware/bin/post-cli node promote \
    --domain-name ${POST_DOMAIN} \
    --password ${POST_PASS} \
    --host-name ${HOSTNAME}
else
    promote_second_node
fi

/bin/bash
