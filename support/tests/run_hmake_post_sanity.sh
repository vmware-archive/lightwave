#!/bin/bash
#Assumptions:
#1. env file is available at $LIGHTWAVE_ENV_FILE
#How to run this test
#1. Pre-requisites: docker, hmake docker-compose
#2. Invoke test: hmake test-post
#3. To skip build and run with built rpms: hmake -S pack -S build -S build-lightwave-photon2 test-post

ping_post_server()
{
  local post_srv_node=$1
  local max_attempts=20
  local attempts=1
  local wait_seconds=5

  while [ $attempts -lt $max_attempts ]; do

    /opt/vmware/bin/post-cli node state \
      --server-name $post_srv_node \
      --login administrator@${POST_DOMAIN} \
      --password ${POST_PASS}

    if [ $? -ne 0 ]; then
      echo "waiting for $post_srv_node to start"
    else
      break
    fi

    sleep $wait_seconds
    attempts=$[attempts+1]
  done

  elapsed=$[$attempts*$wait_seconds]

  if [ $attempts -eq $max_attempts ]; then
    echo "Waited $elapsed seconds for $post_srv_node to start. Giving up"
    exit 1
  else
    echo "POST node $post_srv_node ping passed in $elapsed seconds."
  fi
}

#source env variables
source $LIGHTWAVE_ENV_FILE

#check environment vars
if [ -z "$POST_DOMAIN" -o -z "$LIGHTWAVE_PASS" ]; then
  echo "Please set POST_DOMAIN and LIGHTWAVE_PASS in .env file"
  exit 1
fi

POST_PASS=${LIGHTWAVE_PASS}

#prepare by installing rpms built in this build
rpm -Uvh --nodeps /src/build/rpmbuild/RPMS/x86_64/lightwave-client*.rpm
rpm -Uvh --nodeps /src/build/rpmbuild/RPMS/x86_64/lightwave-post*.rpm

primary=server.$POST_DOMAIN
secondary=server-n2.$POST_DOMAIN

ping_post_server $primary
ping_post_server $secondary

