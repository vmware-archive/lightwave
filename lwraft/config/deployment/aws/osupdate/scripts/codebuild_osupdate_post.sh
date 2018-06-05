#!/bin/bash

AMI_FILE="ami-file"
MANIFESTFILE=rpmmanifest
RPMUPDATELISTFILE=rpmupdatelist
REBOOTLISTFILE=rebootlist
RPMLISTFILE=rpmlist

#The following will be passed down as environment variables from codebuild
APPLICATION_NAME=${POST_DEPLOYMENT_APP}
DEPLOYMENT_GROUP_NAME=${POST_DEPLOYMENT_DG}
SOURCE_PATH="${SOURCE_PATH}"
BUCKET_NAME=${BUCKET_NAME}
MANIFEST_PATH=${MANIFEST_PATH}

CODEBUILD_PATH="/var/vmware/codebuild"

if [ ! -f $AMI_FILE ]; then
    echo "No ami file present"
    exit 0
fi
if [ ! -f $RPMUPDATELISTFILE ]; then
    echo "No rpm update list file present"
    exit 1
fi
if [[ ! -s $RPMUPDATELISTFILE ]]; then
    echo "Rpm update list file empty!"
    exit 1
fi

if [ -z "$APPLICATION_NAME" ] || [ -z "$DEPLOYMENT_GROUP_NAME" ] || [ -z "$SOURCE_PATH" ] || [ -z "$BUCKET_NAME" ] || [ -z "$MANIFEST_PATH" ]; then
    echo "Environment variables not set properly"
    echo "APPLICATION_NAME: $APPLICATION_NAME, DEPLOYMENT_GROUP_NAME: $DEPLOYMENT_GROUP_NAME, SOURCE_PATH: $SOURCE_PATH, BUCKET_NAME: $BUCKET_NAME, MANIFEST_PATH: $MANIFEST_PATH"
    exit 1
fi

ASG_NAME=`aws deploy get-deployment-group --application-name $APPLICATION_NAME --deployment-group-name $DEPLOYMENT_GROUP_NAME --output=text --query 'deploymentGroupInfo.autoScalingGroups[0].name'`
if [ -z "$ASG_NAME" ] || [ "$ASG_NAME" == "None" ]; then
    echo "No ASG found for deployment $DEPLOYMENT_GROUP_NAME"
    exit 1
fi

LAUNCH_CONFIG=`aws autoscaling describe-auto-scaling-groups --auto-scaling-group-names $ASG_NAME --output=text --query 'AutoScalingGroups[0].LaunchConfigurationName'`
if [ -z "$LAUNCH_CONFIG" ] || [ "$LAUNCH_CONFIG" == "None" ]; then
    echo "No launch config attached to asg $ASG"
    exit 1
fi

EC2_REGION=$AWS_REGION
if [ -z "$EC2_REGION" ]; then
    echo "ASG_NAME: $ASG_NAME, LAUNCH_CONFIG: $LAUNCH_CONFIG"
    exit 1
fi

IMAGE_ID=`grep $EC2_REGION $AMI_FILE | cut -d':' -f2`

if [[ "$IMAGE_ID" == "" ]]; then
    echo "AMI ID not found"
    exit 1
fi
LAUNCH_CONFIG_PREFIX=$ASG_NAME
NEW_LAUNCH_CONFIG="$LAUNCH_CONFIG_PREFIX-$IMAGE_ID"

if [ ! -f ./jq ]; then
    echo "jq not found!"
    exit 1
fi

ELB=`aws autoscaling describe-load-balancers --auto-scaling-group-name $ASG_NAME --query "LoadBalancers[0].LoadBalancerName"  | sed -e 's/^"//' -e 's/"$//'`
if [ $ELB == "" ] || [ $ELB == "None" ] ; then
    echo "No ELB found attached to $ASG_NAME"
    exit 1
fi

