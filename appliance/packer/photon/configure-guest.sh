#!/bin/bash
# Copyright 2016 VMware, Inc. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License"); you may not
# use this file except in compliance with the License.  You may obtain a copy of
# the License at http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software distributed
# under the License is distributed on an "AS IS" BASIS, without warranties or
# conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the License for the
# specific language governing permissions and limitations under the License.
#
# This script runs on first boot up of the ova to confgigure the system.
XML_FILE=configovf.xml

#
# This script runs on first boot up of the ova to configure the system.
#

OVF_ENV_XML_FILE=configovf.xml
LIGHTWAVE_CFG_FILE=/etc/photon/config/lightwave-server.cfg
LIGHTWAVE_CFG_INSTANCE_FILE=/etc/photon/config/lightwave-server-instance.cfg

function cleanup()
{
    # the XML file contains passwords
    rm -rf $OVF_ENV_XML_FILE

    # the Lightwave config files contain passwords
    rm -rf $LIGHTWAVE_CFG_INSTANCE_FILE

    #remove itself from startup
    systemctl disable configure-guest
}

function set_ntp_servers()
{
    if [ -z "$ntp_servers" ]
    then
        echo "No ntp_servers."
        return
    fi

    # make an array using , as the separator
    IFS=','
    read -a ntp_servers_arr <<< "$ntp_servers"

    # trim the spaces
    IFS=' '
    ntp_servers_arr=(${ntp_servers_arr[@]})

    unset IFS

    cat > "/etc/systemd/timesyncd.conf" <<-EOF
NTP=${ntp_servers_arr[@]}
EOF

    systemctl daemon-reload
    systemctl restart systemd-timesyncd
}

function mask2cidr()
{
    bits=0
    IFS=.
    for dig in $netmask0
    do
        case $dig in
            255) let bits+=8;;
            254) let bits+=7 ; break ;;
            252) let bits+=6 ; break ;;
            248) let bits+=5 ; break ;;
            240) let bits+=4 ; break ;;
            224) let bits+=3 ; break ;;
            192) let bits+=2 ; break ;;
            128) let bits+=1 ; break ;;
            0);;
            *) echo "Error: $dig is not correct"; exit 1
        esac
    done
    unset IFS
    echo "$bits"
}

function set_hostname_registry()
{
    LW_BIN_DIR=/opt/likewise/bin
    IDM_KEY="[HKEY_THIS_MACHINE\\Software\\VMware\\Identity\\Configuration]"
    VAL_NAME="Hostname"

    # Does value exist?
    $LW_BIN_DIR/lwregshell list_values $IDM_KEY | grep $VAL_NAME &>/dev/null

    # If value doesn't exist, create it. Otherwise, set it.
    if [ $? -ne 0 ]; then
        #Add Value
        $LW_BIN_DIR/lwregshell add_value $IDM_KEY $VAL_NAME REG_SZ "$ip0"
    else
        #Set Value
        $LW_BIN_DIR/lwregshell set_value $IDM_KEY $VAL_NAME "$ip0"
    fi

}

