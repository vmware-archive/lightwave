#!/bin/bash -e

APP_NAME=$1
DG_NAME=$2
ASG_NAME=$3
REGION=$4
VPC_ID=$5
S3_BUCKET=$6
S3_KEY=$7

#Delete hook to prevent automatic deployment
HOOK_NAME=$(aws autoscaling describe-lifecycle-hooks --auto-scaling-group-name $ASG_NAME --region $REGION --query 'LifecycleHooks[0].LifecycleHookName')
HOOK_NAME=`echo $HOOK_NAME | tr -d '"'`
aws autoscaling delete-lifecycle-hook --auto-scaling-group-name $ASG_NAME --region $REGION --lifecycle-hook-name $HOOK_NAME
echo "Deleted hook "$HOOK_NAME

#Set default DNS
CIDR=$(aws ec2 describe-vpcs --vpc-ids $VPC_ID --region $REGION --query Vpcs[0].CidrBlockAssociationSet[0].CidrBlock | tr -d '"')
DEFAULT_DNS=$(echo ${CIDR} | sed -e 's/\.[^\.]*$/\.2/' <<< ${CIDR})
echo "Setting default DNS "$DEFAULT_DNS
DOPT_ID=$(aws --region $REGION ec2 create-dhcp-options --dhcp-configurations Key=domain-name-servers,Values=$DEFAULT_DNS --query DhcpOptions.DhcpOptionsId --output text)
aws ec2 create-tags --region $REGION --resources $DOPT_ID --tags Key=Name,Value=$ASG_NAME
aws ec2 associate-dhcp-options --region $REGION --dhcp-options-id $DOPT_ID --vpc-id $VPC_ID

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

echo "SSH into VM and run lw_restore.sh"
