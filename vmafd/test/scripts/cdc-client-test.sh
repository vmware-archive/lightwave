#!/bin/bash

###
# This script verifies cdc-cli tool is able to properly list Domain Controllers
###

DOMAIN=$1
PASSWORD=$2

if [[ "$#" != "2" ]] ; then
    echo "[ERROR] invalid arguments"
    echo -e "Usage:"
    echo -e "\t$0 DOMAIN_NAME PASSWORD"
    exit 1
fi

echo "Joining domain ${DOMAIN}"
/opt/vmware/bin/ic-join --password ${PASSWORD} --domain ${DOMAIN}
if [ ! $? -eq 0 ]; then
    echo "Failed to join ${DOMAIN}"
    exit 1
fi

echo ""
echo "Setting HA mode to default"
/opt/vmware/bin/cdc-cli client-affinity default

echo "Waiting to find DCs..."
sleep 30

echo ""
echo "Listing Domain Controllers:"
RES=$(/opt/vmware/bin/cdc-cli cache list)
if [[ ${RES} == "No DCs found" ]]; then
    echo "Failed to get DC list with cdc-cli"
    exit 1
fi

echo "${RES}"

echo ""
echo "Getting affinitized DC"
DC=$(/opt/vmware/bin/vmafd-cli get-dc-name --server-name localhost)
if [ ! $? -eq 0 ]; then
    echo "Failed to get affinitzed DC"
    exit 1
fi

echo ""
echo "Verifying affinitized DC is in cdc DC list"
grep ${DC} <<< ${RES} > /dev/null
if [ ! $? -eq 0 ]; then
    echo "Failed find affinitized DC in list of DCs"
    exit 1
fi

echo ""
echo "Leaving Domain"
domainjoin leave
if [ ! $? -eq 0 ]; then
    echo "Failed to leave domain"
    exit 1
fi

echo ""
echo "PASSED"
