#!/bin/bash -xe

DISTRO=`cat /etc/os-release | grep VERSION_ID | cut -d= -f2`
if [ $DISTRO != "1.0" ]; then
  cp $(dirname $(realpath $0))/lightwave.repo /etc/yum.repos.d/lightwave.repo
fi

echo "Step 1: Upgrade/install createrepo and its dependencies"

tdnf makecache
tdnf install -y sed zip unzip netcat

echo "Install patched version of cyrus-sasl"
echo "Install likewise-open"
echo "Upgrade openssl"
if [ $DISTRO == "1.0" ]; then
    # This cyrus-sasl version has a  patch for SRP plugin memory leak
    tdnf install -y createrepo \
    c-rest-engine-1.2-3.ph1 \
    cyrus-sasl-2.1.26-10.ph1 \
    likewise-open-6.2.11-7 \
    openssl-1.0.2n
else
    tdnf install -y createrepo_c \
    c-rest-engine-1.1-1.ph2 \
    cyrus-sasl-2.1.26 \
    likewise-open-6.2.11 \
    openssl-1.0.2n
fi

echo "Step 2: Enable DNS caching"

sed -i 's/hosts: files/hosts:files resolve/g' /etc/nsswitch.conf
