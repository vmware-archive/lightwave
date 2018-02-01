#!/bin/bash -xe

echo "Step 1: Upgrade/install createrepo and its dependencies"

tdnf makecache
tdnf install -y sed zip unzip createrepo c-rest-engine-1.1-7.ph1 netcat

echo "Install patched version of cyrus-sasl"

tdnf install -y cyrus-sasl-2.1.26-10.ph1

echo "Install likewise-open"

tdnf install -y likewise-open-6.2.11-7

echo "Step 2: Enable DNS caching"

sed -i 's/hosts: files/hosts:files resolve/g' /etc/nsswitch.conf
