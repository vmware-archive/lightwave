#!/bin/bash
dnsIP=`hostname -i`    # IP Address of PSC above!!!
vmDomainName=DC_DOMAIN
ns1=`grep nameserver /etc/resolv.conf | head -n 1 | grep nameserver | sed 's|nameserver *|DNS=|'`
ns2=`grep nameserver /etc/resolv.conf | head -n 2 | tail -n 1 | grep nameserver | sed 's|nameserver *|DNS=|'`
sed -e "s/\(\[Network\]\)/\1\nDNS=${dnsIP}\n$ns1\n$ns2/" -i /etc/systemd/network/*.network
sed -e "s/\(\[Network\]\)/\1\nDomains=${vmDomainName}/" -i /etc/systemd/network/*.network
cat <<NNNN>> /etc/systemd/network/*.network
[DHCP]
  UseHostname=false
  UseDNS=false
NNNN
systemctl restart systemd-networkd
systemctl restart systemd-resolved
