#!/bin/bash -e

export PATH=$PATH:/root/.local/bin

logger -t post-demote-deads "Starts"


logger -t post-demote-deads "Step 1: Read ASG tags"

INSTANCE=$(curl -sS http://169.254.169.254/latest/meta-data/instance-id)
REGION=$(curl -s http://169.254.169.254/latest/meta-data/placement/availability-zone | sed -e "s:\([0-9][0-9]*\)[a-z]*\$:\\1:")
ASG=$(aws autoscaling describe-auto-scaling-instances --instance-ids ${INSTANCE} --region ${REGION} --query AutoScalingInstances[].AutoScalingGroupName --output text)

POST_PASSWORD=$(aws autoscaling describe-auto-scaling-groups --auto-scaling-group-names ${ASG} --region ${REGION} --query AutoScalingGroups[].Tags[?Key==\'POST_PASSWORD\'].Value --output text)
LW_DOMAIN=$(aws autoscaling describe-auto-scaling-groups --auto-scaling-group-names ${ASG} --region ${REGION} --query AutoScalingGroups[].Tags[?Key==\'LW_DOMAIN\'].Value --output text)


logger -t post-demote-deads "Step 2: Check if localhost is the leader (if yes, continue)"

LOCALHOST=`hostname -f | awk '{print tolower($0)}'`
LEADER=$(/opt/vmware/bin/post-cli node state --server-name localhost --login administrator --password ${POST_PASSWORD} | grep Leader | awk '{print $1}')
logger -t post-demote-deads "Leader: ${LEADER}"
if [[ ${LOCALHOST} != ${LEADER} ]]
then
    logger -t post-demote-deads "This is not the leader node, nothing to do"
    logger -t post-demote-deads "Ends"
    exit 0
else
    logger -t post-demote-deads "This is the leader node, continue"
fi


logger -t post-demote-deads "Step 3: Get ASG node list"

ASG_NODES=()
IDS=($(aws autoscaling describe-auto-scaling-groups --auto-scaling-group-names ${ASG} --region ${REGION} --query AutoScalingGroups[].Instances[].InstanceId --output text))
for ID in ${IDS[@]}
do
    HOSTNAME=`awk '{print tolower($0)}' <<< ${ASG}-${ID}.${LW_DOMAIN}`
    logger -t post-demote-deads "${HOSTNAME}"
    ASG_NODES+=(${HOSTNAME})
done


logger -t post-demote-deads "Step 4: Get POST node list"

ROLE=0
ROLE_STR=(unknown leader follower offline)
OFFLINE_NODES=()
while read LINE
do
if [[ -n ${LINE} ]]
    then
        if [[ ${LINE} == "Raft leader:" ]]
        then
            ROLE=1
        elif [[ ${LINE} == "Raft follower:" ]]
        then
            ROLE=2
        elif [[ ${LINE} == "Raft offline or candidate member:" ]]
        then
            ROLE=3
        else
            HOSTNAME=${LINE}
            logger -t post-demote-deads "${HOSTNAME} (${ROLE_STR[ROLE]})"
            if [[ ${ROLE} -eq 3 ]]
            then
                OFFLINE_NODES+=(${HOSTNAME})
            fi
        fi
    fi
done < <(/opt/vmware/bin/post-cli node list --server-name localhost | head -n -3)


logger -t post-demote-deads "Step 5: Demote dead nodes"

for ONODE in ${OFFLINE_NODES[@]}
do
    DEAD=1
    for ANODE in ${ASG_NODES[@]}
    do
        if [[ ${ONODE} == ${ANODE} ]]
        then
            DEAD=0
            break
        fi
    done
    if [[ ${DEAD} -eq 1 ]]
    then
        logger -t post-demote-deads "/opt/vmware/bin/post-cli node demote --server-name localhost --login administrator --password <censored> --demote-host-name ${ONODE}"
        /opt/vmware/bin/post-cli node demote --server-name localhost --login administrator --password ${POST_PASSWORD} --demote-host-name ${ONODE} 2>&1 | logger -t post-demote-deads

        # TODO - clean up DNS records, certs, and computer object
        # https://bugzilla.eng.vmware.com/show_bug.cgi?id=1977642
    fi
done


logger -t post-demote-deads "Ends"
