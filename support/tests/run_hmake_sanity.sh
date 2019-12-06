#!/bin/bash

wait_for_server()
{
  local lw_srv_node=$1

  local max_attempts=20
  local attempts=1
  local response=1
  local nc_ok=0
  local wait_seconds=5

  while [ $response -ne 0 ] && [ $attempts -lt $max_attempts ]; do
    sleep $wait_seconds
    netcat -v -z $lw_srv_node 636
    response=$?
    echo "waiting for $lw_srv_node, response=$response [ $attempts/$max_attempts ]"
    attempts=$[attempts+1]
  done

  elapsed=$[$attempts*$wait_seconds]

  if [ $response -eq $nc_ok ]; then
    echo "$lw_srv_node up in $elapsed seconds."
  else
    echo "Waited $elapsed seconds. Giving up. Expected $nc_ok. Got $response"
  fi
}

obtain_a_token()
{
  local max_attempts=20
  local attempts=1
  local http_ok='200'
  local response='000'
  local wait_seconds=1

  while [ $response -ne $http_ok ] && [ $attempts -lt $max_attempts ]; do
      sleep $wait_seconds
      response=$(curl -k --write-out %{http_code} --silent --output /dev/null \
         "https://$lw_sts/$LIGHTWAVE_DOMAIN/idp/oidc/token" \
         -H 'content-type: application/x-www-form-urlencoded' \
         -d 'grant_type=password' \
         -d "username=administrator@$LIGHTWAVE_DOMAIN" \
         --data-urlencode "password=$LIGHTWAVE_PASS" \
         -d 'scope=openid rs_vmdir')

    echo "acquire a token from $lw_srv_node, response=$response [ $attempts/$max_attempts ]"
    attempts=$[attempts+1]
  done

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
lw_sts=client.$LIGHTWAVE_DOMAIN

#check environment vars
if [ -z "$LIGHTWAVE_DOMAIN" -o -z "$LIGHTWAVE_PASS" ]; then
  echo "Please set LIGHTWAVE_DOMAIN and LIGHTWAVE_PASS in .env file"
  exit 1
fi

#prepare by installing rpms built in this build
rpm -Uvh --nodeps /src/build/rpmbuild/RPMS/x86_64/lightwave-client*.rpm

wait_for_server $lw_server_1
wait_for_server $lw_server_2

obtain_a_token
