#!/bin/bash -xe

echo "Step 1: Upgrade/install createrepo and its dependencies"

tdnf makecache
tdnf install -y sed zip unzip createrepo c-rest-engine-1.0.4-2.ph1

echo "Install patched version of cyrus-sasl"

tdnf install -y cyrus-sasl-2.1.26-9.ph1
