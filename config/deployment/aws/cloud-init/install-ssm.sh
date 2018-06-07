#!/bin/bash -xe

echo "Installing SSM agent"

# SSM allows users to execute remote commands against the VM
# need to be able to download the RPM and install RPM
wget https://s3.amazonaws.com/ec2-downloads-windows/SSMAgent/latest/linux_amd64/amazon-ssm-agent.rpm
rpm -i amazon-ssm-agent.rpm
rm -rf amazon-ssm-agent.rpm
systemctl enable amazon-ssm-agent
systemctl start amazon-ssm-agent
