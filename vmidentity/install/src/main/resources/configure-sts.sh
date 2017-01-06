#!/bin/bash

_STATUS_FILE=/tmp/vmware-sts-config-status
TC_INSTANCE_ROOT=/opt/vmware
TC_INSTANCE_NAME=vmware-sts
TC_INSTANCE_BASE=$TC_INSTANCE_ROOT/$TC_INSTANCE_NAME
SSO_LOG_PATH=/var/log/vmware/sso
TC_CONF_PATH=$TC_INSTANCE_BASE/conf/

function bail_on_error()
{
    local errcode=0
    local message=$2

    if [ ! -z "$1" ]; then
        errcode=$1
    fi

    if [ ! -z "$2" ]; then
        message=$2
    fi

    if [[ $errcode -ne 0 ]]
    then
        echo $message
        cleanup $errcode
    fi
}

function cleanup()
{
    local STATUS=1

    if [ ! -z "$1" ]
    then
        STATUS=$1
    fi

    echo $STATUS > $_STATUS_FILE

    if [[ "$STATUS" != "0" ]]
    then
        exit 1
    else
        exit 0
    fi
}

function copy_tc_security_override
{
    echo "Copying Security Override to TC conf"

    /bin/cp -f /etc/vmware/java/vmware-override-java.security $TC_CONF_PATH/vmware-identity-override-java.security
    bail_on_error $? "Failed to install Security Override file"
}

function config_tc_log
{
    echo "Creating TC Instance"

    TC_INSTANCE_LOG_PATH=$TC_INSTANCE_BASE/logs
    if [ -d $TC_INSTANCE_LOG_PATH ]; then
        /bin/rm -rf $TC_INSTANCE_LOG_PATH
    fi

	mkdir -m 755 -p $SSO_LOG_PATH
	bail_on_error $? "Failed to create path [$SSO_LOG_PATH]"

    #Create symbolic link for logs
    /bin/ln -s $SSO_LOG_PATH $TC_INSTANCE_LOG_PATH
    bail_on_error $? "Error: Failed to create symlink [$TC_INSTANCE_LOG_PATH->$SSO_LOG_PATH]"

    return 0
}

#
# Main
#

trap cleanup SIGINT SIGHUP SIGTERM

echo "Configuring VMware STS"

config_tc_log
bail_on_error $? "Failed to create TC instance"

copy_tc_security_override
bail_on_error $? "Failed to copy security override"

echo "VMware STS configured successfully"

cleanup 0

