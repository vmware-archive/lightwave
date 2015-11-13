#!/bin/bash

if [ $# -lt 3 ]; then
    echo "Usage: build-lightwave-container.sh <path: docker src root> <path: RPM cache> <path: docker image>"
    exit 1
fi

DOCKERFILE_PATH=$1
RPM_CACHE_DIR=$2
DOCKER_IMAGE=$3

DOCKER_IMAGE_TAG="vmware/lightwave-sts"

# start docker daemon
systemctl start docker
if [ $? -ne 0 ]; then
    echo "Failed to start Docker service"
    exit 1
fi

sleep 5

# build vmware/lightwave-sts docker image
docker build --no-cache --tag $DOCKER_IMAGE_TAG $DOCKERFILE_PATH
if [ $? -ne 0 ]; then
    echo "Failed to build docker image"
    exit 1
fi

# save image
docker save $DOCKER_IMAGE_TAG > $DOCKER_IMAGE
if [ $? -ne 0 ]; then
    echo "Failed to export docker image"
    exit 1
fi

echo "Build docker image successfully at [$DOCKER_IMAGE]"

