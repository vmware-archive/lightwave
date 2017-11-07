#!/bin/bash -e

LOGDIR="/opt/codedeploy-agent/deployment-root/${DEPLOYMENT_GROUP_ID}/${DEPLOYMENT_ID}/logs"
export LOGDIR
export PATH=$PATH:/root/.local/bin

# retrieves instance ID of this instance
get_current_instance_id() {
    INSTANCE=$(curl -sS http://169.254.169.254/latest/meta-data/instance-id)
    eval "$1=${INSTANCE}"
}

# retrieves IP of this instance
get_current_instance_ip() {
    IP=$(curl -sS http://169.254.169.254/latest/meta-data/local-ipv4)
    eval "$1=${IP}"
}

# retrieves region of this instance
get_current_region() {
    REGION=$(curl -s http://169.254.169.254/latest/meta-data/placement/availability-zone | sed -e "s:\([0-9][0-9]*\)[a-z]*\$:\\1:")
    eval "$1=${REGION}"
}

# retrieves autoscaling group of this instance
get_current_asg() {
    get_current_instance_id INSTANCE
    get_current_region REGION
    ASG=$(aws autoscaling describe-auto-scaling-instances --instance-ids ${INSTANCE} --region ${REGION} --query AutoScalingInstances[].AutoScalingGroupName --output text)
    eval "$1=${ASG}"
}

# retrieves the value of a specific tag on an autoscaling group
get_tag_value() {
    get_current_region REGION
    get_current_asg ASG
    TAG=$1
    VALUE=$(aws autoscaling describe-auto-scaling-groups --auto-scaling-group-names ${ASG} --region ${REGION} --query AutoScalingGroups[].Tags[?Key==\'${TAG}\'].Value --output text)
    eval "$2=${VALUE}"
}

# updates the value of a specific tag on an autoscaling group
set_tag_value() {
    get_current_region REGION
    get_current_asg ASG
    TAG=$1
    VALUE=$2
    aws autoscaling create-or-update-tags --region ${REGION} --tags "ResourceId=${ASG},ResourceType=auto-scaling-group,Key=${TAG},Value=${VALUE},PropagateAtLaunch=true"
}

# read a S3 file
read_s3_file()
{
    S3_FILE=$1
    TMP_FILE=/tmp/s3file-${RANDOM}
    aws s3 cp ${S3_FILE} ${TMP_FILE}
    CONTENT=$(cat ${TMP_FILE})
    rm ${TMP_FILE}
    eval "$2=${CONTENT}"
}

# retrieves post admin password
get_post_password() {
    get_tag_value "PASSWORD_PATH" PASSWORD_PATH
    if [[ -z ${PASSWORD_PATH} ]]
    then
        PASSWORD_PATH=s3://cascade-passwords/post-password  # default path
    fi
    read_s3_file ${PASSWORD_PATH} PASSWORD
    eval "$1=${PASSWORD}"
}

# lists IP of all nodes in the given autoscaling group
get_asg_node_list() {
    get_current_region REGION
    ASG=$1
    IDS=($(aws autoscaling describe-auto-scaling-groups --auto-scaling-group-names ${ASG} --region ${REGION} --query AutoScalingGroups[].Instances[].InstanceId --output text))
    NODES=()
    for ID in ${IDS[@]}
    do
        IP=$(aws ec2 describe-instances --instance-ids ${ID} --region ${REGION} --query Reservations[].Instances[].PrivateIpAddress --output text)
        NODES+=(${IP})
    done
    eval "$2=(`echo ${NODES[@]}`)"
}

# lists all existing post partners
find_post_partners() {
    get_current_instance_ip LOCAL
    get_current_region REGION
    get_current_asg ASG
    get_asg_node_list ${ASG} NODES
    PARTNERS=()
    for NODE in ${NODES[@]}
    do
        if [[ ${NODE} != ${LOCAL} ]]
        then
            PARTNERS+=(${NODE})
        fi
    done
    eval "$1=(`echo ${PARTNERS[@]}`)"
}

# leaves lightwave domain and perform required clean up
leave_lightwave() {
    LW_DOMAIN=$1
    NODE=$2
    # TODO - clean up DNS records, certs, and computer object
    # https://bugzilla.eng.vmware.com/show_bug.cgi?id=1977642
}
