#!/bin/bash

echo "Add tdnf upgrade/install commands here to upgrade/install new dependencies"
# required to create local yum repository
tdnf install -y sed
tdnf install -y createrepo

tdnf install -y zip unzip
