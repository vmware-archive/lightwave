#!/bin/bash -xe

export PATH=$PATH:/root/.local/bin

# retrives the value of a specific tag on an autoscaling group
get_tag_value() {
    REGION=$1
    ASG=$2
    TAG=$3
    VALUE=$(aws autoscaling describe-auto-scaling-groups --auto-scaling-group-names ${ASG} --region ${REGION} --query AutoScalingGroups[].Tags[?Key==\'${TAG}\'].Value --output text)
    eval "$4=${VALUE}"
}

# listing all VMs within the same autoscaling group of this instance
find_partners() {
    REGION=$1
    ASG=$2
    INSTANCE=$3
    IDS=($(aws autoscaling describe-auto-scaling-groups --auto-scaling-group-names ${ASG} --region ${REGION} --query AutoScalingGroups[].Instances[].InstanceId --output text))
    PARTNERS=()
    for ID in ${IDS[@]}
    do
        if [[ ${ID} != ${INSTANCE} ]]
        then
            IP=$(aws ec2 describe-instances --instance-ids ${ID} --region ${REGION} --query Reservations[].Instances[].PrivateIpAddress --output text)
            PARTNERS+=(${IP})
        fi
    done
    eval "$4=(`echo ${PARTNERS[@]}`)"
}

echo "Step 1: Get variables from AWS"

INSTANCE=$(curl -sS http://169.254.169.254/latest/meta-data/instance-id)
REGION=$(curl -s http://169.254.169.254/latest/meta-data/placement/availability-zone | sed -e "s:\([0-9][0-9]*\)[a-z]*\$:\\1:")
ASG=$(aws autoscaling describe-auto-scaling-instances --instance-ids ${INSTANCE} --region ${REGION} --query AutoScalingInstances[].AutoScalingGroupName --output text)

get_tag_value ${REGION} ${ASG} "LW_DOMAIN" LW_DOMAIN
get_tag_value ${REGION} ${ASG} "POST_PASSWORD" POST_PASSWORD
find_partners ${REGION} ${ASG} ${INSTANCE} PARTNERS

echo "Step 2: Start POST"

/opt/likewise/bin/lwsm start post

echo "Step 3: Promote POST if necessary"

if [[ -z $(/opt/vmware/bin/post-cli node list --server-name localhost | grep `hostname`) ]]
then
    if [[ ${#PARTNERS[@]} -eq 0 ]]; then
        echo "Step 3-A: Promote POST as the first instance"
        /opt/vmware/bin/post-cli node promote --domain-name ${LW_DOMAIN} --password ${POST_PASSWORD}
    else
        echo "Step 3-B: Promote POST as a subsequent instance"
        /opt/vmware/bin/post-cli node promote --partner-name ${PARTNERS[0]} --password ${POST_PASSWORD}
    fi
else
    echo "Step 3-C: POST is already promoted - No action"
fi
