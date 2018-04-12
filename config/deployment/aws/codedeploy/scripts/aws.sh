#!/bin/sh -e


###
# Initialize aws script
init_aws()
{
  if [[ -z ${AWS_PATH_INIT} ]]
  then
    export PATH="${PATH}:/root/.local/bin"
    export AWS_PATH_INIT=true
  fi
}

###
# Load all configuration variables from AWS
load_aws_variables()
{
  # the deployment root directory
  export ROOTDIR=$(dirname $(dirname $(realpath $0)))
  echo "ROOTDIR: ${ROOTDIR}"
  [[ ${ROOTDIR} ]]

  # the AWS region of the current VM
  export REGION=$(curl -s http://169.254.169.254/latest/meta-data/placement/availability-zone \
      | sed -e "s:\([0-9][0-9]*\)[a-z]*\$:\\1:")
  echo "REGION: ${REGION}"
  [[ ${REGION} ]]

  # the id of current VM
  export INSTANCE_ID=$(curl -sS http://169.254.169.254/latest/meta-data/instance-id)
  echo "INSTANCE_ID: ${INSTANCE_ID}"
  [[ ${INSTANCE_ID} ]]

  # the name of the auto scaling group this VM belongs to
  export ASG=$(aws autoscaling describe-auto-scaling-instances \
      --instance-ids ${INSTANCE_ID} \
      --region ${REGION} \
      --query AutoScalingInstances[].AutoScalingGroupName \
      --output text)
  echo "ASG: ${ASG}"
  [[ ${ASG} ]]

  # read all auto scaling group tags
  TAGS=$(aws autoscaling describe-auto-scaling-groups \
      --auto-scaling-group-names ${ASG} \
      --region ${REGION} \
      --query AutoScalingGroups[].Tags[])

  # check mandatory tags
  export COREDUMP_BUCKET_PATH=$(echo $TAGS | jq -r ".[] | select(.Key == \"COREDUMP_BUCKET_PATH\") | .Value")
  echo "COREDUMP_BUCKET_PATH: ${COREDUMP_BUCKET_PATH}"
  [[ ${COREDUMP_BUCKET_PATH} ]]

  export LW_DOMAIN=$(echo $TAGS | jq -r ".[] | select(.Key == \"LW_DOMAIN\") | .Value")
  echo "LW_DOMAIN: ${LW_DOMAIN}"
  [[ ${LW_DOMAIN} ]]

  export SAN_ENTRY=$(echo $TAGS | jq -r ".[] | select(.Key == \"SAN_ENTRY\") | .Value")
  echo "SAN_ENTRY: ${SAN_ENTRY}"
  [[ ${SAN_ENTRY} ]]

  export SECRET_STORE=$(echo $TAGS | jq -r ".[] | select(.Key == \"SECRET_STORE\") | .Value")
  echo "SECRET_STORE: ${SECRET_STORE}"
  [[ ${SECRET_STORE} ]]

  export SITE=$(echo $TAGS | jq -r ".[] | select(.Key == \"SITE\") | .Value")
  echo "SITE: ${SITE}"
  [[ ${SITE} ]]

  # check optional tags
  export DEFAULT_PARTNER=$(echo $TAGS | jq -r ".[] | select(.Key == \"DEFAULT_PARTNER\") | .Value")
  echo "DEFAULT_PARTNER: ${DEFAULT_PARTNER}"

  export LOGSTASH_ELB=$(echo $TAGS | jq -r ".[] | select(.Key == \"LOGSTASH_ELB\") | .Value")
  echo "LOGSTASH_ELB: ${LOGSTASH_ELB}"

  export WAVEFRONT_PROXY_ELB=$(echo $TAGS | jq -r ".[] | select(.Key == \"WAVEFRONT_PROXY_ELB\") | .Value")
  echo "WAVEFRONT_PROXY_ELB: ${WAVEFRONT_PROXY_ELB}"

  # pick a partner to join to
  if [[ -n "${DEFAULT_PARTNER}" ]]
  then
    export PARTNER=${DEFAULT_PARTNER}
  else
    find_asg_partners
    export PARTNER=${PARTNERS[0]}
  fi
  echo "PARTNER: ${PARTNER}"
}

###
# Load all user credentials from AWS
load_aws_credentials()
{
  PASSES=$(aws secretsmanager get-secret-value \
      --region ${REGION} \
      --secret-id ${SECRET_STORE} \
      --query SecretString)

  PASS=$(echo $PASSES | jq -r "fromjson.\"${ADMINISTRATOR_USER}\"")
  if [[ "${PASS}" != null ]]
  then
    export ADMINISTRATOR_PASS=${PASS}
  fi

  PASS=$(echo $PASSES | jq -r "fromjson.\"${DOMAIN_PROMOTER_USER}\"")
  if [[ "${PASS}" != null ]]
  then
    export DOMAIN_PROMOTER_PASS=${PASS}
  fi

  PASS=$(echo $PASSES | jq -r "fromjson.\"${DOMAIN_JOINER_USER}\"")
  if [[ "${PASS}" != null ]]
  then
    export DOMAIN_JOINER_PASS=${PASS}
  fi

  PASS=$(echo $PASSES | jq -r "fromjson.\"${SRV_ACCT_MGR_USER}\"")
  if [[ "${PASS}" != null ]]
  then
    export SRV_ACCT_MGR_PASS=${PASS}
  fi

  PASS=$(echo $PASSES | jq -r "fromjson.\"${SCHEMA_MGR_USER}\"")
  if [[ "${PASS}" != null ]]
  then
    export SCHEMA_MGR_PASS=${PASS}
  fi
}

###
# Finding replication through by listing all VMs within the autoscaling GROUP
# the VM is aprt of
find_asg_partners()
{
  echo "Finding partners in ASG ${ASG}"
  # listing all VMs within the auto scaling group
  IDS=($(aws autoscaling describe-auto-scaling-groups \
      --auto-scaling-group-names ${ASG} \
      --region ${REGION} \
      --query AutoScalingGroups[].Instances[].InstanceId \
      --output text))

  for ID in ${IDS[@]}
  do
    # ignoring localhost
    if [[ "${ID}" != "${INSTANCE_ID}" ]]
    then
      # convert ID to IP
      PARTNERS+=($(aws ec2 describe-instances \
          --instance-ids ${ID} \
          --region ${REGION} \
          --query Reservations[].Instances[].PrivateIpAddress \
          --output text))
    fi
  done
}

init_aws
