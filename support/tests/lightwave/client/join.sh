#!/bin/sh

#check environment vars
if [ -z "$LIGHTWAVE_DOMAIN" -o -z "$LIGHTWAVE_PASS" ]; then
  echo "Please set LIGHTWAVE_DOMAIN and LIGHTWAVE_PASS in .env file"
  exit 1
fi

#prepare by installing rpms built in this build
rpm -Uvh --nodeps buildrpms/x86_64/lightwave-client*.rpm
rpm -Uvh --nodeps buildrpms/x86_64/lightwave-sts*.rpm

primary=lightwave_lightwave-server_1

#wait for server to promote
response=1
while [ $response -ne 0 ]; do
  netcat -v -z $primary 636 
  response=$?
  echo "waiting for $primary, response=$response"
  sleep 5
done

#join
/opt/vmware/bin/ic-join --domain-controller $primary --domain $LIGHTWAVE_DOMAIN --password $LIGHTWAVE_PASS > /tmp/join.out 2>&1

#verify
/opt/vmware/bin/dir-cli nodes list --login Administrator@$LIGHTWAVE_DOMAIN --password $LIGHTWAVE_PASS --server-name $primary >> /tmp/join.out 2>&1

echo "LDAP SRV record query ..."
nslookup -q=SRV _ldap._tcp.$LIGHTWAVE_DOMAIN.  >> /tmp/join.out 2>&1

# install STS
/opt/vmware/bin/configure-sts --username administrator --password $LIGHTWAVE_PASS --domain $LIGHTWAVE_DOMAIN >> /tmp/join.out 2>&1
if [ $? -ne 0 ]; then
  echo "Failed to configure STS"
  exit 2
fi

echo "starting sts ..."
# start sts
/opt/vmware/sbin/vmware-stsd.sh start >> /tmp/join.out 2>&1

#
/bin/bash
