#!/bin/bash -xe

if [[ $# -ne 2 ]]
then
  echo "setup-ssd-fs expects 2 arguments but $# provided"
  exit 1
fi

DEVICE=$1
MOUNT_POINT=$2
FS_TYPE=ext4

if [ -b ${DEVICE} ] && !( parted --script ${DEVICE}1 print )
then
  echo "Setting up SSD filesystem"
  mkfs -t $FS_TYPE ${DEVICE}
  mkdir -p ${MOUNT_POINT}
  mount ${DEVICE} ${MOUNT_POINT}
  echo "${DEVICE} ${MOUNT_POINT} ${FS_TYPE} defaults,barrier,noatime,noacl,data=ordered 1 1" >> /etc/fstab
  echo ""
else
  echo "Device ${DEVICE} not found or partition ${DEVICE}1 already exists."
  exit 1
fi
