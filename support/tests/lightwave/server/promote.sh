#!/bin/sh

#check environment vars
if [ -z "$LIGHTWAVE_DOMAIN" -o -z "$LIGHTWAVE_PASS" ]; then
  echo "Please set LIGHTWAVE_DOMAIN and LIGHTWAVE_PASS in .env file"
  exit 1
fi

LIGHTWAVE_NODE_1=server.$LIGHTWAVE_DOMAIN

#if a hostname variable is not already set, set one
if [ -z "$LIGHTWAVE_HOSTNAME" ]; then
  LIGHTWAVE_HOSTNAME=$LIGHTWAVE_NODE_1
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

if [ $LIGHTWAVE_HOSTNAME = $LIGHTWAVE_NODE_1 ]
then
  /opt/vmware/bin/configure-lightwave-server \
    --domain $LIGHTWAVE_DOMAIN \
    --password $LIGHTWAVE_PASS \
    --ssl-subject-alt-name "${LIGHTWAVE_HOSTNAME},localhost"
else
  /opt/vmware/bin/configure-lightwave-server \
    --domain $LIGHTWAVE_DOMAIN \
    --password $LIGHTWAVE_PASS \
    --server $LIGHTWAVE_NODE_1 \
    --ssl-subject-alt-name "${LIGHTWAVE_HOSTNAME},localhost"
fi

/opt/likewise/bin/lwsm restart vmdir

/opt/likewise/bin/lwsm restart vmca

/opt/vmware/sbin/vmware-stsd.sh start

/bin/bash
