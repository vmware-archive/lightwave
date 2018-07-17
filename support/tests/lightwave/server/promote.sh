#!/bin/sh

#prepare by installing rpms built in this build
rpm -Uvh --nodeps buildrpms/x86_64/lightwave-client*.rpm
rpm -Uvh --nodeps buildrpms/x86_64/lightwave-server*.rpm
rpm -Uvh --nodeps buildrpms/x86_64/lightwave-1*.rpm

/opt/likewise/sbin/lwsmd --start-as-daemon
/opt/likewise/bin/lwsm start vmafd
/opt/likewise/bin/lwsm start vmca
/opt/likewise/bin/lwsm start vmdir
/opt/likewise/bin/lwsm start vmdns
/opt/vmware/bin/ic-promote --domain $LIGHTWAVE_DOMAIN --password $LIGHTWAVE_PASS
/bin/bash
