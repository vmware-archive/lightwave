#!/bin/sh

#check environment vars
if [ -z "$LIGHTWAVE_DOMAIN" -o -z "$LIGHTWAVE_PASS" ]; then
  echo "Please set LIGHTWAVE_DOMAIN and LIGHTWAVE_PASS in .env file"
  exit 1
fi
#warn on missing aws kms params for ca security
if [ -z "$AWS_KMS_ARN" ]; then
  echo "Missing AWS_KMS_ARN in .env file. ca security plugin tests will fail."
fi

#prepare by installing rpms built in this build
rpm -Uvh --nodeps buildrpms/x86_64/lightwave-client*.rpm
rpm -Uvh --nodeps buildrpms/x86_64/lightwave-post*.rpm
rpm -Uvh --nodeps buildrpms/x86_64/lightwave-mutentca*.rpm
rpm -Uvh --nodeps buildrpms/x86_64/lightwave-casecurity-aws-kms-*.rpm

/opt/likewise/sbin/lwsmd --start-as-daemon
/opt/likewise/bin/lwsm autostart

primary=lightwave_lightwave-server_1

#wait for server to promote
response='000'
while [ $response -ne '404' ]; do
  response=$(curl -k --write-out %{http_code} --silent --output /dev/null https://$primary:7478)
  echo "waiting for $primary, response=$response"
  sleep 5
done

#join
/opt/vmware/bin/lightwave domain join --domain-controller $primary --domain $LIGHTWAVE_DOMAIN --password $LIGHTWAVE_PASS

#verify
/opt/vmware/bin/dir-cli nodes list --login Administrator@$LIGHTWAVE_DOMAIN --password $LIGHTWAVE_PASS --server-name $primary
#

#modify ca security config
sed -i 's/@@KEYSPEC@@/AES_256/' /opt/vmware/share/config/casecurity-aws-kms.json
sed -i 's|@@ARN@@|$AWS_KMS_ARN|' /opt/vmware/share/config/casecurity-aws-kms.json
#

#modify policy config
#using allow all config for rest api integration tests
cp scrips/mutentca-policy-allowall.json /opt/vmware/share/config/mutentca-policy-allowall.json
sed -i 's/mutentca-policy.json/mutentca-policy-allowall.json/g' /opt/vmware/share/config/mutentcaconfig.json
#

#restart post and mutentca
/opt/likewise/bin/lwsm stop mutentca
/opt/likewise/bin/lwsm stop post
/opt/likewise/bin/lwsm start post
/opt/likewise/bin/lwsm start mutentca
#

/bin/bash
