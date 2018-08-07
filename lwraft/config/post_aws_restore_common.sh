#!/bin/bash
source /var/vmware/lightwave/scripts/common.sh

get_params() {
    ADMIN_USER="administrator"
    get_post_password ADMIN_PASS
}

download_db() {
    aws s3 cp s3://$1 $2
}
