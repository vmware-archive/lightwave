#!/bin/bash -xe

echo "Step 1: Create symlinks in /tmp/vmware/lightwave to rpm files"

rm -rf /tmp/vmware/lightwave
mkdir -p /tmp/vmware/lightwave/x86_64

# sort by time for re-deployment cases
ls -rt `find /opt/codedeploy-agent/deployment-root/${DEPLOYMENT_GROUP_ID}/ -name "*.rpm"` | while read org
do
    f=`awk -F'/' '{ print $NF; }' <<< ${org}`
    tgt="/tmp/vmware/lightwave/x86_64/${f}"
    ln -nfs ${org} ${tgt}
done


echo "Step 2: Create lightwave yum repo in /tmp/vmware/lightwave"

sed -i -e "s|https://vmware.bintray.com/lightwave-dev/photon/master|file:///tmp/vmware/lightwave|" -e "s|gpgcheck=1|gpgcheck=0|" /etc/yum.repos.d/lightwave.repo
createrepo "/tmp/vmware/lightwave"


echo "Step 3: Upgrade/install lightwave-post and lightwave-client"

tdnf makecache
tdnf install -y lightwave-post lightwave-client


# TODO - this should not be necessary when DNS is stabilized
echo "Step 4: Set proxy curl timeout"

/opt/likewise/bin/lwregshell add_value '[HKEY_THIS_MACHINE\Services\post\Parameters]' CurlTimeoutSec REG_DWORD 10 || echo "CurTimeoutSec is already set"
