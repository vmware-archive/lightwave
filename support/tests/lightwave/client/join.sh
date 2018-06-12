#!/bin/sh

#prepare by installing rpms built in this build
rpm -Uvh --nodeps buildrpms/x86_64/lightwave-client*.rpm

/opt/likewise/sbin/lwsmd --start-as-daemon
/opt/likewise/bin/lwsm start vmafd

primary=lightwave_lightwave-server_1
#wait for server to promote
response='000'
while [ $response -ne '404' ]; do
  response=$(curl -k --write-out %{http_code} --silent --output /dev/null https://$primary:7478)
  echo "waiting for $primary, response=$response"
  sleep 5
done

#join
/opt/vmware/bin/ic-join --domain-controller $primary --domain $LIGHTWAVE_DOMAIN --password $LIGHTWAVE_PASS

#verify
/opt/vmware/bin/dir-cli nodes list --login Administrator@$LIGHTWAVE_DOMAIN --password $LIGHTWAVE_PASS --server-name $primary
#
/bin/bash
