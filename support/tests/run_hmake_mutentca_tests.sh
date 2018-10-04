#!/bin/bash
#Assumptions:
#1. env file is available at $LIGHTWAVE_ENV_FILE
#2. test setup with a lwserver and mutentca is available

#source env variables
source $LIGHTWAVE_ENV_FILE

#check environment vars
if [ -z "$LIGHTWAVE_DOMAIN" -o -z "$LIGHTWAVE_PASS" ]; then
  echo "Please set LIGHTWAVE_DOMAIN and LIGHTWAVE_PASS in .env file"
  exit 1
fi

#prepare by installing rpms built in this build
echo "Installing lightwave client"
rpm -Uvh --nodeps /src/build/rpmbuild/RPMS/x86_64/lightwave-client*.rpm

/opt/likewise/sbin/lwsmd --start-as-daemon
/opt/likewise/bin/lwsm autostart

lwserver=server.$LIGHTWAVE_DOMAIN
mutentca=mutentca.$LIGHTWAVE_DOMAIN

#wait for server to promote
max_attempts=20
attempts=1
response='000'
http_ok='200'
wait_seconds=5

while [ $response -ne $http_ok ] && [ $attempts -lt $max_attempts ]; do
  response=$(curl -k --write-out %{http_code} --silent --output /dev/null https://$lwserver)
  echo "waiting for $lwserver, response=$response [ $attempts/$max_attempts ]"
  sleep $wait_seconds
  attempts=$[attempts+1]
done

elapsed=$[$attempts*$wait_seconds]

if [ $response -eq $http_ok ]; then
  echo "Lightwave up in $elapsed seconds. Joining the client to the server."
else
  echo "Waited $elapsed seconds. Giving up. Expected $http_ok. Got $response"
fi

#wait a bit for join to happen
sleep $wait_seconds

#join
/opt/vmware/bin/lightwave domain join --domain-controller $lwserver --domain $LIGHTWAVE_DOMAIN --password $LIGHTWAVE_PASS

#verify
/opt/vmware/bin/dir-cli nodes list --login Administrator@$LIGHTWAVE_DOMAIN --password $LIGHTWAVE_PASS --server-name $lwserver

#setup go test environment
echo "Setting up go test environment"
export GOPATH=/src/mutentca/tests/integration-tests/go
cd /src/mutentca/tests/integration-tests/go/src

#download go dependencies
echo "Downloading go test dependencies"
go get github.com/stretchr/testify/suite
go get github.com/tebeka/go2xunit
go get ./...

#run mutentca integration tests
echo "Starting mutenca integration tests"
./run_tests.sh -s $lwserver -n $mutentca -d $LIGHTWAVE_DOMAIN -u Administrator -p $LIGHTWAVE_PASS
