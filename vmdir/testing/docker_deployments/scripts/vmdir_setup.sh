#!/bin/bash
set -x

mode=$1
partner='partner'
/opt/likewise/sbin/lwsmd --start-as-daemon
/opt/likewise/bin/lwsm start vmdns
sleep 1s
/opt/likewise/bin/lwsm start vmca
sleep 1s
/opt/likewise/bin/lwsm start vmdir
sleep 1s
if [ ${mode} == ${partner} ]; then
  /opt/vmware/bin/ic-join \
  --password $VMDIR_PWD \
  --domain vmdir.test \
  --domain-controller $VMDIR_FIRST_NODE_NAME
else
  /opt/vmware/bin/ic-promote \
  --password $VMDIR_PWD \
  --domain vmdir.test
fi
