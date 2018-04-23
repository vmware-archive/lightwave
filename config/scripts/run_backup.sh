#!/bin/bash
source /opt/vmware/bin/aws_backup_common.sh

SLEEP_QUANTUM=900

logger -t run_backup "Started run_backup.sh"

check_or_create_bucket

#Find this node's position in sorted order
get_node_list

RANK=0

for ID in ${ID_LIST[@]}; do
    if [ "$ID" \< "$INSTANCE_ID" ]; then
        ((RANK++))
    fi
done

if [ ! $RANK -eq 0 ]; then
    (( SLEEP_TIME=$SLEEP_QUANTUM * $RANK ))
    logger -t run_backup "Node is in position "$RANK" in sorted order. Sleeping."
    sleep $SLEEP_TIME

    #Check if any other node has done the backup.
    get_latest_bkp

    MTIME=${LATEST: -24}
    CURRENT_TIME=`date '+%Y%m%d%H%M'`
    C_HOUR=${CURRENT_TIME:8:2}
    C_HOUR=`echo $C_HOUR | sed 's/^0//'`
    C_MIN=${CURRENT_TIME:10:2}
    C_MIN=`echo $C_MIN | sed 's/^0//'`
    M_HOUR=${MTIME:8:2}
    M_HOUR=`echo $M_HOUR | sed 's/^0//'`
    M_MIN=${MTIME:10:2}
    M_MIN=`echo $M_MIN | sed 's/^0//'`
    C_DAY=${CURRENT_TIME:6:2}
    M_DAY=${MTIME:6:2}

    (( C_ELAPSED=$C_HOUR*60 + $C_MIN ))
    (( M_ELAPSED=$M_HOUR*60 + $M_MIN ))

    if [ $C_DAY -eq $M_DAY ]; then
        (( TIME_DIFF=C_ELAPSED-M_ELAPSED ))
    else
        (( TIME_DIFF=24*60-C_ELAPSED+M_ELAPSED ))
    fi

    if (( TIME_DIFF < SLEEP_TIME+15 )); then
        logger -t run_backup "Another node took the backup "$TIME_DIFF" mins ago. Backup not required."
        exit
    fi
fi

/opt/vmware/bin/lw_backup.sh
