#!/bin/sh

# Directories
LW_DIR="/opt/likewise"
LW_BIN_DIR="$LW_DIR/bin"
LW_SBIN_DIR="$LW_DIR/sbin"
VM_DIR="/opt/vmware"
VM_BIN_DIR="$VM_DIR/bin"
VM_SBIN_DIR="$VM_DIR/sbin"
VM_CONFIG_DIR="$VM_DIR/share/config"
VM_LOG_DIR="/var/log/lightwave"

# registry keys
VMAFD_PARAM_KEY="[HKEY_THIS_MACHINE\\Services\\vmafd\\Parameters]"
VMDIR_KEY="[HKEY_THIS_MACHINE\\Services\\vmdir]"
VMDIR_UPGRADE_KEY="[HKEY_THIS_MACHINE\\Services\\vmdir-upgrade]"
ADMIN="Administrator"
LIST_VALUES="list_values"
USAGE="vmdir_upgrade.sh [--password <password>] [--domainname <domain-name>]"

exit_upgrade(){
    if [ $RUN_LWSM ]; then
        echo "Stopping Likewise services"
        $LW_BIN_DIR/lwsm shutdown
    fi

    # Successful Upgrade
    if [ $1 -eq 0 ]; then
        echo "Directory upgrade success."
    else
        echo "Directory upgrade failure."
    fi

    exit $1
}

if [ $# -gt 0 ]; then

    STATE="options"

    for arg in "$@"
    do
        case "$STATE" in
            "options")
                case "$arg" in
                    "--password")
                        STATE="password"
                        ;;
                    "--domainname")
                        STATE="domainname"
                        ;;
                    *)
                        echo "Invalid parameter: $arg"
                        echo $USAGE
                        exit_upgrade 1
                        ;;
                esac
                ;;
            "password")
                PASSWORD=$arg
                STATE="options"
                ;;
            "domainname")
                DOMAIN_NAME=$arg
                STATE="options"
                ;;
        esac
    done
fi

# Start Likewise services
if [ -z "`pidof lwsmd`" ]; then
    echo "Starting Likewise services"
    $LW_SBIN_DIR/lwsmd --start-as-daemon --syslog
    RUN_LWSM=1
else
    echo "Likewise services already running"
fi

$LW_BIN_DIR/lwsm stop vmdir
echo "Running schema patch"
$VM_SBIN_DIR/vmdird -u -c -s -f $VM_CONFIG_DIR/vmdirschema.ldif
echo "Schema patch finished"

# Begin vdcupgrade
if [ -z "$DOMAIN_NAME" ]; then
    # get domain name from registry

    DOMAIN_NAME=`$LW_BIN_DIR/lwregshell "$LIST_VALUES" "$VMAFD_PARAM_KEY" | \
    grep 'DomainName' | sed 's/+//' | cut -d '"' -f4`
    if [ -z "$DOMAIN_NAME" ]; then
        echo "Invalid Domain Name"
        exit_upgrade 1
    fi
fi

# get DCAccountDN from registry
DCACCOUNTDN=`$LW_BIN_DIR/lwregshell $LIST_VALUES "$VMDIR_KEY" | \
    grep 'dcAccountDN' | sed 's/+//' | cut -d '"' -f4`

if [ -z "$DCACCOUNTDN" ]; then
    echo "Invalid DCAccountDN"
    exit_upgrade 1
fi

ADMIN_NAME="$ADMIN@$DOMAIN_NAME"

if [ -z "$PASSWORD" ]; then
    if ! exec </dev/tty; then
        echo "To finish upgrading VMware Directory Service, run the following command to configure the server:"
        echo $VM_BIN_DIR/vdcupgrade -H localhost -D "$ADMIN_NAME" -d "$DCACCOUNTDN"
    else

        echo "Enter password for $ADMIN_NAME >>> "
        read -s PASSWORD
        echo
    fi
fi

# Restart vmdir
echo "Starting vmdir"
$LW_BIN_DIR/lwsm start vmdir
echo "Running vdcupgrade"
echo "$PASSWORD" | $VM_BIN_DIR/vdcupgrade -H localhost -D "$ADMIN_NAME" -d "$DCACCOUNTDN" \
                                          >$VM_LOG_DIR/vdcupgrade.log 2>&1
if [ $? -ne 0 ]; then
    echo "vdcupgrade failed. Resolve issues in $VM_LOG_DIR/vdcupgrade.log before retrying."
    exit_upgrade 1
fi

exit_upgrade 0
