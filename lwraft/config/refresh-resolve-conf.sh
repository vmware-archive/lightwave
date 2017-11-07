#!/bin/bash -e

export PATH=$PATH:/root/.local/bin

logger -t refresh-resolve-conf "Starts"


logger -t refresh-resolve-conf "Step 1: Get DNS list from DHCP option set"

INSTANCE=$(curl -sS http://169.254.169.254/latest/meta-data/instance-id)
REGION=$(curl -s http://169.254.169.254/latest/meta-data/placement/availability-zone | sed -e "s:\([0-9][0-9]*\)[a-z]*\$:\\1:")
VPC=$(aws ec2 describe-instances --region ${REGION} --instance-ids ${INSTANCE} --query 'Reservations[].Instances[].NetworkInterfaces[].VpcId' --output text)

DHCP=$(aws ec2 describe-vpcs --region ${REGION} --vpc-ids ${VPC} --query 'Vpcs[].DhcpOptionsId' --output text)
logger -t refresh-resolve-conf "DHCP option set: ${DHCP}"

DHCP_DNS=($(aws ec2 describe-dhcp-options --region ${REGION} --dhcp-options-ids ${DHCP} --query 'DhcpOptions[].DhcpConfigurations[].Values[].Value' --output text))
logger -t refresh-resolve-conf "DHCP DNS list: ${DHCP_DNS[@]}"


logger -t refresh-resolve-conf "Step 2: Compare the list against /etc/resolv.conf"

RESOLV_DNS=($(grep nameserver /etc/resolv.conf | awk '{print $2;}'))
logger -t refresh-resolve-conf "/etc/resolv.conf: ${RESOLV_DNS[@]}"

if [[ ${DHCP_DNS[@]} == ${RESOLV_DNS[@]} ]]
then
    logger -t refresh-resolve-conf "/etc/resolv.conf is in sync with DHCP option set, nothing to do"
else
    logger -t refresh-resolve-conf "systemctl restart systemd-networkd systemd-resolved"
    systemctl restart systemd-networkd systemd-resolved
fi


logger -t refresh-resolve-conf "Ends"
