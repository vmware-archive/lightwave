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

echo "Step 4: Set the default vmdir lsass provider bind protocol to SRP and disable port 38900 simple bind"

/opt/likewise/bin/lwregshell set_value '[HKEY_THIS_MACHINE\Services\lsass\Parameters\Providers\VmDir]' BindProtocol srp
/opt/likewise/bin/lwsm restart lsass

/opt/likewise/bin/lwregshell set_value '[HKEY_THIS_MACHINE\services\post\parameters]' AllowInsecureAuthentication 0

# TODO - this should not be necessary when DNS is stabilized
echo "Step 5: Set proxy curl timeout"

/opt/likewise/bin/lwregshell add_value '[HKEY_THIS_MACHINE\Services\post\Parameters]' CurlTimeoutSec REG_DWORD 10 || echo "CurTimeoutSec is already set"

echo "Step 6: Install filebeat, logrotate and configure journalctl"

# Change journalctl config to forward logs to syslog
cat > /etc/systemd/journald.conf <<EOF
[Journal]
ForwardToSyslog=yes
RateLimitBurst=0
RateLimitIntervalSec=0
EOF

# Install or upgrade syslog
tdnf install -y syslog-ng

# Remove the existing config of syslog-ng for lightwave
# Everything will be written to /var/log/messages
sed -i '/lightwave.conf.d/d' /etc/syslog-ng/syslog-ng.conf
if [[ -z $(grep "frac_digits(3)" /etc/syslog-ng/syslog-ng.conf) ]]
then
    echo "" >> /etc/syslog-ng/syslog-ng.conf
    echo "options { frac_digits(3); };" >> /etc/syslog-ng/syslog-ng.conf
    echo "" >> /etc/syslog-ng/syslog-ng.conf
fi

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
get_tag_value "LOGSTASH_ELB" LOGSTASH_ELB
echo "LOGSTASH_ELB=${LOGSTASH_ELB}"

cat > /etc/filebeat/filebeat.yml <<EOF
filebeat.prospectors:
- input_type: log
  paths:
    - /var/log/messages
output.logstash:
  hosts: ["$LOGSTASH_ELB:5043"]
EOF

systemctl start filebeat

# install telegraph if the WAVEFRONT_PROXY tag is set
get_tag_value "WAVEFRONT_PROXY" WAVEFRONT_PROXY

if [[ -n "$WAVEFRONT_PROXY" ]]; then
    echo "Step 6: Install and confiure telegraf for wavefront"
    cat << EOF >/etc/yum.repos.d/wavefront_telegraf.repo
[wavefront_telegraf]
name=wavefront_telegraf
baseurl=https://packagecloud.io/wavefront/telegraf/el/7/\$basearch
repo_gpgcheck=0
gpgcheck=0
enabled=1
gpgkey=https://packagecloud.io/wavefront/telegraf/gpgkey
sslverify=1
sslcacert=/etc/pki/tls/certs/ca-bundle.crt
metadata_expire=300
EOF
  tdnf makecache
  # (PR 2021327) install 1.4.0 until we resolve issue with 1.5.0 installation
  tdnf install -y telegraf-1.4.0~34b7a4c

  sed '/User=telegraf/s/^/#/g' /usr/lib/telegraf/scripts/telegraf.service >/usr/lib/systemd/system/telegraf.service
  sed "s/@@WAVEFRONT_PROXY@@/${WAVEFRONT_PROXY}/" /opt/vmware/share/config/telegraf.conf >/etc/telegraf/telegraf.conf

  find /opt/vmware -name "*-telegraf.conf" | xargs cp -t /etc/telegraf/telegraf.d

  systemctl daemon-reload
  systemctl restart telegraf
fi
