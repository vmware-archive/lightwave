#!/bin/bash

_STATUS_FILE=/tmp/vmware-sts-config-status
TC_ROOT_PATH=$VMWARE_TCROOT
WEBAPPS_PATH=/opt/vmware/webapps
TC_INSTANCE_ROOT=/usr/lib/vmware-sso
TC_INSTANCE_NAME=vmware-sts
TC_INSTANCE_BASE=$TC_INSTANCE_ROOT/$TC_INSTANCE_NAME
SSO_LOG_PATH=/var/log/vmware/sso

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

function set_tc_root_path
{
    echo "Setting TC Root Path"

    if [ -z "$TC_ROOT_PATH" ]; then
        TC_ROOT_PATH=/opt/vmware/vfabric-tc-server-standard
        if [ ! -d $TC_ROOT_PATH ]; then
           TC_ROOT_PATH=/usr/local/tcserver/vfabric-tc-server-standard
        fi
    fi

    if [ ! -d $TC_ROOT_PATH ]; then
        bail_on_error 1 "Error: Path does not exist - [$TC_ROOT_PATH]"
    else
        return 0
    fi
}

function copy_tc_template
{
    echo "Copying TC Templates"

    BIO_SSL_LOCALHOST_PATH=$TC_ROOT_PATH/templates/bio-ssl-localhost

    if [ -d $BIO_SSL_LOCALHOST_PATH ]; then
        /bin/rm -rf $BIO_SSL_LOCALHOST_PATH
    fi

    /bin/cp -r $WEBAPPS_PATH/bio-ssl-localhost $BIO_SSL_LOCALHOST_PATH
    bail_on_error $? "Error : Failed to setup bio-ssl-localhost"

    BIO_CUSTOM_PATH=$TC_ROOT_PATH/templates/bio-custom

    if [ -d $BIO_CUSTOM_PATH ]; then
        /bin/rm -rf $BIO_CUSTOM_PATH
    fi

    /bin/cp -r $WEBAPPS_PATH/bio-custom $BIO_CUSTOM_PATH
    bail_on_error $? "Error : Failed to setup bio-custom"

    return 0
}

function create_tc_instance
{
    echo "Creating TC Instance"

    /bin/mkdir -m 700 -p $TC_INSTANCE_ROOT
    bail_on_error $?  "Failed to create folder at [$TC_INSTANCE_ROOT]"

    (
        cd $TC_INSTANCE_ROOT

        $TC_ROOT_PATH/tcruntime-instance.sh create \
                -t bio-custom \
                --property bio-custom.http.port=7080 \
                --force \
                -t bio-ssl-localhost \
                --property bio-ssl-localhost.https.port=7444 \
                $TC_INSTANCE_NAME
        bail_on_error $? "Failed to create TC instance"
    )

    TC_INSTANCE_LOG_PATH=$TC_INSTANCE_BASE/logs
    if [ -d $TC_INSTANCE_LOG_PATH ]; then
        /bin/rm -rf $TC_INSTANCE_LOG_PATH
    fi

    mkdir -m 755 -p $SSO_LOG_PATH
    bail_on_error $? "Failed to create path [$SSO_LOG_PATH]"

    /bin/ln -s $SSO_LOG_PATH $TC_INSTANCE_LOG_PATH
    bail_on_error $? "Error: Failed to create symlink [$TC_INSTANCE_LOG_PATH->$SSO_LOG_PATH]"

    if [ -d $TC_INSTANCE/conf ]; then
        chown -R root $TC_INSTANCE_BASE/conf
        bail_on_error $? "Error: Failed to set ownership of TC configuration folder"

        chmod -R 700 $TC_INSTANCE_BASE/conf
        bail_on_error $? "Error: Failed to set permissions of TC configuration folder"
    fi

    return 0
}

function installWarFiles
{
    echo "Installing war files"

    WAR_FILES="afd.war \
               idm.war \
               ims.war \
               lookupservice.war \
               openidconnect.war \
               sso-adminserver.war \
               sts.war \
               websso.war"

    for f in $WAR_FILES
    do
        echo "Installing file [$TC_INSTANCE_BASE/webapps/$f]"
        /bin/cp -f $WEBAPPS_PATH/$f $TC_INSTANCE_BASE/webapps/
        bail_on_error $? "Failed to install file [$TC_INSTANCE_BASE/webapps/$f]"
    done

    return 0
}

function installSSLCert
{
    keys_path=/etc/vmware-sso/keys

    echo "Installing SSL Cert for TC instance"

    /bin/cp -f $keys_path/ssoserver.crt /usr/lib/vmware-sso/vmware-sts/conf/
    bail_on_error $? "Failed to copy SSL cert for STS instance configuration"

    return 0
}


function init_tc_instance
{
    echo "Installing SSL certificates for TC Instance"

    installSSLCert
    bail_on_error $? "Failed to initialize TC instance"

    installWarFiles
    bail_on_error $? "Failed to initialize TC instance"

    return 0
}

#
# Main
#

trap cleanup SIGINT SIGHUP SIGTERM

export JAVA_HOME=/usr/java/jre-vmware

echo "Configuring VMware STS"

set_tc_root_path
bail_on_error $? "Failed to set TC Root"

copy_tc_template
bail_on_error $? "Failed to copy TC templates"

create_tc_instance
bail_on_error $? "Failed to create TC instance"

init_tc_instance
bail_on_error $? "Failed to initialize TC instance"

echo "VMware STS configured successfully"

cleanup 0

