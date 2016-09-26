#!/bin/sh -xe

LIGHTWAVE_PASSWORD=${LIGHTWAVE_PASSWORD:-"Admin!23"}
LIGHTWAVE_PARTNER_0=${LIGHTWAVE_PARTNER_0:-192.168.114.2}
LIGHTWAVE_PARTNER_1=${LIGHTWAVE_PARTNER_1:-192.168.114.3}
LIGHTWAVE_PARTNER_2=${LIGHTWAVE_PARTNER_2:-192.168.114.4}

# Create network
# Try creating overlay network in case there is key/value store setup for docker swarm
docker network create --driver overlay --subnet=192.168.114.0/27 lightwave || true

if [ $(docker network ls | grep lightwave | wc -l) -ne 1 ]; then
  echo "WARNING: Failed to create an overlay network, trying creating bridged network now."

  # Now try creating network with bridge networking if previous command failed because swarm was not setup.
  docker network create  --subnet=192.168.114.0/27 lightwave || true

  if [ $(docker network ls | grep lightwave | wc -l) -ne 1 ]; then
    echo "ERROR: Failed to create a network. Exiting!"
    exit 1
  fi
fi

./run-lw-container.sh $LIGHTWAVE_PARTNER_0 $LIGHTWAVE_PARTNER_0 $LIGHTWAVE_PASSWORD standalone 0
./run-lw-container.sh $LIGHTWAVE_PARTNER_1 $LIGHTWAVE_PARTNER_0 $LIGHTWAVE_PASSWORD partner 1
./run-lw-container.sh $LIGHTWAVE_PARTNER_2 $LIGHTWAVE_PARTNER_0 $LIGHTWAVE_PASSWORD partner 2