function set_network_properties()
{
    if [ ! -z "$lw_hostname" ]
    then
        echo "Setting hostname to $lw_hostname"
        fqdn=$lw_hostname
        hostname=`echo $lw_hostname | awk -F "." '{print $1;}'`
        if [ "$hostname" = "$lw_hostname" ];
        then
            fqdn=$hostname.$lw_domain
        else
            fqdn=$lw_hostname
        fi
        hostnamectl set-hostname $hostname
        hostnamectl set-hostname --static $hostname

        cat > /etc/hosts <<-EOF
#Begin /etc/hosts (network card version)

::1 localhost ipv6-localhost ipv6-loopback
127.0.0.1 localhost
$ip0 $fqdn $hostname
#End /etc/hosts (network card version)
EOF
    fi

    if [ -z "$dns" ]
    then
        multiline_dns=""
    else

        # convert to array using , as seperator
        IFS=','
        read -a dns_arr <<< "$dns"

        #add DNS= to the beginning
        len=${#dns_arr[@]}
        for ((i=0;i<len;i++))
        do
            dns_entry=$(echo "${dns_arr[i]}" | sed 's/^[[:blank:]]*//')
            dns_arr[i]="DNS=${dns_entry}"
        done

        #make it multiline
        multiline_dns=$(IFS=$'\n'; echo "${dns_arr[*]}")

        unset IFS
    fi

    rm -rf /etc/systemd/network/10-dhcp-en.network

    if [ -z "$ip0" ] || [ -z "$netmask0" ] || [ -z "$gateway" ]
    then
        echo "Using DHCP"
        nwConfig="DHCP=yes"
    else
        nwConfig=$(cat <<-EOF
[Address]
Address=${ip0}/$(mask2cidr)
EOF
                )
    fi

    echo "Setting Network properties"

    en_name=$(ip addr show label "e*" | head -n 1 | sed 's/^[0-9]*: \(e.*\): .*/\1/')

    cat > "/etc/systemd/network/10-dhcp-${en_name}.network" <<-EOF
[Match]
Name=$en_name

[Network]
$multiline_dns

$nwConfig

[Route]
Gateway=${gateway}
EOF

    systemctl restart systemd-networkd
}

function set_root_password()
{
    if [ -z "$root_password" ]
    then
        echo "No root_password."
        return
    fi

    echo -e "${root_password}\n${root_password}" | passwd
    exit_code=$?
    if [ 0 -ne $exit_code ]
    then
        echo "password setting failed: $* with $exit_code"
        exit $exit_code
    fi
}

function set_photon_password()
{
    if [ -z "$photon_password" ]
    then
        echo "No photon_password."
        return
    fi

    echo -e "${photon_password}\n${photon_password}" | passwd photon
    exit_code=$?
    if [ 0 -ne $exit_code ]
    then
        echo "password setting failed: $* with $exit_code"
        exit $exit_code
    fi
}

function set_enable_ssh()
{
    if [ ! -z "$enable_ssh" ]
    then
        echo "Enabling root SSH login."
        sed -i 's/PermitRootLogin no/PermitRootLogin yes/g' /etc/ssh/sshd_config
        systemctl restart sshd
        return
    fi
}

function configure_lightwave()
{
    context="{\
    \"DEPLOYMENT\" : \"$lw_deployment\", \
    \"LIGHTWAVE_DOMAIN\" : \"$lw_domain\", \
    \"LIGHTWAVE_PASSWORD\" : \"$lw_password\", \
    \"IS_FIRST_INSTANCE\" : \"$lw_is_first_instance\", \
    \"LIGHTWAVE_HOSTNAME\" : \"$lw_hostname\", \
    \"SSL_SUBJECT_ALT_NAME\" : \"$ip0\" \
  }"
    content=`cat $LIGHTWAVE_CFG_FILE`
    pystache "$content" "$context" > $LIGHTWAVE_CFG_INSTANCE_FILE

    # adding master node reference
    if [[ $lw_deployment == "partner" ]]
    then
        echo "replication-partner-hostname=$lw_replication" >> $LIGHTWAVE_CFG_INSTANCE_FILE
    fi

    # restart lightwave to pick up new config
    systemctl start lwsmd

    mkdir -p /var/log/vmware/sso
    /opt/vmware/bin/configure-lightwave-server --config-file $LIGHTWAVE_CFG_INSTANCE_FILE > /var/log/vmware/sso/configure-lightwave-server.log 2>&1

    set_hostname_registry

    echo "Enabling vmware-idmd to be auto started"
    systemctl enable vmware-idmd

    echo "Enabling vmware-stsd to be auto started"
    systemctl enable vmware-stsd
}

function parse_ovf_env()
{
    # vm config
    ip0=$(xmllint $OVF_ENV_XML_FILE --xpath "string(//*/@*[local-name()='key' and .='ip0']/../@*[local-name()='value'])")
    netmask0=$(xmllint $OVF_ENV_XML_FILE --xpath "string(//*/@*[local-name()='key' and .='netmask0']/../@*[local-name()='value'])")
    gateway=$(xmllint $OVF_ENV_XML_FILE --xpath "string(//*/@*[local-name()='key' and .='gateway0']/../@*[local-name()='value'])")
    dns=$(xmllint $OVF_ENV_XML_FILE --xpath "string(//*/@*[local-name()='key' and .='DNS']/../@*[local-name()='value'])")
    ntp_servers=$(xmllint $OVF_ENV_XML_FILE --xpath "string(//*/@*[local-name()='key' and .='ntp_servers']/../@*[local-name()='value'])")
    enable_ssh=$(xmllint $OVF_ENV_XML_FILE --xpath "string(//*/@*[local-name()='key' and .='enable_ssh']/../@*[local-name()='value'])")

    # users
    root_password=$(xmllint $OVF_ENV_XML_FILE --xpath "string(//*/@*[local-name()='key' and .='root_password']/../@*[local-name()='value'])")
    photon_password=$(xmllint $OVF_ENV_XML_FILE --xpath "string(//*/@*[local-name()='key' and .='photon_password']/../@*[local-name()='value'])")

    # lightwave config
    lw_domain=$(xmllint $OVF_ENV_XML_FILE --xpath "string(//*/@*[local-name()='key' and .='lw_domain']/../@*[local-name()='value'])") # some.domain.com
    lw_hostname=$(xmllint $OVF_ENV_XML_FILE --xpath "string(//*/@*[local-name()='key' and .='lw_hostname']/../@*[local-name()='value'])") # {{{LIGHTWAVE_HOSTNAME}}}
    lw_password=$(xmllint $OVF_ENV_XML_FILE --xpath "string(//*/@*[local-name()='key' and .='lw_password']/../@*[local-name()='value'])") # >7 chars, one number, one upper case char, one lowercase char, one special char
    lw_replication=$(xmllint $OVF_ENV_XML_FILE --xpath "string(//*/@*[local-name()='key' and .='lw_replication']/../@*[local-name()='value'])") # if deployment = partner
    lw_deployment="standalone"
    lw_is_first_instance="true"
    # we know this instance is a replcation partner if a replication host is specified
    if [ ! -z "$lw_replication" ]
    then
        lw_deployment="partner"
        lw_is_first_instance="false"
    fi
    if [ -z "$lw_password" ]
    then
        missing_values = "Missing lw_password"
    fi
    if [ -z "$lw_hostname" ]
    then
        missing_values = ${missing_values}", lw_hostname"
    fi
    if [ -z "$lw_domain" ]
    then
        missing_values = ${missing_values}", lw_domain"
    fi
    if [ ! -z "$missing_values" ]
    then
        echo $missing_values
        exit 1
    fi
}

#
# Main
#

trap cleanup EXIT

# Exit immediately if a command returns with a non-zero status
set +e

# Get env variables set in this OVF thru properties
ovf_env=$(vmtoolsd --cmd 'info-get guestinfo.ovfEnv')

if [ ! -z "${ovf_env}" ]
then
    # remove passwords from guestinfo.ovfEnv
    vmtoolsd --cmd "info-set guestinfo.ovfEnv `vmtoolsd --cmd 'info-get guestinfo.ovfEnv' | grep -v password`"

    # this file needs to be deleted since it contains passwords
    echo "$ovf_env" > $OVF_ENV_XML_FILE

    parse_ovf_env

    set_ntp_servers
    set_network_properties
    set_root_password
    set_photon_password
    set_enable_ssh

    configure_lightwave
fi

set -e
