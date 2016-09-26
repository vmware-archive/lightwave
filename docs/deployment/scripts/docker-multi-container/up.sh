#!/bin/sh +xe

# Cleanup old run
docker rmi vmware/photon-controller-lightwave-client || true
./delete-lw-cluster.sh

# Start
./load-images.sh
./make-lw-cluster.sh
./make-users.sh
