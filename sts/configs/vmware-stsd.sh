#!/bin/bash
#set -x

LOG_DIR="/var/log/lightwave"
SBIN_DIR="/opt/vmware/sbin"
DAEMON=stssrv

ShowUsage()
{
    echo "Usage: vmware-stsd.sh { start | stop }"
}

WaitForServiceStart()
{
   count=20

    while [ $count -ge 0 ]; do
        sleep 1
        netstat -nap | grep ":443 " > /dev/null 2>&1

        if [ $? -eq 0 ]; then 
          return 0
        fi

	let count=$count-1
    done

    return 1
}

StartService()
{
    # check if service is already up
    if [ -n "$(pgrep -x $DAEMON)" ]; then
        return 0;
    fi

    # Start service
    # TODO, log rotation and aging?
    su -c "$SBIN_DIR/$DAEMON -config=/etc/vmware/$DAEMON.conf >> /var/log/lightwave/$DAEMON.log 2>&1 &" -s /bin/sh lightwave

    WaitForServiceStart

    if [ $? -ne 0 ]; then
        echo "Failed to start $DAEMON service. Error code : $rc"
    else
	echo "$DAEMON started successfully!"
    fi

    return $rc
}

WaitForServiceStop()
{
    while [ 1 ]; do
       if [ -n "$(pgrep -x $DAEMON)" ]; then
         sleep 1
       else
         return 0
       fi
    done

    return 1
}

StopService()
{
    rc=0

    if [ -n "$(pgrep -x $DAEMON)" ]; then
        kill -INT `pidof $DAEMON` > /dev/null 2>&1

        rc=$?
        if [ $rc -ne 0 ]; then
            echo "Failed to stop $DAEMON service. Error code : $rc"
        else
            WaitForServiceStop vmdird
	    echo "$DAEMON stopped successfully!"
        fi
    fi

    return $rc
}

## main

if [ $# -lt 1 ]; then
    ShowUsage
    exit 1
fi

rc=0

case "$1" in
    start)
        StartService
        rc=$?
        ;;
    stop)
        StopService
        rc=$?
        ;;
    *)
        echo "Error: Unsupported action : $1"
        ShowUsage
        rc=1
        ;;
esac

exit $rc
