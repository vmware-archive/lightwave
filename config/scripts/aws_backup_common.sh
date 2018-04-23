source /var/vmware/lightwave/scripts/common.sh

#Returns list of instance IDs of instances within the ASG
get_node_list() {
    ID_LIST=($(aws autoscaling describe-auto-scaling-groups \
        --auto-scaling-group-names ${ASG_NAME} \
        --region ${EC2_REGION} \
        --query AutoScalingGroups[].Instances[].InstanceId \
        --output text))
    if [ $? -ne 0 ]; then
        exit
    fi
}

#Returns name of latest archive.zip that has been uploaded by any node in the region to S3
get_latest_bkp() {
    LATEST=`aws s3 ls $FULL_PATH/ | grep "archive.zip" | sort | tail -n 1 | awk '{print $4}'`
}

#Chek if the bucket exists. If not, create it
check_or_create_bucket() {
    if aws s3 ls "s3://$BUCKET" 2>&1 | grep -q 'NoSuchBucket'
    then
        logger -t run_backup "Creating bucket "$BUCKET
        aws s3api create-bucket --bucket $BUCKET --region $REGION --create-bucket-configuration LocationConstraint=$REGION
        if [ $? -ne 0 ]; then
            exit
        fi
    fi
}

get_account_id() {
    ACCOUNT_ID=`curl -s http://169.254.169.254/latest/dynamic/instance-identity/document | grep accountId | cut -d ":" -f 2 | cut -d "\"" -f 2`
    if [ $? -ne 0 ]; then
        logger -t backup "Error while getting account id"
        exit
    fi
}

upload_to_cloud() {
    aws s3api put-object --bucket $BUCKET --key $1 --body $2 --metadata md5chksum=$3 --content-md5 $3
    if [ $? -ne 0 ]; then
        return 1
    fi
    return 0
}

#Get list of all files in the backup directory
get_bkp_list() {
    LIST=`aws s3 ls $FULL_PATH/ | grep archive`
    if [ $? -ne 0 ]; then
        exit
    fi
}

delete_file() {
    aws s3 rm s3://$FULL_PATH/$1
}

REGION=$EC2_REGION
get_account_id
BUCKET=dr-$ACCOUNT_ID-$REGION
FULL_PATH=$BUCKET/lw-dr/$ASG_NAME
LW_BKP_PATH=lw-dr/$ASG_NAME
