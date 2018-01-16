#!/bin/bash

###
# This script test vmafd-cli client apis
###

PASSWORD=$1

if [[ "$#" != "1" ]] ; then
    echo "[ERROR] invalid arguments"
    echo -e "Usage:"
    echo -e "\t$0 PASSWORD"
    exit 1
fi


# Test Begin/End Upgrade
echo "Checking Heartbeat status"
RES=$(/opt/vmware/bin/vmafd-cli get-heartbeat-status | sed 's/.*:\t*//g')
if [[ ${RES} != 1 ]]; then
    echo "Error: Heartbeat status should be 1"
    exit 1
fi

echo "Begining Upgrade"
/opt/vmware/bin/vmafd-cli begin-upgrade --server-name localhost --user-name Administrator --password ${PASSWORD}

echo "Checking Heartbeat status"
RES=$(/opt/vmware/bin/vmafd-cli get-heartbeat-status | sed 's/.*:\t*//g')
if [[ ${RES} != 0 ]]; then
    echo "Error: Heartbeat status should be 0"
    exit 1
fi

echo "Ending Upgrade"
/opt/vmware/bin/vmafd-cli end-upgrade --server-name localhost --user-name Administrator --password ${PASSWORD}

echo "Checking Heartbeat status"
RES=$(/opt/vmware/bin/vmafd-cli get-heartbeat-status | sed 's/.*:\t*//g')
if [[ ${RES} != 1 ]]; then
    echo "Error: Heartbeat status should be 1"
    exit 1
fi

echo ""
echo "PASSED"
