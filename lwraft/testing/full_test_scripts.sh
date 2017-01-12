#!/bin/bash

mode="package"
host="localhost"
port=389

while getopts m:h:p: opt;
do
    case $opt in 
        m)
            mode=$OPTARG
            ;;
        h)
            host=$OPTARG
            ;;
        p)
            port=$OPTARG
            ;;
    esac
done

echo "Mode is $mode"
echo "Port is $port"

if [ $mode = "package" ]; then
   VDCPROMO_PATH=/usr/lib/vmware-vmdir/bin
else
   export VDCPROMO_PATH=../build/tools/vdcpromo
fi

REL_DIR=`dirname $0`
cd $REL_DIR
TESTING_PATH=`pwd`

VMDIR_DATA_PATH=/storage/db/vmware-vmdir

# Set up vdc instance
if [ $mode = "package" ]; then
   rm $VMDIR_DATA_PATH/*
   /opt/likewise/bin/lwsm restart vmdir
fi

$VDCPROMO_PATH/vdcpromo -d vmware.com -u administrator -w 123

echo "Checking existence of data directory......."
echo $TESTING_PATH/data

$TESTING_PATH/test_scripts.sh -h $host -p $port

# Clean up
if [ $mode = "package" ]; then
   rm $VMDIR_DATA_PATH/*
   /opt/likewise/bin/lwsm restart vmdir
fi
