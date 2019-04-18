#!/bin/bash

wait_for_server()
{
  local lw_srv_node=$1

  local max_attempts=20
  local attempts=1
  local response='000'
  local http_ok='200'
  local wait_seconds=5

  while [ $response -ne $http_ok ] && [ $attempts -lt $max_attempts ]; do
    response=$(curl -k --write-out %{http_code} --silent --output /dev/null https://$lw_srv_node)
    echo "waiting for $lw_srv_node, response=$response [ $attempts/$max_attempts ]"
    sleep $wait_seconds
    attempts=$[attempts+1]
  done

  elapsed=$[$attempts*$wait_seconds]

  if [ $response -eq $http_ok ]; then
    echo "$lw_srv_node up in $elapsed seconds."
  else
    echo "Waited $elapsed seconds. Giving up. Expected $http_ok. Got $response"
  fi
}

obtain_a_token()
{
  local lw_srv_node=$1

  local http_ok='200'
  local response='000'

  response=$(curl -k --write-out %{http_code} --silent --output /dev/null \
         "https://$lw_srv_node/openidconnect/token/$LIGHTWAVE_DOMAIN" \
         -H 'content-type: application/x-www-form-urlencoded' \
         -d 'grant_type=password' \
         -d "username=administrator@$LIGHTWAVE_DOMAIN" \
         --data-urlencode "password=$LIGHTWAVE_PASS" \
         -d 'scope=openid rs_vmdir')

  if [ $response -eq $http_ok ]; then
    echo "Acquire token from $lw_srv_node success. Sanity test complete!"
  else
    echo "Failed to acquire token from $lw_srv_node. Expected $http_ok. Got $response"
    exit 1
  fi
}

#source env variables
source $LIGHTWAVE_ENV_FILE

lw_server_1=server.$LIGHTWAVE_DOMAIN
lw_server_2=server-n2.$LIGHTWAVE_DOMAIN
lw_client=client.$LIGHTWAVE_DOMAIN

#check environment vars
if [ -z "$LIGHTWAVE_DOMAIN" -o -z "$LIGHTWAVE_PASS" ]; then
  echo "Please set LIGHTWAVE_DOMAIN and LIGHTWAVE_PASS in .env file"
  exit 1
fi

#prepare by installing rpms built in this build
rpm -Uvh --nodeps /src/build/rpmbuild/RPMS/x86_64/lightwave-client*.rpm

wait_for_server $lw_server_1
wait_for_server $lw_server_2

obtain_a_token $lw_server_1
obtain_a_token $lw_server_2
