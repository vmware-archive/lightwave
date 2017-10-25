#!/bin/bash -e

# Note: this step is exclusively for blue/green deployment

source $(dirname $(realpath $0))/common.sh

echo "Step 1: Check if localhost is the leader (if yes, continue)"

get_tag_value "POST_PASSWORD" POST_PASSWORD

LOCALHOST=`hostname -f | awk '{print tolower($0)}'`
LEADER=$(/opt/vmware/bin/post-cli node state --server-name localhost --login administrator --password ${POST_PASSWORD} | grep Leader | awk '{print $1}')
echo "Leader: ${LEADER}"
if [[ ${LOCALHOST} != ${LEADER} ]]
then
    echo "This is not the leader node, nothing to do"
    exit 0
else
    echo "This is the leader node, continue"
fi


echo "Step 2: Read CURRENT_ASG tag and check if it's an old ASG (if yes, continue)"

get_tag_value "CURRENT_ASG" CURRENT_ASG_TAGGED
echo "CURRENT_ASG_TAGGED=${CURRENT_ASG_TAGGED}"

get_current_asg CURRENT_ASG_ACTUAL
echo "CURRENT_ASG_ACTUAL=${CURRENT_ASG_ACTUAL}"

if [[ ${CURRENT_ASG_TAGGED} = ${CURRENT_ASG_ACTUAL} ]]
then
    echo "CURRENT_ASG tag is up to date, nothing to do"
    exit 0
else
    echo "CURRENT_ASG tag is NOT up to date, continue"
fi


echo "Step 3: Clean up the old ASG"

if [[ -z ${CURRENT_ASG_TAGGED} ]]
then
    echo "CURRENT_ASG tag is not set, no cleanup to do"
else
    get_tag_value "LW_DOMAIN" LW_DOMAIN
    get_asg_node_list ${CURRENT_ASG_TAGGED} NODES
    for NODE in ${NODES[@]}
    do
        echo "Cleaning after node ${NODE} from ${LW_DOMAIN}"
        leave_lightwave ${LW_DOMAIN} ${NODE}
    done
fi

echo "Step 4: Update CURRENT_ASG tag"

set_tag_value "CURRENT_ASG" ${CURRENT_ASG_ACTUAL}
echo "CURRENT_ASG tag is updated successfully (${CURRENT_ASG_ACTUAL})"
