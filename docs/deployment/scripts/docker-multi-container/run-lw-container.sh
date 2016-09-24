#!/bin/bash

LIGHTWAVE_IP=$1
LIGHTWAVE_MASTER_IP=$2
LIGHTWAVE_PASSWORD=$3
LIGHTWAVE_DEPLOYMENT=$4
NUMBER=$5

CONTAINER_NAME=lightwave-$NUMBER
LIGHTWAVE_DOMAIN=photon.local
LIGHTWAVE_SITE=Default-first-site

LW_TMP_DIR=$(mktemp -d "$PWD/lw_tmp.XXXXX")
trap "rm -rf $LW_TMP_DIR" EXIT

LIGHTWAVE_CONFIG_DIR=${LW_TMP_DIR}/config
LIGHTWAVE_CONFIG_PATH=${LIGHTWAVE_CONFIG_DIR}/lightwave-server.cfg

mkdir -p $LIGHTWAVE_CONFIG_DIR

cat << EOF > $LIGHTWAVE_CONFIG_PATH
deployment=$LIGHTWAVE_DEPLOYMENT
domain=$LIGHTWAVE_DOMAIN
admin=Administrator
password=$LIGHTWAVE_PASSWORD
site-name=$LIGHTWAVE_SITE
hostname=$LIGHTWAVE_IP
first-instance=false
replication-partner-hostname=$LIGHTWAVE_MASTER_IP
disable-dns=1
EOF

echo "Starting Lightwave container #$NUMBER..."
docker run -d \
           --name ${CONTAINER_NAME} \
           --privileged \
           --net=lightwave \
           --ip=$LIGHTWAVE_IP \
           -v /sys/fs/cgroup:/sys/fs/cgroup:ro \
           -v $LIGHTWAVE_CONFIG_DIR:/var/lib/vmware/config \
           vmware/lightwave-sts

# Check if lightwave server is up
attempts=1
reachable="false"
total_attempts=50
while [ $attempts -lt $total_attempts ] && [ $reachable != "true" ]; do
  http_code=$(docker exec -t $CONTAINER_NAME curl -I -so /dev/null -w "%{response_code}" -s -X GET --insecure https://127.0.0.1) || true
  # The curl returns 000 when it fails to connect to the lightwave server
  if [ "$http_code" == "000" ]; then
    echo "Waiting for Lightwave server to startup at $CONTAINER_NAME (attempt $attempts/$total_attempts), will try again."
    attempts=$[$attempts+1]
    sleep 5
  else
    reachable="true"
    break
  fi
done
if [ $attempts -eq $total_attempts ]; then
  echo "Could not connect to Lightwave REST client at $node after $total_attempts attempts"
  exit 1
fi
