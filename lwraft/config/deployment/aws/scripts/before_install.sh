#!/bin/bash -xe

echo "Step 1: Upgrade/install createrepo and its dependencies"
tdnf install -y sed zip unzip createrepo
