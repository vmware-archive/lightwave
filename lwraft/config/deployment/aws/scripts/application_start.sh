#!/bin/bash -e

source $(dirname $(realpath $0))/common.sh

echo "Step 1: Get domain, password, and existing partners from AWS"

get_tag_value "LW_DOMAIN" LW_DOMAIN
echo "LW_DOMAIN=${LW_DOMAIN}"

find_post_partners PARTNERS
echo "PARTNERS=${PARTNERS[*]}"

get_post_password POST_PASSWORD


echo "Step 2: Start POST and VmAfd"

/opt/likewise/bin/lwsm start vmafd
echo "VmAfd started successfully"

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

echo "Step 4: Reaffinitize to DC"
set_dc_name

echo "Step 5: Generate SSL cert if it does not exist"
generate_ssl_cert
