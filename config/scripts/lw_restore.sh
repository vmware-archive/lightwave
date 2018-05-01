#!/bin/bash -e
source /var/vmware/lightwave/scripts/common.sh
source /var/vmware/lightwave/scripts/generate_ssl_cert.sh
source /opt/vmware/bin/aws_restore_common.sh

fix_ssl_cert() {
    echo "Fixing hostname"
    get_san
    /opt/likewise/bin/lwregshell set_value '[HKEY_THIS_MACHINE\Software\VMware\Identity\Configuration]' "Hostname" $NAME
    systemctl restart vmware-stsd

    echo "Fixing SSL"
    /opt/vmware/bin/certool --enableserveroption --option=multiplesan

    generate_ssl_cert $ASG_NAME $EC2_REGION

    systemctl restart lightwave-proxy
}

BACKUP_PATH=$1

get_params

echo "Performing Lightwave after install actions"

# Promote
echo "Running promote command"
/root/first-node/first-dc-init.sh --domain=$LW_DOMAIN --site=AWS-$EC2_REGION --admin-pass=$DOMAIN_PROMOTER_PASS --promoter-pass=$DOMAIN_PROMOTER_PASS --joiner-pass=$JOINER_PASS --srvmgr-pass=$SRV_ACCT_MGR_PASS --schemamgr-pass=$SCHEMA_MGR_PASS --cascade-ou=$CASCADE_OU

#Run fixup
fix_ssl_cert

if [ ! -d /var/lib/vmware/vmdir/partner ]; then
    mkdir /var/lib/vmware/vmdir/partner
fi

download_db $BACKUP_PATH /var/lib/vmware/vmdir/partner/data.mdb

/opt/vmware/bin/dir-cli disaster-recovery --login $PROMOTER_USER --password $DOMAIN_PROMOTER_PASS

/opt/likewise/bin/lwsm restart vmafd
/opt/likewise/bin/lwsm restart vmdir

/opt/vmware/bin/vdcpromo -u $PROMOTER_USER -d $LW_DOMAIN -w $DOMAIN_PROMOTER_PASS -n

/opt/likewise/bin/lwsm restart vmdns

systemctl restart vmware-stsd

/root/first-node/create-lwp-srv-account.sh 127.0.0.1 $LW_DOMAIN $SRV_ACCT_MGR_USR $SRV_ACCT_MGR_PASS

systemctl restart lightwave-proxy

echo "Finished restore procedure."
