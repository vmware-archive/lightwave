#!/bin/bash -xe

DISTRO=`cat /etc/os-release | grep VERSION_ID | cut -d= -f2`
if [ $DISTRO != "1.0" ]; then
  cp $(dirname $(realpath $0))/lightwave.repo /etc/yum.repos.d/lightwave.repo
fi

echo "Step 1: Upgrade/install createrepo and its dependencies"

tdnf makecache
tdnf install -y sed zip unzip createrepo netcat

if [ $DISTRO == "1.0" ]; then
    tdnf install -y c-rest-engine-1.1-10.ph1
else
    tdnf install -y c-rest-engine-1.1-1.ph2
fi

echo "Install patched version of cyrus-sasl"

tdnf install -y cyrus-sasl-2.1.26

echo "Install likewise-open"

tdnf install -y likewise-open-6.2.11-7

echo "Upgrade openssl"

tdnf install -y openssl-1.0.2n

echo "Step 2: Enable DNS caching"

sed -i 's/hosts: files/hosts:files resolve/g' /etc/nsswitch.conf
