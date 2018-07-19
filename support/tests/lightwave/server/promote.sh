#!/bin/sh

#prepare by installing rpms built in this build
rpm -Uvh --nodeps buildrpms/x86_64/lightwave-client*.rpm
rpm -Uvh --nodeps buildrpms/x86_64/lightwave-server*.rpm
rpm -Uvh --nodeps buildrpms/x86_64/lightwave-1*.rpm

/opt/likewise/sbin/lwsmd --start-as-daemon

#set multiplesan option so localhost can be added to cert
#cannot use cli for this before promote
#if this is not done before promote, steps will get complicated
#and involve a set via cli, ca restart, regenerate cert to include localhost
/opt/likewise/bin/lwregshell add_value \
  '[HKEY_THIS_MACHINE\Services\vmca]' "ServerOption" REG_DWORD 0x1

/opt/likewise/bin/lwsm autostart
sleep 1

/opt/vmware/bin/configure-lightwave-server \
  --domain $LIGHTWAVE_DOMAIN \
  --password $LIGHTWAVE_PASS \
  --ssl-subject-alt-name "server.${LIGHTWAVE_DOMAIN},localhost"

/opt/vmware/sbin/vmware-stsd.sh start

/bin/bash
