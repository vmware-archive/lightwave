#!/bin/bash
#set -x

LOG_DIR="/var/log/lightwave"
SBIN_DIR="/opt/vmware/sbin"

ShowUsage()
{
    echo "Usage: vmware-vmcad.sh { start | stop }"
}

WaitForServiceStart()
{
   local srvName=$1

   count=20

    while [ $count -ge 0 ]; do
        sleep 1
        netstat -nap | grep $srvName | grep 2014 > /dev/null 2>&1

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
    if [ -n "$(pgrep -x vmcad)" ]; then
        return 0;
    fi

    # Start service
    su -c "$SBIN_DIR/vmcad > /dev/null 2>&1 &" -s /bin/sh lightwave

    WaitForServiceStart vmcad

    if [ $? -ne 0 ]; then
        echo "Failed to start vmca service. Error code : $rc"
    else
	echo "vmca started successfully!"
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

    if [ -n "$(pgrep -x vmcad)" ]; then
        kill -INT `pidof vmcad` > /dev/null 2>&1

        rc=$?
        if [ $rc -ne 0 ]; then
            echo "Failed to stop vmca service. Error code : $rc"
        else
            WaitForServiceStop vmcad
	    echo "vmca stopped successfully!"
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

