#!/bin/bash -e

DR_TYPE=$1      #Can be lightwave or post
APP_NAME=$2
DG_NAME=$3
ASG_NAME=$4
REGION=$5
S3_PATH=$6
VPC_ID=$7       #Not needed for POST
ELB_NAME=$8     #Temporary. To be removed once single node upgrade works
S3_BUCKET=`cut -d "/" -f 1 <<< "$S3_PATH"`
S3_KEY=`cut -d "/" -f 2- <<< "$S3_PATH"`

if [ "$DR_TYPE" != "lightwave" ] && [ "$DR_TYPE" != "post" ]; then
    echo "First argument must be either lightwave or post"
    exit
fi

if [ "$DR_TYPE" == "lightwave" ] && [ $# -ne 8 ]; then
    echo "Usage: dr_aws_setup.sh lightwave <Application name> <Deployment group name> <ASG> <Region> <S3 path to LW archive> <VPC ID> <ELB name>"
    exit
elif [ "$DR_TYPE" == "post" ] && [ $# -ne 6 ]; then
    echo "Usage: dr_aws_setup.sh post <Application name> <Deployment group name> <ASG> <Region> <S3 path to post archive>"
    exit
fi

TOTAL_START_TIME="$(date -u +%s)"

#Delete hook to prevent automatic deployment
HOOK_NAME=$(aws autoscaling describe-lifecycle-hooks --auto-scaling-group-name $ASG_NAME --region $REGION --query 'LifecycleHooks[0].LifecycleHookName')
HOOK_NAME=`echo $HOOK_NAME | tr -d '"'`
aws autoscaling delete-lifecycle-hook --auto-scaling-group-name $ASG_NAME --region $REGION --lifecycle-hook-name $HOOK_NAME
echo "Deleted code deploy hook "$HOOK_NAME

#Set default DNS
if [ "$DR_TYPE" == "lightwave" ]; then
    CIDR=$(aws ec2 describe-vpcs --vpc-ids $VPC_ID --region $REGION --query Vpcs[0].CidrBlockAssociationSet[0].CidrBlock | tr -d '"')
    DEFAULT_DNS=$(echo ${CIDR} | sed -e 's/\.[^\.]*$/\.2/' <<< ${CIDR})
    echo "Setting default DNS "$DEFAULT_DNS
    DOPT_ID=$(aws --region $REGION ec2 create-dhcp-options --dhcp-configurations Key=domain-name-servers,Values=$DEFAULT_DNS --query DhcpOptions.DhcpOptionsId --output text)
    aws ec2 create-tags --region $REGION --resources $DOPT_ID --tags Key=Name,Value=$ASG_NAME
    aws ec2 associate-dhcp-options --region $REGION --dhcp-options-id $DOPT_ID --vpc-id $VPC_ID
fi

#Set ASG to 1
aws autoscaling set-desired-capacity --region $REGION --auto-scaling-group-name $ASG_NAME --desired-capacity 1

#Wait for VM to come up
ASG_STATUS=$(aws autoscaling describe-auto-scaling-groups --region $REGION --auto-scaling-group-name $ASG_NAME --output=text --query 'AutoScalingGroups[0].Instances[].LifecycleState')
n=0
START_TIME="$(date -u +%s)"
until ([ $n -ge 40 ] || [ "$ASG_STATUS" == "InService" ] )
do
    sleep 10
    ASG_STATUS=$(aws autoscaling describe-auto-scaling-groups --region $REGION --auto-scaling-group-name $ASG_NAME --output=text --query 'AutoScalingGroups[0].Instances[].LifecycleState')
    n=$[$n+1]
    END_TIME="$(date -u +%s)"
    ELAPSED=$((END_TIME - START_TIME))
    echo "VM status: $ASG_STATUS, Elapsed Time: $ELAPSED seconds"
done
if [ "$ASG_STATUS" != "InService" ]; then
    echo "ASG deployment failed (or timed out)"
    exit 1;
fi

#Trigger deployment
DEPLOYMENT_ID=$(aws deploy create-deployment \
    --region $REGION \
    --application-name $APP_NAME \
    --deployment-config-name CodeDeployDefault.OneAtATime \
    --deployment-group-name $DG_NAME \
    --s3-location bucket=$S3_BUCKET,bundleType=zip,key=$S3_KEY \
    --query 'deploymentId' --output text )

echo "Deployment started: $DEPLOYMENT_ID"

#Wait for deployment to finish
DEPLOYMENT_STATUS=$(aws deploy get-deployment --region $REGION --deployment-id $DEPLOYMENT_ID --query "deploymentInfo.status" --output text)
n=0
START_TIME="$(date -u +%s)"
until ([ $n -ge 100 ] || [ "$DEPLOYMENT_STATUS" == "Succeeded" ] )
do
    sleep 10
    DEPLOYMENT_STATUS=$(aws deploy get-deployment --region $REGION --deployment-id $DEPLOYMENT_ID --query "deploymentInfo.status" --output text)
    n=$[$n+1]
    END_TIME="$(date -u +%s)"
    ELAPSED=$((END_TIME - START_TIME))
    echo "Deployment status: $DEPLOYMENT_STATUS, Elapsed Time: $ELAPSED seconds"
done

if [ "$DEPLOYMENT_STATUS" != "Succeeded" ]; then
    echo "Deployment failed (or timed out)"
    exit 1;
fi

#Dummy update to add the hook that was deleted earlier
aws deploy update-deployment-group --region $REGION --application-name $APP_NAME --current-deployment-group-name $DG_NAME --auto-scaling-groups $ASG_NAME

echo "SUCCESS: Deployment finished."

S3_MDB=${S3_PATH%-*}-data.mdb
INSTANCE=(`aws autoscaling describe-auto-scaling-groups --auto-scaling-group-name $ASG_NAME --region $REGION --output=text --query 'AutoScalingGroups[0].Instances[].InstanceId'`)

if [ "$DR_TYPE" == "lightwave" ]; then
    echo "Running lw_restore.sh on instance $INSTANCE. Restoring from $S3_MDB."
    COMMAND_ID=$(aws ssm send-command --region $REGION --instance-ids "${INSTANCE}" --document-name "AWS-RunShellScript" --comment "DR restore" \
                                      --parameters commands="/opt/vmware/bin/lw_restore.sh $S3_MDB" --output=text --query 'Command.CommandId')
else
    echo "Running post_restore.sh on instance $INSTANCE. Restoring from $S3_MDB."
    COMMAND_ID=$(aws ssm send-command --region $REGION --instance-ids "${INSTANCE}" --document-name "AWS-RunShellScript" --comment "DR restore" \
                                      --parameters commands="/opt/vmware/bin/post_restore.sh $S3_MDB" --output=text --query 'Command.CommandId')
fi

echo "Command started: $COMMAND_ID"

COMMAND_STATUS=`aws ssm list-command-invocations --command-id $COMMAND_ID --details --region $REGION --output=text --query 'CommandInvocations[0].Status'`
n=0
START_TIME="$(date -u +%s)"
until ([ $n -ge 100 ] || [ "$COMMAND_STATUS" == "Success" ] || [ "$COMMAND_STATUS" == "Failed" ])
do
    sleep 10
    COMMAND_STATUS=`aws ssm list-command-invocations --command-id $COMMAND_ID --details --region $REGION --output=text --query 'CommandInvocations[0].Status'`
    n=$[$n+1]
    END_TIME="$(date -u +%s)"
    ELAPSED=$((END_TIME - START_TIME))
    echo "Command status: $COMMAND_STATUS, Elapsed Time: $ELAPSED seconds"
done

aws ssm list-command-invocations --command-id $COMMAND_ID --details --region $REGION --output=text --query 'CommandInvocations[0].CommandPlugins[0].Output'

if [ "$COMMAND_STATUS" != "Success" ]; then
    echo "Command status is $COMMAND_STATUS"
    exit
else
    echo "SUCCESS: Restore script executed"
fi

echo "Upgrading to latest binaries."

#TODO: Remove delete-deployment-group and create-deployment-group steps once single node upgrade works
if [ "$DR_TYPE" == "lightwave" ]; then
    aws deploy delete-deployment-group --region $REGION --application-name $APP_NAME --deployment-group-name $DG_NAME

    IAM_ROLE_ARN=$(aws iam get-role --role-name "gophotonService" --query "Role.Arn" --output text)

    aws deploy create-deployment-group \
      --region $REGION \
      --application-name $APP_NAME \
      --auto-scaling-groups $ASG_NAME \
      --deployment-group-name $DG_NAME \
      --deployment-config-name CodeDeployDefault.OneAtATime \
      --service-role-arn $IAM_ROLE_ARN \
      --deployment-style deploymentType=IN_PLACE,deploymentOption=WITH_TRAFFIC_CONTROL \
      --load-balancer-info elbInfoList=[{name="$ELB_NAME"}]
fi

S3_BUCKET_LATEST=$(aws deploy list-application-revisions \
  --region $REGION \
  --application-name $APP_NAME \
  --sort-by lastUsedTime \
  --sort-order descending \
  --query 'revisions[1].s3Location.bucket' --output text )

S3_KEY_LATEST=$(aws deploy list-application-revisions \
  --region $REGION \
  --application-name $APP_NAME \
  --sort-by lastUsedTime \
  --sort-order descending \
  --query 'revisions[1].s3Location.key' --output text )

DEPLOYMENT_ID=$(aws deploy create-deployment \
  --region $REGION \
  --application-name $APP_NAME \
  --deployment-config-name CodeDeployDefault.OneAtATime \
  --deployment-group-name $DG_NAME \
  --file-exists-behavior "OVERWRITE"\
  --s3-location bucket=$S3_BUCKET_LATEST,bundleType=zip,key=$S3_KEY_LATEST \
  --query 'deploymentId' --output text )

echo "Deployment started: $DEPLOYMENT_ID"

DEPLOYMENT_STATUS=$(aws deploy get-deployment --region $REGION --deployment-id $DEPLOYMENT_ID --query "deploymentInfo.status" --output text)
n=0
START_TIME="$(date -u +%s)"
until ([ $n -ge 100 ] || [ "$DEPLOYMENT_STATUS" == "Succeeded" ] )
do
    sleep 10
    DEPLOYMENT_STATUS=$(aws deploy get-deployment --region $REGION --deployment-id $DEPLOYMENT_ID --query "deploymentInfo.status" --output text)
    n=$[$n+1]
    END_TIME="$(date -u +%s)"
    ELAPSED=$((END_TIME - START_TIME))
    echo "Deployment status: $DEPLOYMENT_STATUS, Elapsed Time: $ELAPSED seconds"
done

if [ "$DEPLOYMENT_STATUS" != "Succeeded" ]; then
        echo "Deployment failed (or timed out)"
        exit 1;
fi

echo "Deployment succeeded"

if [ "$DR_TYPE" == "lightwave" ]; then
    COMM=`aws ssm send-command --region $REGION --instance-ids "${INSTANCE}" --document-name "AWS-RunShellScript" --comment "Restart LW Proxy" --parameters commands="systemctl restart lightwave-proxy"`
fi

TOTAL_END_TIME="$(date -u +%s)"
TOTAL_ELAPSED=$((TOTAL_END_TIME - TOTAL_START_TIME))
echo "Disaster recovery procedure for $DR_TYPE completed. Total time taken = $TOTAL_ELAPSED seconds."
