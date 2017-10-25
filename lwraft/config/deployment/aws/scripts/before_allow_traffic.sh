#!/bin/bash -xe

CRONDIR=$(dirname $(dirname $(realpath $0)))/crontab

echo "Step 1: Install cronjob to run the clean-up script"

if [[ -z `pgrep crond` ]]
then
    echo "crond is not running, starting crond"
    service crond start
fi

crontab ${CRONDIR}/post-cron.txt
crontab -l