#!/bin/bash -e


# Workaround for Photon 2.0 not showing CodeDeploy parameters
# Assumes code deploy runs under /opt/codedeploy-agent/deployment-root/${DEPLOYMENT_GROUP_ID}/${DEPLOYMENT_ID}
if [[ ! $DEPLOYMENT_GROUP_ID ]] ; then
  DEPLOYMENT_GROUP_ID=$(realpath $0 | cut -d "/" -f5)
fi

if [[ ! $DEPLOYMENT_ID ]] ; then
  DEPLOYMENT_ID=$(realpath $0 | cut -d "/" -f6)
fi

LOGDIR="/opt/codedeploy-agent/deployment-root/${DEPLOYMENT_GROUP_ID}/${DEPLOYMENT_ID}/logs"
export LOGDIR
export PATH=$PATH:/root/.local/bin

## the AWS region of the current VM
export EC2_REGION=$(curl -s http://169.254.169.254/latest/meta-data/placement/availability-zone \
                    | sed -e "s:\([0-9][0-9]*\)[a-z]*\$:\\1:")
## the id of current VM
export INSTANCE_ID=$(curl -sS http://169.254.169.254/latest/meta-data/instance-id)
## the name of the auto scaling group this VM belongs to
export ASG_NAME=$(aws autoscaling describe-auto-scaling-instances \
                  --instance-ids ${INSTANCE_ID} \
                  --region ${EC2_REGION} \
                  --query AutoScalingInstances[].AutoScalingGroupName \
                  --output text)

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

# generates SSL cert with SAN of ELB and adds to vecs
generate_ssl_cert() {
    set -x
    VECS_DIR="/var/lib/vmware/vmafd/vecs"
    SSL_STORE="MACHINE_SSL_CERT"
    DEFAULT_CERT="__MACHINE_CERT"

    get_current_region REGION
    get_current_asg ASG
    RES=$(aws autoscaling describe-auto-scaling-groups --auto-scaling-group-names ${ASG} --region ${REGION} --query AutoScalingGroups[].LoadBalancerNames[*] --output text)
    ELB=$(aws elb describe-load-balancers --region ${REGION} --load-balancer-names ${RES} --query LoadBalancerDescriptions[].DNSName --output text)
    echo "ELB is ${ELB}"

    LW_DC=$(/opt/vmware/bin/vmafd-cli get-dc-name --server-name localhost)

    if [ 0 -eq $(openssl x509 -in ${VECS_DIR}/cert.pem -noout -text | grep ${ELB} > /dev/null; echo $?) ]; then
        echo "Cert already generated, skipping step"
        return
    fi

    # Delete old cert
    /opt/vmware/bin/vecs-cli entry delete --store ${SSL_STORE} --alias ${DEFAULT_CERT} -y

    echo "Generating SSL cert for ELB name ${ELB}"
    HOSTNAME=$(hostname -f)
    # Generate the public/private key pair to prepare for certificate signing
    /opt/vmware/bin/certool --genkey --privkey=${VECS_DIR}/key.pem --pubkey=${VECS_DIR}/pub_key.pem
    # Make the certificate config file
    echo "Country = US" > cert.cfg
    echo "Name = CA" >> cert.cfg
    echo "Organization = VMware" >> cert.cfg
    echo "OrgUnit = VMware Engineering" >> cert.cfg
    echo "State = CA" >> cert.cfg
    echo "Locality = Palo Alto" >> cert.cfg
    echo "IPAddress = 127.0.0.1" >> cert.cfg
    echo "Hostname = ${ELB},localhost,${HOSTNAME}" >> cert.cfg

    echo "Generating cert using server ${LW_DC}"
    /opt/vmware/bin/certool --gencert --config=cert.cfg --cert=${VECS_DIR}/cert.pem --privkey=${VECS_DIR}/key.pem --server ${LW_DC}

    echo "Adding cert to vecs store"
    /opt/vmware/bin/vecs-cli entry create --store ${SSL_STORE} --alias ${DEFAULT_CERT} --cert ${VECS_DIR}/cert.pem --key ${VECS_DIR}/key.pem
    rm cert.cfg
    set +x
}

# uses vmafd-cli to set DC name to LW node specified by LW_AFFINITIZED_DC tag value
set_dc_name() {
    set -x
    get_tag_value "LW_AFFINITIZED_DC" LW_DC

    if [[ -n "${LW_DC}" ]]; then
        echo "Setting DC Name to ${LW_DC}"
        /opt/vmware/bin/vmafd-cli set-dc-name --server-name localhost --dc-name ${LW_DC}
    else
        echo "Could not find LW_AFFINITIZED_DC ASG Tag, skipping"
    fi
    set +x
}
