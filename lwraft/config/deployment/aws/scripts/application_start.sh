#!/bin/bash -e

source $(dirname $(realpath $0))/common.sh

echo "Step 1: Get domain, password, and existing partners from AWS"

get_tag_value "LW_DOMAIN" LW_DOMAIN
echo "LW_DOMAIN=${LW_DOMAIN}"

get_tag_value "POST_PASSWORD" POST_PASSWORD
echo "POST_PASSWORD=<censored>"

find_post_partners PARTNERS
echo "PARTNERS=${PARTNERS[*]}"


echo "Step 2: Start POST"

/opt/likewise/bin/lwsm start post
echo "POST started successfully"


echo "Step 3: Promote POST if necessary"

if [[ -n $(/opt/vmware/bin/post-cli node list --server-name localhost) ]]
then
    echo "Step 3-A: POST is already promoted - No action"
else
    PROMOTED_PARTNER=""
    for PARTNER in ${PARTNERS[@]}
    do
        if [[ -n $(/opt/vmware/bin/post-cli node list --server-name ${PARTNER}) ]]
        then
            PROMOTED_PARTNER=${PARTNER}
            break
        fi
    done

    if [[ -n ${PROMOTED_PARTNER} ]]
    then
        echo "Step 3-B: Promote POST as a subsequent instance (partner=${PROMOTED_PARTNER})"
        /opt/vmware/bin/post-cli node promote --partner-name ${PROMOTED_PARTNER} --password ${POST_PASSWORD}
    else
        echo "Step 3-C: Promote POST as the first instance (domain=${LW_DOMAIN})"
        /opt/vmware/bin/post-cli node promote --domain-name ${LW_DOMAIN} --password ${POST_PASSWORD}
    fi
    echo "POST promoted successfully"
fi