CloneLaunchConfig()
{
    #Copy configuration
    USERDATA="file://cloud-init"
    LAUNCHCONFIGDATA=`aws autoscaling describe-launch-configurations --launch-configuration-names $LAUNCH_CONFIG --query 'LaunchConfigurations[0]'`
    INSTANCETYPE=`echo $LAUNCHCONFIGDATA | ./jq -r '.InstanceType'`
    IAMROLE=`echo $LAUNCHCONFIGDATA | ./jq -r '.IamInstanceProfile'`
    SECURITYGROUP=`echo $LAUNCHCONFIGDATA | ./jq -r '.SecurityGroups[]'`
    KEYNAME=`echo $LAUNCHCONFIGDATA | ./jq -r '.KeyName'`
    IPADDRESSPUBLIC=`echo $LAUNCHCONFIGDATA | ./jq -r '.AssociatePublicIpAddress'`
    NUM_BM=`echo $LAUNCHCONFIGDATA | ./jq -r '.BlockDeviceMappings' | ./jq length`

    BD_MAPPINGS=""
    for i in `seq 0 1 $(expr $NUM_BM - 1)`; do
        if [[ ! -z $BD_MAPPINGS ]]
        then
          BD_MAPPINGS=$BD_MAPPINGS","
        fi
        echo $BD_MAPPINGS
        DEVICE=`echo $LAUNCHCONFIGDATA | ./jq -r ".BlockDeviceMappings[$i].DeviceName"`
        TYPE_EBS=`echo $LAUNCHCONFIGDATA | ./jq -r ".BlockDeviceMappings[$i].Ebs"`
        if [[ $TYPE_EBS == "null" ]] || [[ $TYPE_EBS == "None" ]] ; then
            VIRTUALNAME=`echo $LAUNCHCONFIGDATA | ./jq -r ".BlockDeviceMappings[$i].VirtualName"`
            BD_MAPPINGS=$BD_MAPPINGS"{\"DeviceName\": \"${DEVICE}\",\"VirtualName\": \"${VIRTUALNAME}\"}"
        else
            VOLUMESIZE=`echo $LAUNCHCONFIGDATA | ./jq -r ".BlockDeviceMappings[$i].Ebs.VolumeSize"`
            VOLUME_TYPE=`echo $LAUNCHCONFIGDATA | ./jq -r ".BlockDeviceMappings[$i].Ebs.VolumeType"`
            DELETE_ON_TERMINATION=`echo $LAUNCHCONFIGDATA | ./jq -r ".BlockDeviceMappings[$i].Ebs.DeleteOnTermination"`
            IOPS=`echo $LAUNCHCONFIGDATA | ./jq -r ".BlockDeviceMappings[$i].Ebs.Iops"`

            if [[ "$IOPS" == "null" ]] || [[ "$IOPS" == "None" ]]; then
                IOPS_PARAM=''
            else
                IOPS_PARAM=",\"Iops\": $IOPS"
            fi

            if [[ "$DELETE_ON_TERMINATION" == "false" ]]; then
                DEL_TERMINATE=",\"DeleteOnTermination\":false"
            elif [[ "$DELETE_ON_TERMINATION" == "true" ]]; then
                DEL_TERMINATE=",\"DeleteOnTermination\":true"
            else
                DEL_TERMINATE=''
            fi
            BD_MAPPINGS=$BD_MAPPINGS"{\"DeviceName\": \"${DEVICE}\",\"Ebs\":{\"VolumeSize\":${VOLUMESIZE}"${DEL_TERMINATE}",\"VolumeType\": \"$VOLUME_TYPE\""${IOPS_PARAM}"}}"
        fi
    done

    BD_MAPPINGS="["$BD_MAPPINGS"]"

    INSTANCE_MONITORING=`echo $LAUNCHCONFIGDATA | ./jq -r '.InstanceMonitoring.Enabled'`
    EBS_OPTIMIZED=`echo $LAUNCHCONFIGDATA | ./jq -r '.EbsOptimized'`

    if [[ "$EBS_OPTIMIZED" == "false" ]]; then
        EBS_OPTIMISATION="--no-ebs-optimized"
    else
        EBS_OPTIMISATION="--ebs-optimized"
    fi

    if [[ "$INSTANCE_MONITORING" == "false" ]]; then
        ENABLE_MONITORING="false"
    else
        ENABLE_MONITORING="true"
    fi

    if [[ "$SECURITYGROUP" == "None" ]]; then
        SEC_GROUP=""
    else
        SEC_GROUP="--security-groups "
        for secgp in $SECURITYGROUP; do
            SEC_GROUP=$SEC_GROUP"$secgp "
        done
    fi

    if [[ ! -z $KEYNAME ]]; then
        KEYNAME="--key-name $KEYNAME"
    fi

    if [[ ! -z $IAMROLE ]]; then
        IAMROLE="--iam-instance-profile $IAMROLE"
    fi
    if [[ "$IPADDRESSPUBLIC" == "true" ]]; then
        IPADDRESSPUBLIC="--associate-public-ip-address"
    elif [[ "$IPADDRESSPUBLIC" == "false" ]]; then
        IPADDRESSPUBLIC="--no-associate-public-ip-address"
    else
        IPADDRESSPUBLIC=""
    fi

    LAUNCHCONFIGEXISTS=`aws autoscaling describe-launch-configurations --launch-configuration-names $NEW_LAUNCH_CONFIG --query "length(LaunchConfigurations[])"`
    if [ $LAUNCHCONFIGEXISTS -ne "0" ]; then
        aws autoscaling delete-launch-configuration --launch-configuration-name $NEW_LAUNCH_CONFIG
    fi

    aws autoscaling create-launch-configuration \
                    --launch-configuration-name $NEW_LAUNCH_CONFIG \
                    --image-id $IMAGE_ID \
                    $KEYNAME \
                    $SEC_GROUP\
                    $EBS_OPTIMISATION \
                    --instance-monitoring Enabled=$ENABLE_MONITORING \
                    $IAMROLE \
                    $IPADDRESSPUBLIC \
                    --instance-type $INSTANCETYPE \
                    --user-data $USERDATA \
                    --block-device-mappings "$BD_MAPPINGS"
}

