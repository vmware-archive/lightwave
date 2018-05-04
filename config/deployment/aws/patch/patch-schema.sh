#!/bin/bash -e

main()
{
  initialize
  get_node_info
  load_schema_mgr_credential
  install_lightwave_rpm
  perform_schema_patch

  echo ""
  echo "Schema patch completed successfully"
}

initialize()
{
  echo ""
  echo "Installing tools and initializing env vars"

  # install tools
  tdnf makecache -q
  tdnf install -yq python3-pip jq createrepo_c
  pip3 install --upgrade pip setuptools
  pip3 install --upgrade --user awscli

  # update PATH to use aws cli
  export PATH="${PATH}:/root/.local/bin"

  # AWS_REGION must be set by codebuild
  echo "AWS_REGION: ${AWS_REGION}"
  [[ ${AWS_REGION} ]]

  # ASG must be set as build project advanced option
  echo "ASG: ${ASG}"
  [[ ${ASG} ]]

  # read auto scaling group tags
  TAGS=$(aws autoscaling describe-auto-scaling-groups \
      --auto-scaling-group-names ${ASG} \
      --region ${AWS_REGION} \
      --query AutoScalingGroups[].Tags[])

  export LW_DOMAIN=$(echo $TAGS | jq -r ".[] | select(.Key == \"LW_DOMAIN\") | .Value")
  echo "LW_DOMAIN: ${LW_DOMAIN}"
  [[ ${LW_DOMAIN} ]]

  export SECRET_STORE=$(echo $TAGS | jq -r ".[] | select(.Key == \"SECRET_STORE\") | .Value")
  echo "SECRET_STORE: ${SECRET_STORE}"
  [[ ${SECRET_STORE} ]]
}

get_node_info()
{
  echo ""
  echo "Finding a node to run the schema patch against"

  # just pick the first node in the instance list
  export NODE_ID=$(aws autoscaling describe-auto-scaling-groups \
      --auto-scaling-group-names ${ASG} \
      --region ${AWS_REGION} \
      --query AutoScalingGroups[].Instances[0].InstanceId \
      --output text)
  echo "NODE_ID: ${NODE_ID}"
  [[ ${NODE_ID} ]]

  export NODE_IP=$(aws ec2 describe-instances \
      --instance-ids ${NODE_ID} \
      --region ${AWS_REGION} \
      --query Reservations[].Instances[].PrivateIpAddress \
      --output text)
  echo "NODE_IP: ${NODE_IP}"
  [[ ${NODE_IP} ]]
}

load_schema_mgr_credential()
{
  echo ""
  echo "Retrieve schema manager account credential"

  PASSES=$(aws secretsmanager get-secret-value \
      --region ${AWS_REGION} \
      --secret-id ${SECRET_STORE} \
      --query SecretString)

  export SRV_ACCT_MGR_USER=schema-mgr
  echo "SRV_ACCT_MGR_USER: ${SRV_ACCT_MGR_USER}"
  [[ ${SRV_ACCT_MGR_USER} ]]

  export SRV_ACCT_MGR_PASS=$(echo $PASSES | jq -r "fromjson.\"${SRV_ACCT_MGR_USER}\"")
  [[ ${SRV_ACCT_MGR_PASS} ]]
}

install_lightwave_rpm()
{
  echo ""
  echo "Installing lightwave rpm"

  # re-point lightwave repo to /tmp/vmware/lightwave
  sed -i -e "s/https:\/\/.*/file:\/\/\/tmp\/vmware\/lightwave/" -e "s/gpgcheck=1/gpgcheck=0/" /etc/yum.repos.d/lightwave.repo

  # move rpm files to /tmp/vmware/lightwave
  mkdir -p /tmp/vmware/lightwave/x86_64
  mv *.rpm /tmp/vmware/lightwave/x86_64/

  createrepo "/tmp/vmware/lightwave"
  tdnf makecache -q
  tdnf install -yq lightwave-server
}

perform_schema_patch()
{
  echo ""
  echo "Running schema patch command"

  /opt/vmware/bin/vdcschema patch-schema-defs \
      --file /opt/vmware/share/config/vmdirschema.ldif \
      --domain ${LW_DOMAIN} \
      --host ${NODE_IP} \
      --login ${SRV_ACCT_MGR_USER} \
      --passwd ${SRV_ACCT_MGR_PASS} \
      --dryrun > output.log

  /opt/vmware/bin/vdcschema patch-schema-defs \
      --file /opt/vmware/share/config/vmdirschema.ldif \
      --domain ${LW_DOMAIN} \
      --host ${NODE_IP} \
      --login ${SRV_ACCT_MGR_USER} \
      --passwd ${SRV_ACCT_MGR_PASS}
}

main "$@"
