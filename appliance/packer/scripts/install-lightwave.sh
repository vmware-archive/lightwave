#!/bin/bash -xe
# Copyright 2016 VMware, Inc. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License"); you may not
# use this file except in compliance with the License.  You may obtain a copy of
# the License at http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software distributed
# under the License is distributed on an "AS IS" BASIS, without warranties or
# conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the License for the
# specific language governing permissions and limitations under the License.

echo "installing lightwave"

mkdir -p /var/run/sshd; chmod -rx /var/run/sshd
rm -rf /etc/ssh/ssh_host_rsa_key
ssh-keygen -t rsa -f /etc/ssh/ssh_host_rsa_key

# Argument indicates using rpms on vm
if [ "$#" -ge 1 ]; then
    tdnf -y install createrepo
    createrepo "/tmp/vmware/lightwave"
    sed -i -e "s/https:\/\/dl.bintray.com/file:\/\/\/tmp/" -e "s/gpgcheck=1/gpgcheck=0/" /etc/yum.repos.d/lightwave.repo
    tdnf makecache
fi

sed -i 's/#Storage=auto/Storage=persistent/' /etc/systemd/journald.conf

tdnf install -y commons-daemon-1.0.15 \
     openjre-1.8.0.112 \
     apache-tomcat-8.5.8
tdnf install -y likewise-open-6.2.11
tdnf install -y boost-1.60.0
tdnf install -y jaxws-ri
tdnf install -y procps-ng
tdnf install -y vmware-lightwave-server-1.2.0

if [ "$#" -ge 1 ]; then
    sed -i -e "s/file:\/\/\/tmp/https:\/\/dl.bintray.com/" -e "s/gpgcheck=0/gpgcheck=1/" /etc/yum.repos.d/lightwave.repo
    tdnf makecache
fi

# open iptables ports
# EXPOSE 22 53/udp 53 88/udp 88 389 443 636 2012 2014 2020
echo "iptables -I INPUT -p tcp --dport 22 -j ACCEPT" >> /etc/systemd/scripts/iptables
echo "iptables -I INPUT -p udp --dport 53 -j ACCEPT" >> /etc/systemd/scripts/iptables
echo "iptables -I INPUT -p tcp --dport 53 -j ACCEPT" >> /etc/systemd/scripts/iptables
echo "iptables -I INPUT -p udp --dport 88 -j ACCEPT" >> /etc/systemd/scripts/iptables
echo "iptables -I INPUT -p tcp --dport 88 -j ACCEPT" >> /etc/systemd/scripts/iptables
echo "iptables -I INPUT -p tcp --dport 389 -j ACCEPT" >> /etc/systemd/scripts/iptables
echo "iptables -I INPUT -p tcp --dport 443 -j ACCEPT" >> /etc/systemd/scripts/iptables
echo "iptables -I INPUT -p tcp --dport 636 -j ACCEPT" >> /etc/systemd/scripts/iptables
echo "iptables -I INPUT -p tcp --dport 2012 -j ACCEPT" >> /etc/systemd/scripts/iptables
echo "iptables -I INPUT -p tcp --dport 2014 -j ACCEPT" >> /etc/systemd/scripts/iptables
echo "iptables -I INPUT -p tcp --dport 2020 -j ACCEPT" >> /etc/systemd/scripts/iptables
