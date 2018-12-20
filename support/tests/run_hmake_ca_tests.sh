#!/bin/bash
#Assumptions:
#1. env file is available at $LIGHTWAVE_ENV_FILE
#2. topsrcdir is mounted as /src
#3. ca tests need a lightwave server with policy config loaded
#How to run this test
#1. Pre-requisites: docker, hmake docker-compose
#2. Invoke test: hmake test-ca
#3. To skip build and run with built rpms: hmake -S pack -S build -S build-lightwave-photon2 test-ca

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
carestport=7778
capath=/etc/ssl/certs

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
  exit 1
fi

#
mkdir -p /etc/vmware/vmware-vmafd

#this join is going to use ldap to communicate to directory
#for the join process but it will use rest for
#certficate and password refresh.
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
cat > /tmp/csr_should_pass.json << EOF
{
  "common_name":    "${THIS_HOST_NAME}",
  "country":        "US",
  "state":          "WA",
  "organization":   "",
  "domain_name":    "${LIGHTWAVE_DOMAIN}"
}
EOF

#make sure we have certificates installed to trust the server
#ca get-cert command uses REST to talk to the ca server.
#note that curl does not use -k this time.
max_attempts=5
attempts=1
exit_code=1

while [ $exit_code -ne 0 ] && [ $attempts -lt $max_attempts ]; do
  response=$(curl --capath $capath --write-out %{http_code} --silent --output /dev/null https://$primary:$carestport)
  exit_code=$?
  echo "$exit_code : waiting for certificate refresh, response=$response [ $attempts/$max_attempts ]"
  sleep $wait_seconds
  attempts=$[attempts+1]
done

elapsed=$[$attempts*$wait_seconds]

if [ $exit_code -eq 0 ]; then
  echo "Certificates refreshed in $elapsed seconds. running cert tests.."
else
  echo "Waited $elapsed seconds. Giving up. Expected curl to pass with exit code 0. Got $exit_code ."
  exit 1
fi

#get-cert call should pass
/opt/vmware/bin/lightwave \
ca get-cert \
--config /tmp/csr_should_pass.json \
--privkey /tmp/csr.key \
--cert /tmp/csr_should_pass.crt

openssl x509 -noout -text -in /tmp/csr_should_pass.crt

#read an existing key. get-cert call should pass
/opt/vmware/bin/lightwave \
ca get-cert \
--config /tmp/csr_should_pass.json \
--privkey /tmp/csr.key \
--cert /tmp/csr_should_pass2.crt

openssl x509 -noout -text -in /tmp/csr_should_pass2.crt
