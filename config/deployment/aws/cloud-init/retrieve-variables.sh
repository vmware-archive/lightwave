#!/bin/bash -xe

echo "Retrieving variables"

INSTANCE_ID=$(curl -sS http://169.254.169.254/latest/meta-data/instance-id)
echo "INSTANCE_ID=${INSTANCE_ID}" >> /root/cloud-init/variables
[[ ${INSTANCE_ID} ]]

REGION=$(curl -s http://169.254.169.254/latest/meta-data/placement/availability-zone |\
         sed -e "s:\([0-9][0-9]*\)[a-z]*\$:\\1:")
echo "REGION=${REGION}" >> /root/cloud-init/variables
[[ ${REGION} ]]

ASG=$(aws autoscaling describe-auto-scaling-instances \
    --instance-ids ${INSTANCE_ID} \
    --region ${REGION} \
    --query AutoScalingInstances[].AutoScalingGroupName \
    --output text)
echo "ASG=${ASG}" >> /root/cloud-init/variables
[[ ${ASG} ]]

TAGS=$(aws autoscaling describe-auto-scaling-groups \
    --auto-scaling-group-names ${ASG} \
    --region ${REGION} \
    --query AutoScalingGroups[].Tags[])

LW_DOMAIN=$(echo $TAGS | jq -r ".[] | select(.Key == \"LW_DOMAIN\") | .Value")
echo "LW_DOMAIN=${LW_DOMAIN}" >> /root/cloud-init/variables
[[ ${LW_DOMAIN} ]]

CODEDEPLOY_INSTALLER_LOCATION=$(echo $TAGS | jq -r ".[] | select(.Key == \"CODEDEPLOY_INSTALLER_LOCATION\") | .Value")
echo "CODEDEPLOY_INSTALLER_LOCATION=${CODEDEPLOY_INSTALLER_LOCATION}" >> /root/cloud-init/variables
[[ ${CODEDEPLOY_INSTALLER_LOCATION} ]]
