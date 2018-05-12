#!/bin/bash -xe

echo "Setting host configuration"

source /root/cloud-init/variables

# Expand main partition to use all space available
# Skip if /dev/xvda2 does not exist
if !( parted --script /dev/xvda2 print )
then
  echo "Primary partition /dev/xvda2 not found."
  exit 1
fi

echo "Expanding primary parition."
parted ---pretend-input-tty /dev/xvda resizepart fix 2 yes 100%
resize2fs /dev/xvda2

echo "Setup SSD filesystems"
/root/cloud-init/setup-ssd-fs.sh /dev/xvdb /var/log
if [ -b /dev/nvme0n1 ]
then
  echo "SSD PCIe card found - setup for /dev/nvme0n1"
  /root/cloud-init/setup-ssd-fs.sh /dev/nvme0n1 /var/lib/vmware
elif [ -b /dev/xvdc ]
then
  echo "SSD PCIe card not found - setup for /dev/xvdc"
  /root/cloud-init/setup-ssd-fs.sh /dev/xvdc /var/lib/vmware
else
  echo "No device for /var/lib/vmware found"
  exit 1
fi

echo "Set up firewall rules"
iptables -P INPUT ACCEPT
iptables -P OUTPUT ACCEPT
iptables -P FORWARD ACCEPT
iptables -F

echo "Setting up Hostname"
HOSTNAME=${ASG}-${INSTANCE_ID}
HOSTNAME_FQDN=${HOSTNAME}.${LW_DOMAIN}

echo "::1        localhost ipv6-localhost ipv6-loopback" > /etc/hosts
echo "127.0.0.1  ${HOSTNAME_FQDN} ${HOSTNAME} localhost localhost.${LW_DOMAIN}" >> /etc/hosts
hostnamectl set-hostname ${HOSTNAME}

sed -i '/^DNS=/s/^/#/' /etc/systemd/resolved.conf
sed -i '/^Domains=/s/^/#/' /etc/systemd/network/99-dhcp-en.network
systemctl restart systemd-networkd systemd-resolved
