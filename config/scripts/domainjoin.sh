#!/bin/bash

function bail_on_error
{
    error_code=$1; shift
    error_msg=$1; shift

    if [ $error_code -ne 0 ]; then
        echo "Error: $error_msg. Code: $error_code"
        exit $error_code
    fi
}

#
# main
#

echo "Joining to VMware directory..."

/opt/vmware/bin/ic-join "$@"
bail_on_error $? "Failed to join VMware directory"

echo "Configuring VMware directory provider..."

/opt/likewise/bin/lwregshell set_value '[HKEY_THIS_MACHINE\Services\lsass\Parameters\Providers]' "LoadOrder" "ActiveDirectory" "VmDir" "Local"
bail_on_error $? "Failed to configure VMware directory provider"

/opt/likewise/bin/lwsm restart lsass
bail_on_error $? "Failed to restart Likewise Authentication Service"

echo "Configuring NSSWITCH..."

/opt/likewise/bin/domainjoin-cli configure --enable nsswitch
bail_on_error $? "Failed to configure NSSWITCH"

echo "Configuring PAM..."

/opt/likewise/bin/domainjoin-cli configure --enable pam
bail_on_error $? "Failed to configure PAM"

echo "Successfully joined VMware directory"
