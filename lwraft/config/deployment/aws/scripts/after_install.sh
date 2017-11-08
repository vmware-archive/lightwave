#!/bin/bash -xe
source $(dirname $(realpath $0))/common.sh

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

echo "Step 5: Install filebeat, logrotate and configure journalctl"

# Change journalctl config to forward logs to syslog
cat > /etc/systemd/journald.conf <<EOF
[Journal]
ForwardToSyslog=yes
EOF

# Install or upgrade syslog
tdnf install -y syslog-ng

# Remove the existing config of syslog-ng for lightwave
# Everything will be written to /var/log/messages
sed -i '/lightwave.conf.d/d' /etc/syslog-ng/syslog-ng.conf

systemctl restart syslog-ng
systemctl restart systemd-journald

# Install logrotate and configure for syslog
tdnf install -y logrotate
cp /opt/vmware/share/config/lightwave-syslog-logrotate.conf /etc/logrotate.d/
chmod 444 /etc/logrotate.d/lightwave-syslog-logrotate.conf

# Install and configure filebeat
if ! rpm -q filebeat; then
    curl -L -O https://artifacts.elastic.co/downloads/beats/filebeat/filebeat-5.6.3-x86_64.rpm
    rpm -i filebeat-5.6.3-x86_64.rpm
    rm filebeat-5.6.3-x86_64.rpm
fi

# configuration steps referred from cascade controller codebase
LOGSTASH_ELB=$(get_tag_value LOGSTASH_ELB)
cat > /etc/filebeat/filebeat.yml <<EOF
filebeat.prospectors:
- input_type: log
  paths:
    - /var/log/messages
output.logstash:
  hosts: ["$LOGSTASH_ELB:5043"]
EOF

systemctl start filebeat
