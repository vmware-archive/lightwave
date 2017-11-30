#!/bin/bash

source $(dirname $(realpath $0))/common.sh

MAX_RETRY=5
RETRY=1
LEADER=""
HOST=`hostname -f`

get_post_password POST_PASSWORD
get_tag_value "LW_DOMAIN" LW_DOMAIN

while [[ ${RETRY} -le ${MAX_RETRY} ]]
do
    /opt/vmware/bin/post-cli node state --server-name localhost --login administrator --password ${POST_PASSWORD} &> ${LOGDIR}/post_cli_node_state.log
    RET=$?
    LEADER=$(grep 'Leader' ${LOGDIR}/post_cli_node_state.log | awk '{print $1;}')
    echo "Attempt ${RETRY}: ${RET} (${LEADER})"
    if [[ -n ${LEADER} && ${HOST} != ${LEADER} ]]
    then
        break
    fi
    if [ ${RETRY} -gt 1 ]
    then
        sleep 5
    fi
    /opt/vmware/bin/post-cli node startvote --login administrator@${LW_DOMAIN} --password ${POST_PASSWORD}
    RET=$?
    let RETRY++
done

echo "Step 1: Stop POST"

/opt/likewise/bin/lwsm stop post
