#!/bin/bash

get_params() {
    ADMIN_USER="administrator"
    get_tag_value PASSWORD_PATH ADMIN_USER_PASS_BUCKET
    set +x
    ADMIN_PASS=$(aws s3 cp ${ADMIN_USER_PASS_BUCKET} -)
    set -x
}

download_db() {
    aws s3 cp s3://$1 $2
}
