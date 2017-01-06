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

sed -i 's/#Storage=auto/Storage=persistent/' /etc/systemd/journald.conf
tdnf install -y procps-ng
tdnf install -y commons-daemon apache-tomcat boost-1.56.0
tdnf install -y vmware-lightwave-server

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
