#!/bin/bash

if [ $# -lt 2 ]; then
    echo "Usage: build-lightwave-container.sh <path: docker src root> <path: docker image>"
    exit 1
fi

DOCKERFILE_PATH=$1
DOCKER_IMAGE=$2

DOCKER_IMAGE_TAG="vmware/lightwave-sts"

# start docker daemon
systemctl start docker
if [ $? -ne 0 ]; then
    echo "Failed to start Docker service"
    exit 1
fi

sleep 5

# create lightwave yum repository

createrepo $DOCKERFILE_PATH

# modify Dockerfile to use local lightwave yum repository

tmpfile=$(mktemp /tmp/lw.XXXXXX)
cat >$tmpfile <<EOF
COPY x86_64 /tmp/vmware/lightwave/x86_64
COPY repodata /tmp/vmware/lightwave/repodata
RUN tdnf makecache && \
    tdnf install -y sed && \
    sed -i -e "s/https:\/\/dl.bintray.com/file:\/\/\/tmp/" -e "s/gpgcheck=1/gpgcheck=0/" /etc/yum.repos.d/lightwave.repo
EOF
sed -i -e "/# Build hook/r $tmpfile" -e "//d" $DOCKERFILE_PATH/Dockerfile
rm $tmpfile

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