ssm_send_command()
{
    local instance=$1
    local max_attempts=$2
    local commandtype=$3
    local comm="$4"
    local sourceinfo="$5"
    local exitcode=0
    local curr_attempt=0
    local pattern=""
    local docname="AWS-RunShellScript"
    local parameters="commands=\"$comm\""

    if [ $commandtype == "remote" ]
    then
        docname="AWS-RunRemoteScript"
        parameters="{\"sourceType\":[\"S3\"],\"sourceInfo\":[\"{\\\"path\\\": \\\"$sourceinfo\\\"}\"],\"commandLine\":[\"$comm\"]}"
        if [ $# -ge 6 ]
        then
            pattern="$6"
        fi
    elif [ $# -ge 5 ]
    then
        pattern="$5"
    fi
    local sh_command_id=$(aws ssm send-command --instance-ids $instance --document-name "$docname" --parameters "$parameters" --output text --query "Command.CommandId")
    sleep 5
    local sh_status=`aws ssm list-command-invocations --command-id "$sh_command_id" --details --query "CommandInvocations[0].Status"  | sed -e 's/^"//' -e 's/"$//'`

    local sh_stdout=`aws ssm list-command-invocations --command-id "$sh_command_id" --details --query "CommandInvocations[0].CommandPlugins[0].Output"`

    while [[ $sh_status == "InProgress" ]] && [ $(( curr_attempt )) -lt $(( max_attempts )) ]
    do
        sleep 5
        sh_status=`aws ssm list-command-invocations --command-id "$sh_command_id" --details --query "CommandInvocations[0].Status"  | sed -e 's/^"//' -e 's/"$//'`
        echo $sh_status
        curr_attempt=$(( curr_attempt + 1 ))
    done
    sh_stdout=`aws ssm list-command-invocations --command-id "$sh_command_id" --details --query "CommandInvocations[0].CommandPlugins[0].Output"`
    if [[ $sh_status == "Success" ]]; then
        if [[ ! -z "$pattern" ]]
        then
            echo $sh_stdout | grep "$pattern"
            if [[ $? -ne 0 ]]; then
                exitcode=1
            fi
        else
            exitcode=0
        fi
    else
        echo $sh_stdout
        exitcode=1
    fi
    return $exitcode
}

PerformUpdateOnInstances()
{
    ASG_DATA=`aws autoscaling describe-auto-scaling-groups --auto-scaling-group-names $ASG_NAME`
    NUMINSTANCES=`echo $ASG_DATA | ./jq -r '.AutoScalingGroups[].Instances' | ./jq length`

    if [ $NUMINSTANCES == "0" ] ; then
        echo "No instances found attached to the ASG : $ASG_NAME"
        return 1
    fi

    for i in `seq 0 1 $(expr $NUMINSTANCES - 1)`; do
        INSTANCE_ID=`echo $ASG_DATA | ./jq -r ".AutoScalingGroups[].Instances[$i].InstanceId"`
        if [ $INSTANCE_ID == "" ] || [ $INSTANCE_ID == "null" ] ; then
            echo "Instance not found!!"
            echo $ASG_DATA
            return 1
        fi

        #Prepare for update
        local cmd="rm -rf $CODEBUILD_PATH && mkdir -p $CODEBUILD_PATH && unzip -o archive.zip -d $CODEBUILD_PATH"
        ssm_send_command $INSTANCE_ID 30 "remote" "$cmd" "$SOURCE_PATH"
        if [[ $? -ne 0 ]]; then
            echo "Send command $cmd Failed"
            return 1
        fi

        #Deregister from load balancer
        DEREG_OUT=`aws elb deregister-instances-from-load-balancer --load-balancer-name $ELB --instances $INSTANCE_ID`
        echo $DEREG_OUT | grep $INSTANCE_ID
        if [[ $? -eq 0 ]]; then
            echo "Could not deregister $INSTANCE_ID from $ELB"
            return 1
        fi

        #scripts/after_block_traffic.sh
        #scripts/stop_server.sh
        local cmd="LOGDIR=$CODEBUILD_PATH $CODEBUILD_PATH/scripts/application_stop.sh"
        ssm_send_command $INSTANCE_ID 30 "shell" "$cmd"
        if [[ $? -ne 0 ]]; then
            echo "Send command $cmd Failed"
            #We can try re-registering here to not disrupt service
            RestartServicesAndRegisterWithELB $INSTANCE_ID
            return 1
        fi

        #Perform the update
        RPMLIST=`tr '\n' ' ' < $RPMUPDATELISTFILE`
        if [[ "$RPMLIST" != "" ]]; then
            cmd="tdnf install -y $RPMLIST"
            ssm_send_command $INSTANCE_ID 30 "shell" "$cmd"
            # Retry with downgrade since install does not handle specific version-release dependencies correctly
            if [[ $? -ne 0 ]]; then
                cmd="tdnf downgrade -y $RPMLIST"
                ssm_send_command $INSTANCE_ID 30 "shell" "$cmd"
                if [[ $? -ne 0 ]]; then
                    echo "Send command $cmd Failed"
                    #We can try re-registering here to not disrupt service
                    RestartServicesAndRegisterWithELB $INSTANCE_ID
                    return 1
                fi
            fi
        fi

        #Reboot the instance if needed
        REBOOTREQUIRED=false
        grep -f $REBOOTLISTFILE $RPMUPDATELISTFILE
        if [[ $? -eq 0 ]]; then
            cmd="reboot"
            ssm_send_command $INSTANCE_ID 40 "shell" "$cmd"
            if [[ $? -ne 0 ]]; then
                echo "Send command $cmd Failed"
                #Do not exit here, ssm incorrectly reports failures during some reboots
                #A failed reboot can be caught in the check below
            fi
        fi

        #Restart operations
        #scripts/start_server.sh
        #scripts/before_allow_traffic.sh
        #Register to the load balancer
        RestartServicesAndRegisterWithELB $INSTANCE_ID
        if [[ $? -ne 0 ]]; then
            echo "Could not restart services and register"
            return 1
        fi

    done
    return 0
}

RestartServicesAndRegisterWithELB()
{
    INSTANCE_ID=$1

    local cmd="LOGDIR=$CODEBUILD_PATH $CODEBUILD_PATH/scripts/application_start.sh"
    ssm_send_command $INSTANCE_ID 20 "shell" "$cmd"
    if [[ $? -ne 0 ]]; then
        echo "Send command $cmd Failed"
        return 1
    fi

    cmd="LOGDIR=$CODEBUILD_PATH $CODEBUILD_PATH/scripts/validate_service.sh"
    ssm_send_command $INSTANCE_ID 20 "shell" "$cmd"
    if [[ $? -ne 0 ]]; then
        echo "Send command $cmd Failed"
        return 1
    fi

    REG_OUT=`aws elb register-instances-with-load-balancer --load-balancer-name $ELB --instances $INSTANCE_ID`

    echo $REG_OUT | grep $INSTANCE_ID
    if [[ $? -ne 0 ]]; then
        echo "Could not register $INSTANCE_ID with $ELB"
        return 1
    fi
}

CheckAndUploadManifest()
{
    ASG_DATA=`aws autoscaling describe-auto-scaling-groups --auto-scaling-group-names $ASG_NAME`
    NUMINSTANCES=`echo $ASG_DATA | ./jq -r '.AutoScalingGroups[].Instances' | ./jq length`

    if [ $NUMINSTANCES == "0" ] ; then
        echo "No instances found attached to the ASG : $ASG_NAME"
        return 1
    fi

    for i in `seq 0 1 $(expr $NUMINSTANCES - 1)`; do
        INSTANCE_ID=`echo $ASG_DATA | ./jq -r ".AutoScalingGroups[].Instances[$i].InstanceId"`
        if [ $INSTANCE_ID == "" ] || [ $INSTANCE_ID == "null" ] ; then
            echo "Instance not found!!"
            echo $ASG_DATA
            return 1
        fi
        cmdId=$(aws ssm send-command --instance-ids $INSTANCE_ID --document-name "AWS-RunShellScript" --query "Command.CommandId" --output text  --output-s3-bucket-name "$BUCKET_NAME" --output-s3-key-prefix "$MANIFEST_PATH" --parameters commands="rpm -qa | grep "\.ph2" | grep -v lwph2")
        while [ "$(aws ssm list-command-invocations --command-id "$cmdId" --query "CommandInvocations[].Status" --output text)" == "InProgress" ]; do sleep 5; done
        outputPath=$(aws ssm list-command-invocations --command-id "$cmdId" --details --query "CommandInvocations[].CommandPlugins[].OutputS3KeyPrefix" --output text)
        aws s3 ls --recursive s3://${BUCKET_NAME}/${outputPath}/ | grep stderr
        if [ $? -eq 0 ]; then
            echo "Error while getting manifest"
            return 1
        fi
        STDOUT=`aws s3 ls --recursive s3://${BUCKET_NAME}/${outputPath}/ | grep stdout | awk -F' ' '{print $4}'`
        aws s3 cp s3://$BUCKET_NAME/$STDOUT ./stdout
        if [ $? -ne 0 ]; then
            echo "Error while getting manifest"
            return 1
        fi
        sed -i '/^$/d' $RPMUPDATELISTFILE #Remove blank lines
        sed -i '/^$/d' ./stdout
        grep -vf ./stdout $RPMUPDATELISTFILE
        if [ $? -eq 0 ]; then
            echo "Update list and rpm manifest do not match for instance $INSTANCE_ID"
            return 1
        fi
    done
    aws s3 cp ./stdout s3://$BUCKET_NAME/$MANIFEST_PATH/$MANIFESTFILE
    if [ $? -ne 0 ]; then
        echo "Error while getting manifest"
        return 1
    fi
    return 0
}

main()
{
    local rc=0
    OLD_IMAGE_ID=`aws autoscaling describe-launch-configurations --launch-configuration-names $LAUNCH_CONFIG  --output=text --query 'LaunchConfigurations[0].ImageId'`
    if [[ $OLD_IMAGE_ID != $IMAGE_ID ]]
    then
        #Create a copy of the launch config with the new ami id
        CloneLaunchConfig
        #TODO:Clean up older launch configurations -tracked by PR 2125965
        if [ $? -eq 0 ]
        then
            aws autoscaling update-auto-scaling-group --auto-scaling-group-name $ASG_NAME --launch-configuration-name $NEW_LAUNCH_CONFIG
            if [ $? -ne 0 ]
            then
                echo "Updating auto scaling group with new launch config failed"
                exit 1
            fi
        else
            echo "Launch Config not copied, exiting"
            exit 1
    fi
    fi

    PerformUpdateOnInstances
    if [ $? -ne 0 ]
    then
        rc=1
    fi

    CheckAndUploadManifest
    if [ $? -ne 0 ]; then
        rc=1
    fi

    exit $rc
}

main "$@"
