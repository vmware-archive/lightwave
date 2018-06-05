#!/bin/bash -e
source /var/vmware/lightwave/scripts/common.sh
source /opt/vmware/bin/post_aws_restore_common.sh

BACKUP_PATH=$1

get_params

if [ ! -d /var/lib/vmware/post/partner ]; then
    mkdir /var/lib/vmware/post/partner
fi

START_TIME="$(date -u +%s)"
download_db $BACKUP_PATH /var/lib/vmware/post/partner/data.mdb
END_TIME="$(date -u +%s)"
ELAPSED=$(( END_TIME - START_TIME ))
echo "Copied DB from bucket in $ELAPSED seconds"

/opt/vmware/bin/post-cli disaster-recovery --login $ADMIN_USER --password $ADMIN_PASS

/opt/likewise/bin/lwsm restart post

echo "Finished restore procedure."
