#!/bin/bash

export PATH=$PATH:/root/.local/bin

logger -t monitor-core-dump "Started"

logger -t monitor-core-dump "Step 1: Get S3 bucket location"

INSTANCE=$(curl -sS http://169.254.169.254/latest/meta-data/instance-id)
REGION=$(curl -s http://169.254.169.254/latest/meta-data/placement/availability-zone | sed -e "s:\([0-9][0-9]*\)[a-z]*\$:\\1:")
ASG=$(aws autoscaling describe-auto-scaling-instances --instance-ids ${INSTANCE} --region ${REGION} --query AutoScalingInstances[].AutoScalingGroupName --output text)
S3_BUCKET=$(aws autoscaling describe-auto-scaling-groups --auto-scaling-group-names ${ASG} --region ${REGION} --query AutoScalingGroups[].Tags[?Key==\'COREDUMP_BUCKET_PATH\'].Value --output text)

logger -t monitor-core-dump "Step 2: Get a list of coredumps in the past hour"

FILES=$(find /var/lib/systemd/coredump/core* -mmin 60)

if [ $? -ne 0 ]
then
    exit 0
fi

BUILDNO=$(rpm -q --queryformat "%{NAME}-%{VERSION}~%{RELEASE}\n" lightwave)

logger -t monitor-core-dump "Step 3: Upload to S3 bucket"
for FILE in $FILES
do
    aws s3 cp $FILE "${S3_BUCKET}lightwave/$BUILDNO/"
done

logger -t monitor-core-dump "Completed"
