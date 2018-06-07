#!/bin/bash -xe

echo "Installing tooling"

tdnf makecache -q
tdnf install -yq bindutils iputils less gawk wget groff parted python2 ruby chkconfig cronie jq
tdnf update -yq curl

# Docker causes DNS resolution issues with Lightwave
echo "Remove Docker from instance"
systemctl stop docker
systemctl disable docker
ip link del docker0
rpm -e docker
