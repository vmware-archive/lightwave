#!/bin/bash -xe

# create lightwave yum repo
## change the lightwave yum repo location to point to /tmp/lightwave
sed -i -e "s|https://vmware.bintray.com/lightwave-dev/photon/master|file:///tmp/vmware/lightwave|" -e "s|gpgcheck=1|gpgcheck=0|" /etc/yum.repos.d/lightwave.repo
## remove existing repo
rm -rf /tmp/vmware/lightwave
mkdir -p /tmp/vmware/lightwave/x86_64
## creating new repo
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cp ${DIR}/../*.rpm /tmp/vmware/lightwave/x86_64
createrepo "/tmp/vmware/lightwave"
tdnf makecache

# install lightwave-post
tdnf install -y lightwave-post
