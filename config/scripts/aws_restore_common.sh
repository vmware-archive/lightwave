#!/bin/bash

get_san() {
    NAME=$(get_tag_value SAN_ENTRY ${ASG_NAME} ${EC2_REGION})
}

get_params() {
    LW_DOMAIN=$(get_tag_value LW_DOMAIN ${ASG_NAME} ${EC2_REGION})
    DOMAIN_PROMOTER_PASS_BUCKET=$(get_tag_value PROMOTER_PASS_BUCKET ${ASG_NAME} ${EC2_REGION})
    SRV_ACCT_MGR_PASS_BUCKET=$(get_tag_value SRV_ACCT_MGR_PASS_BUCKET ${ASG_NAME} ${EC2_REGION})
    SRV_ACCT_MGR_USR=$(get_tag_value SRV_ACCT_MGR_USER ${ASG_NAME} ${EC2_REGION})
    SCHEMA_MGR_PASS_BUCKET=$(get_tag_value SCHEMA_MGR_PASS_BUCKET ${ASG_NAME} ${EC2_REGION})
    JOINER_PASS_BUCKET=$(get_tag_value JOINER_PASS_BUCKET ${ASG_NAME} ${EC2_REGION})
    CASCADE_OU=$(get_tag_value CASCADE_OU ${ASG_NAME} ${EC2_REGION})
    PROMOTER_USER=$(get_tag_value PROMOTER_USER ${ASG_NAME} ${EC2_REGION})
    set +x
    DOMAIN_PROMOTER_PASS=$(aws s3 cp ${DOMAIN_PROMOTER_PASS_BUCKET} -)
    SRV_ACCT_MGR_PASS=$(aws s3 cp ${SRV_ACCT_MGR_PASS_BUCKET} -)
    SCHEMA_MGR_PASS=$(aws s3 cp ${SCHEMA_MGR_PASS_BUCKET} -)
    JOINER_PASS=$(aws s3 cp ${JOINER_PASS_BUCKET} -)
    set -x
}

download_db() {
    aws s3 cp s3://$1 $2
}
