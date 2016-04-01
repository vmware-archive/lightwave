#!/bin/bash

_STATUS_FILE=/tmp/configure-reverse-proxy-status

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

function install_ssl_cert
{
	echo "Retrieving Machine SSL Cert"

	keys_path=/etc/vmware-proxy

	/bin/mkdir -m 700 -p $keys_path
	bail_on_error $? "Failed to create key path"

    /usr/lib/vmware-vmafd/bin/vecs-cli entry getcert \
                                       --store MACHINE_SSL_CERT \
                                       --alias __MACHINE_CERT \
                                       --output $keys_path/machine-ssl-cert.pem
    bail_on_error $? "Failed to get SSL Cert"

    echo "Retrieving Private Key for SSL Cert"

    /usr/lib/vmware-vmafd/bin/vecs-cli entry getkey \
                                       --store MACHINE_SSL_CERT \
                                       --alias __MACHINE_CERT \
                                       --output $keys_path/machine-ssl-key.pem
    bail_on_error $? "Failed to get Private Key for SSL Cert"

    return 0
}

#
# Main
#

trap cleanup SIGINT SIGHUP SIGTERM

echo "Configuring VMware Proxy"

install_ssl_cert
bail_on_error $? "Failed to install SSL cert"

/etc/init.d/vmware-proxyd restart
bail_on_error $? "Failed to restart VMware proxy service"

echo "VMware Proxy configured successfully"

cleanup 0