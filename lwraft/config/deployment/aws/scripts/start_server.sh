#!/bin/bash -xe

# configure lightwave
## add the location of the aws cmd line tool to the path
export PATH=$PATH:/root/.local/bin

# retrives the value of a specific tag on an autoscaling group
#
# TAG: the name of the tag whose value to retrieve
# ASG: the name of the autoscaling group
# REGION: the region the auto scaling group resides in
get_tag_value() {
  TAG=$1
  ASG=$2
  REGION=$3
  local VALUE=($(aws autoscaling describe-auto-scaling-groups --auto-scaling-group-names $ASG --region $REGION --query AutoScalingGroups[].Tags[?Key==\'${TAG}\'].Value --output text))
  echo "$VALUE"
}

# finding replication through by listing all VMs within the autoscaling GROUP
# the VM is aprt of.
find_partners() {
  GROUP=$1
  REGION=$2
  DOMAIN=$3
  # listing all VMs within the auto scaling group
  IDS=($(aws autoscaling describe-auto-scaling-groups --auto-scaling-group-names $GROUP --region $REGION --query AutoScalingGroups[].Instances[].InstanceId --output text))
  for ID in ${IDS[@]}; do
      # ignoring our selfs
      if [[ $ID == $INSTANCE_ID ]]; then
          continue
      fi
      # getting the private IP of a VM and adding it to the partner list
      IP=$(aws ec2 describe-instances --instance-ids $ID --region $REGION --query Reservations[].Instances[].PrivateIpAddress --output text)
      PARTNERS+=($(aws ec2 describe-instances --instance-ids $ID --region $REGION --query Reservations[].Instances[].PrivateIpAddress --output text))
      echo "$IP post-unstable-$ID.$DOMAIN post-unstable-$ID" >> /etc/hosts
  done
  echo "Finished finding partner nodes"
}

## the AWS region of the current VM
EC2_REGION=$(curl -s http://169.254.169.254/latest/meta-data/placement/availability-zone | sed -e "s:\([0-9][0-9]*\)[a-z]*\$:\\1:")
## the instance id of the current VM
INSTANCE_ID=$(curl -sS http://169.254.169.254/latest/meta-data/instance-id)
## the name of the auto scaling group this VM belongs to
ASG_NAME=$(aws autoscaling describe-auto-scaling-instances --instance-ids $INSTANCE_ID --region $EC2_REGION  --query AutoScalingInstances[].AutoScalingGroupName --output text)

LW_DOMAIN_NAME=$(get_tag_value LW_DOMAIN $ASG_NAME $EC2_REGION)
POST_ADMIN_PASSWORD=$(get_tag_value POST_PASSWORD $ASG_NAME $EC2_REGION)

PARTNERS=()
find_partners $ASG_NAME $EC2_REGION $LW_DOMAIN_NAME

/opt/likewise/bin/lwsm start post

if [[ ${#PARTNERS[@]} -eq 0 ]]; then
  # start first instance of POST
  /opt/vmware/bin/post-cli node promote --domain-name $LW_DOMAIN_NAME --password $POST_ADMIN_PASSWORD || true
else
  /opt/vmware/bin/post-cli node promote --partner-name ${PARTNERS[0]} --password $POST_ADMIN_PASSWORD || true
fi
