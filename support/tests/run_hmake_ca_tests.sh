#!/bin/bash
#Assumptions:
#1. env file is available at $LIGHTWAVE_ENV_FILE
#2. topsrcdir is mounted as /src
#3. ca tests need a lightwave server with policy config loaded

#source env variables
source $LIGHTWAVE_ENV_FILE

#check environment vars
if [ -z "$LIGHTWAVE_DOMAIN" -o -z "$LIGHTWAVE_PASS" ]; then
  echo "Please set LIGHTWAVE_DOMAIN and LIGHTWAVE_PASS in .env file"
  exit 1
fi

#prepare by installing rpms built in this build
rpm -Uvh --nodeps /src/build/rpmbuild/RPMS/x86_64/lightwave-client*.rpm

/opt/likewise/sbin/lwsmd --start-as-daemon

primary=caserver.$LIGHTWAVE_DOMAIN
THIS_HOST_NAME=`hostname`

/opt/likewise/bin/lwsm autostart
sleep 1

#wait for server to promote
max_attempts=20
attempts=1
response='000'
http_ok='200'
wait_seconds=5

while [ $response -ne $http_ok ] && [ $attempts -lt $max_attempts ]; do
  response=$(curl -k --write-out %{http_code} --silent --output /dev/null https://$primary)
  echo "waiting for $primary, response=$response [ $attempts/$max_attempts ]"
  sleep $wait_seconds
  attempts=$[attempts+1]
done

elapsed=$[$attempts*$wait_seconds]

if [ $response -eq $http_ok ]; then
  echo "Lightwave up in $elapsed seconds. Joining a node and acquiring token.."
else
  echo "Waited $elapsed seconds. Giving up. Expected $http_ok. Got $response"
fi

#this join is going to use ldap to communicate to directory
#for the join process but it will use rest for
#cert refresh and pass refresh.
/opt/vmware/bin/ic-join \
  --domain-controller $primary \
  --domain $LIGHTWAVE_DOMAIN \
  --password $LIGHTWAVE_PASS

#verify this joined
/opt/vmware/bin/dir-cli nodes list \
--login Administrator@$LIGHTWAVE_DOMAIN \
--password $LIGHTWAVE_PASS \
--server-name $primary

#set up csr for a get-cert call that should pass
cat > csr_should_pass.json << EOF
{
  "common_name":    "${THIS_HOST_NAME}",
  "country":        "US",
  "state":          "WA",
  "organization":   "",
  "domain_name":    "${LIGHTWAVE_DOMAIN}"
}
EOF

#get-cert call should pass
/opt/vmware/bin/lightwave \
ca get-cert \
--config csr_should_pass.json \
--privkey keys\csr.key > csr_should_pass.crt

openssl x509 -noout -text -in csr_should_pass.crt
