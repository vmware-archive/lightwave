#!/bin/bash -xe

main() {
  # create lightwave yum repo
  ## change the lightwave yum repo location to point to /tmp/lightwave
  sed -i -e "s/https:\/\/dl.bintray.com/file:\/\/\/tmp/" -e "s/gpgcheck=1/gpgcheck=0/" /etc/yum.repos.d/lightwave.repo
  ## remove existing repo
  rm -rf /tmp/vmware/lightwave
  mkdir -p /tmp/vmware/lightwave/x86_64
  ## creating new repo
  DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
  cp ${DIR}/../*.rpm /tmp/vmware/lightwave/x86_64
  ## add the location of the aws cmd line tool to the path
  export PATH=$PATH:/root/.local/bin

  install_or_upgrade_lightwave
  configure_lightwave

  sleep 20
  # TODO: run script to create HA topology
}

####
# updates a.k.a. deletes and creates a new option set that adds the current ip as the DNS server in case
# of override or appends the current ip to the existing list of DNS servers, replaces the first entry
# to stay withing the limit of 3 DNS servers that PhotonOS allows
update_option_set() {
  # Takes 3 arguments
  # 1. ASG_NAME
  # 2. EC2_REGION
  # 3. override/append
  #
  ASG_NAME=$1
  EC2_REGION=$2
  INSTANCE_ID=$(curl -sS http://169.254.169.254/latest/meta-data/instance-id)
  NEW_DNS_IP=$(aws ec2 describe-instances --instance-ids $INSTANCE_ID --region $EC2_REGION --query Reservations[].Instances[].PrivateIpAddress --output text)
  ID=$(aws autoscaling describe-auto-scaling-groups --region $EC2_REGION --auto-scaling-group-names $ASG_NAME --query "AutoScalingGroups[].Instances[0].InstanceId" --output text)
  VPC_ID=$(aws ec2 describe-instances --region $EC2_REGION --instance-ids $ID --query "Reservations[].Instances[].VpcId" --output text)
  OPTION_SET_ID=$(aws --region $EC2_REGION ec2 describe-vpcs --vpc-ids $VPC_ID --query "Vpcs[].DhcpOptionsId" --output text)
  OPTION_SET_NAME=$(aws --region $EC2_REGION ec2 describe-dhcp-options --filters "Name=dhcp-options-id,Values=$OPTION_SET_ID" --query "DhcpOptions[].Tags[?Key=='Name'].Value" --output text)
  #
  # TODO: Instead of getting the last 2 check if any of the DNS servers are down and replace those. This is because AWS DHCP Options set has a limit of 4 for the number of DNS IPs.
  #
  DNS_IPS=$(aws --region $EC2_REGION ec2 describe-dhcp-options --filters "Name=dhcp-options-id,Values=$OPTION_SET_ID" --query "DhcpOptions[].DhcpConfigurations[].Values[].Value" --output json \
  | grep -v "\[" \
  | grep -v "\]" \
  | grep -v $NEW_DNS_IP \
  | tr -d \"\ \
  | tr -d \, \
  | tail -n2 \
  | tr '\n' ',')

  if [[ $3 == "override" ]]; then
    echo "Override the dhcp options set"
    DNS_IPS="${NEW_DNS_IP}"
  else
    echo "Append to the dhcp options set"
    DNS_IPS="${DNS_IPS}${NEW_DNS_IP}"
  fi
  NEW_OPTION_SET_ID=$(aws --region $EC2_REGION ec2 create-dhcp-options --dhcp-configurations "Key=domain-name-servers,Values=$DNS_IPS" --query "DhcpOptions.DhcpOptionsId" --output text)
  aws --region $EC2_REGION ec2 associate-dhcp-options --dhcp-options-id $NEW_OPTION_SET_ID --vpc-id $VPC_ID
  aws --region $EC2_REGION ec2 create-tags --resources $NEW_OPTION_SET_ID --tags "Key=Name,Value=$OPTION_SET_NAME"
  aws --region $EC2_REGION ec2 delete-dhcp-options --dhcp-options-id $OPTION_SET_ID
  echo "Finished updating the option set"
}

####
# finding replication through by listing all VMs within the autoscaling GROUP
# the VM is aprt of.
find_partners() {
  GROUP=$1
  REGION=$2
  # listing all VMs within the auto scaling group
  IDS=($(aws autoscaling describe-auto-scaling-groups --auto-scaling-group-names $GROUP --region $REGION --query AutoScalingGroups[].Instances[].InstanceId --output text))
  for ID in ${IDS[@]}; do
      # ignoring our selfs
      if [[ "${ID}" == "${INSTANCE_ID}" ]]; then
          continue
      fi
      # getting the private IP of a VM and adding it to the partner list
      PARTNERS+=($(aws ec2 describe-instances --instance-ids $ID --region $REGION --query Reservations[].Instances[].PrivateIpAddress --output text))
  done
  echo "Finished finding partner nodes"
}

####
# check whether to upgrade or install lightwave, then tdnf install
# refresh lwsm after install/upgrade
install_or_upgrade_lightwave() {
  set +e
  RES=$(rpm -qa \
        | grep "lightwave")
  if [[ "${RES}" != ""  ]] ; then
    LW_INSTALLED=1
    /opt/likewise/bin/lwsm autostart
  fi
  set -e

  createrepo "/tmp/vmware/lightwave"
  tdnf makecache

  if [[ -v LW_INSTALLED ]]; then
    tdnf upgrade -y lightwave lightwave-client lightwave-server
    exit
  fi

  tdnf install -y lightwave lightwave-client lightwave-server
  /opt/likewise/bin/lwsm autostart
}

####
# configure lw instance to join as a partner or domain controller based on aws instances up on load balancer
configure_lightwave() {
  ## the AWS region of the current VM
  EC2_REGION=$(curl -s http://169.254.169.254/latest/meta-data/placement/availability-zone \
  | sed -e "s:\([0-9][0-9]*\)[a-z]*\$:\\1:")
  ## the instance id of the current VM
  INSTANCE_ID=$(curl -sS http://169.254.169.254/latest/meta-data/instance-id)
  ## the name of the auto scaling group this VM belongs to
  ASG_NAME=$(aws autoscaling describe-auto-scaling-instances --instance-ids $INSTANCE_ID --region $EC2_REGION  --query AutoScalingInstances[].AutoScalingGroupName --output text)

  ## we are using different deployment groups to distinguish between production and Test
  ## all test deployment groups will have the word Test in them
  ## vms deployed in the test environment should use .test domain and vms deployed in the
  ## production domain should use the .prod domain.
  echo ""
  if [[ "$DEPLOYMENT_GROUP_NAME" == *Test* ]]; then
    export LW_DOMAIN_NAME="cloud.test"
  else
    export LW_DOMAIN_NAME="cloud.prod"
  fi

  # TODO: fix password management
  # we probably want to use the AWS en/decription services
  export LW_ADMIN_PASSWORD="L1ghtWave!"

  PARTNERS=()
  find_partners $ASG_NAME $EC2_REGION

  if [[ ${#PARTNERS[@]} -eq 0 ]]; then
    # deploying standalone server
    /opt/vmware/bin/configure-lightwave-server --domain $LW_DOMAIN_NAME --password $LW_ADMIN_PASSWORD
    update_option_set $ASG_NAME $EC2_REGION "override"
  else
    /opt/vmware/bin/configure-lightwave-server --domain $LW_DOMAIN_NAME --password $LW_ADMIN_PASSWORD --server ${PARTNERS[0]}
    update_option_set $ASG_NAME $EC2_REGION "append"
  fi
  ## always adding an external DNS server to make sure tdnf install works for VMs using lightwave as their
  ## DNS server
  /opt/vmware/bin/vmdns-cli add-forwarder 8.8.8.8 --server localhost --username administrator --domain $LW_DOMAIN_NAME --password $LW_ADMIN_PASSWORD | true
  # picking up the new DNS server
  systemctl restart systemd-networkd systemd-resolved
}

main
