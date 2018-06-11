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

#
# Main
#

trap cleanup SIGINT SIGHUP SIGTERM

echo "Configuring VMware STS"

echo "VMware STS configured successfully"

cleanup 0

