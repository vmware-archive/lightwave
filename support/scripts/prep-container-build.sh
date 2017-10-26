#!/bin/bash

PROJECT_ROOT=$(pwd)

DOCKER_ROOT=$PROJECT_ROOT/build/docker
DOCKER_SRC_ROOT=$PROJECT_ROOT/support/docker/sts

mkdir -p $DOCKER_ROOT

rm -rf $DOCKER_ROOT/*

cp -r $PROJECT_ROOT/build/rpmbuild/RPMS/x86_64 $DOCKER_ROOT

cp $DOCKER_SRC_ROOT/lightwave-init $DOCKER_ROOT
cp $DOCKER_SRC_ROOT/Dockerfile $DOCKER_ROOT

# modify Dockerfile to use local lightwave yum repository

#
# Assumes that sed and createrepo are already installed
#
tmpfile=$(mktemp /tmp/lw.XXXXXX)
cat >$tmpfile <<EOF
COPY x86_64 /tmp/vmware/lightwave/x86_64
RUN sed -i -e "s/https:\/\/dl.bintray.com/file:\/\/\/tmp/" -e "s/gpgcheck=1/gpgcheck=0/" /etc/yum.repos.d/lightwave.repo && \
    createrepo /tmp/vmware/lightwave && \
    tdnf makecache
EOF
sed -i -e "/# Build hook/r $tmpfile" -e "//d" $DOCKER_ROOT/Dockerfile
rm $tmpfile

