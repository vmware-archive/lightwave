#!/bin/bash
source /opt/vmware/bin/aws_backup_common.sh

BACKUP_PATH="/var/lib/vmware/db_bkp/vmdir"

#Uploads the mdb file. Sample name: <instance_id>-data.mdb
upload_bundle() {
    #Calculate checksum
    md5=`openssl md5 -binary $BACKUP_PATH/data.mdb | base64`
    if [ $? -ne 0 ]; then
        exit
    fi
    #Upload bundle to cloud storage
    retry=1
    while [ $retry -le 5 ]
    do
	    logger -t backup "Uploading $LW_BKP_PATH/$INSTANCE_ID-$time-data.mdb"
        start_time="$(date -u +%s)"
		upload_to_cloud "$LW_BKP_PATH/$INSTANCE_ID-$time-data.mdb" "$BACKUP_PATH/data.mdb" "$md5"
        if [ $? -ne 0 ]; then
            logger -t backup "Error while uploading bundle. Retrying."
            (( retry = $retry + 1 ))
            continue
        fi
        end_time="$(date -u +%s)"
        upload_time=$(( end_time - start_time ))
        logger -t backup "DB Upload took "$upload_time" seconds."
        return 0
    done
    return 1
}

#uploads the rpm archive. Sample name: <instance_id>-lw_rpm_archive.zip
write_version() {
    pushd /var/vmware/lightwave; zip -r /var/log/lightwave/lw_rpm_archive.zip *; popd
    if [ $? -ne 0 ]; then
        logger -t backup "Error while zipping archive."
        exit
    fi
    md5=`openssl md5 -binary /var/log/lightwave/lw_rpm_archive.zip | base64`
    if [ $? -ne 0 ]; then
        exit
    fi

    retry=1
    while [ $retry -le 5 ]
    do
        start_time="$(date -u +%s)"
        upload_to_cloud "$LW_BKP_PATH/$INSTANCE_ID-$time-lw_rpm_archive.zip" "/var/log/lightwave/lw_rpm_archive.zip" "$md5"
        if [ $? -ne 0 ]; then
            logger -t backup "Error while uploading bundle. Retrying."
            (( retry = $retry + 1 ))
            continue
        fi
        end_time="$(date -u +%s)"
        upload_time=$(( end_time - start_time ))
        logger -t backup "DB Upload took "$upload_time" seconds."
        rm -rf /var/log/lightwave/lw_rpm_archive.zip
        return 0
    done
    rm -rf /var/log/lightwave/lw_rpm_archive.zip
    return 1
}

logger -t backup "Taking backup"
time=`date '+%Y%m%d%H%M'`

#Hot copy
if [ -d $BACKUP_PATH ]; then
    rm -rf $BACKUP_PATH
fi

/opt/vmware/bin/dir-cli database-backup --backuppath $BACKUP_PATH
if [ $? -ne 0 ]; then
    logger -t backup "Error while taking db backup."
    exit
fi

#WAL flush
/opt/vmware/bin/lw_mdb_walflush $BACKUP_PATH
if [ $? -ne 0 ]; then
    logger -t backup "Error while flishing WAL."
    exit
fi

upload_bundle
if [ $? -ne 0 ]; then
    logger -t backup "Upload of mdb file backup failed. Exiting."
    rm -rf $BACKUP_PATH
    exit 1
fi
rm -rf $BACKUP_PATH

write_version
if [ $? -ne 0 ]; then
    logger -t backup "Failed to take backup of rpm archive. Exiting."
    exit 1
fi

#Delete backups older than 5 days
get_bkp_list
C_DAY=${time:6:2}
C_DAY=`echo $C_DAY | sed 's/^0//'`
C_MONTH=${time:4:2}
i=0
for line in $LIST; do
    (( i = i + 1 ))
    (( mod = $i % 4 ))
    if [ $mod -ne 0 ]; then
        continue
    fi
    MTIME=${line: -31}
    M_DAY=${MTIME:6:2}
    M_DAY=`echo $M_DAY | sed 's/^0//'`
    M_MONTH=${MTIME:4:2}

    if [ "$C_MONTH" = "$M_MONTH" ]; then
        (( ELAPSED_DAYS = $C_DAY - $M_DAY ))
    else
        if [[ "$M_MONTH" = "01" || "$M_MONTH" = "03" || "$M_MONTH" = "05" || "$M_MONTH" = "07" || "$M_MONTH" = "08" || "$M_MONTH" = "10" || "$M_MONTH" = "12" ]]
        then
            DAYS_IN_MONTH=31
        elif [ "$M_MONTH" = "02" ]
        then
            DAYS_IN_MONTH=28
        else
            DAYS_IN_MONTH=30
        fi
        (( ELAPSED_DAYS = $C_DAY + $DAYS_IN_MONTH - $M_DAY ))
    fi

    if [ $ELAPSED_DAYS -gt 5 ]; then
        logger -t backup "DELETING "$BUCKET/$LW_BKP_PATH/$line
        mdb_file=${line::-18}data.mdb
        delete_file $line
        delete_file $mdb_file
    fi
done
