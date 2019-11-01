#!/bin/bash
#set -x

LOG_DIR="/var/log/lightwave"
SBIN_DIR="/opt/vmware/sbin"

ShowUsage()
{
    echo "Usage: vmware-vmdnsd.sh { start | stop }"
}

WaitForServiceStart()
{
   local srvName=$1

   count=20

    while [ $count -ge 0 ]; do
        sleep 1
        netstat -nap | grep $srvName | grep 2015 > /dev/null 2>&1

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
    if [ -n "$(pgrep -x vmdnsd)" ]; then
        return 0;
    fi

    # Start service
    $SBIN_DIR/vmdnsd -L $LOG_DIR/vmdnsd.log > /dev/null 2>&1 &

    WaitForServiceStart vmdnsd

    if [ $? -ne 0 ]; then
        echo "Failed to start vmdns service. Error code : $rc"
    else
	echo "vmdns started successfully!"
    fi

    return $rc
}

WaitForServiceStop()
{
   local srvName=$1

    while [ 1 ]; do
       if [ -n "$(pgrep -x $srvName)" ]; then
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

    if [ -n "$(pgrep -x vmdnsd)" ]; then
        kill -INT `pidof vmdnsd` > /dev/null 2>&1

        rc=$?
        if [ $rc -ne 0 ]; then
            echo "Failed to stop vmdns service. Error code : $rc"
        else
            WaitForServiceStop vmdnsd
	    echo "vmdns stopped successfully!"
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

