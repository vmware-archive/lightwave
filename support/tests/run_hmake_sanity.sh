#!/bin/bash

#source env variables
source $LIGHTWAVE_ENV_FILE

#check environment vars
if [ -z "$LIGHTWAVE_DOMAIN" -o -z "$LIGHTWAVE_PASS" ]; then
  echo "Please set LIGHTWAVE_DOMAIN and LIGHTWAVE_PASS in .env file"
  exit 1
fi

#prepare by installing rpms built in this build
rpm -Uvh --nodeps /src/build/rpmbuild/RPMS/x86_64/lightwave-client*.rpm

primary=server.$LIGHTWAVE_DOMAIN
partner=client.$LIGHTWAVE_DOMAIN

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

#wait a bit for join to happen
sleep $wait_seconds

#obtain a token
response=$(curl -k --write-out %{http_code} --silent --output /dev/null \
         "https://$primary/openidconnect/token/$LIGHTWAVE_DOMAIN" \
         -H 'content-type: application/x-www-form-urlencoded' \
         -d 'grant_type=password' \
         -d "username=administrator@$LIGHTWAVE_DOMAIN" \
         --data-urlencode "password=$LIGHTWAVE_PASS" \
         -d 'scope=openid rs_vmdir')

if [ $response -eq $http_ok ]; then
  echo "Acquire token success. Sanity test complete!"
else
  echo "Failed to acquire token. Expected $http_ok. Got $response"
  exit 1
fi
