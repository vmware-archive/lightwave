#!/bin/sh
#
# Script to promote a Photon OS system to a Lightwave Domain Controller
#

#
# Configure these constants to match your environment
# ADMIN_PASSWORD must meet password complexity requirements of vmdir
#                Upper/Lower/Number/Legal punctuation/ 9 < pwd_len <= 20
LIGHTWAVE_AD=10.118.97.57
ADMIN_PASSWORD="VMware123@"

LIKEWISE_BASE=/home/abernstein/workspaces/git/lightwave/likewise-open
LIGHTWAVE_BASE=/home/abernstein/workspaces/git/lightwave/lightwave/

# Command line usage: ./lightwave-promote-dc.sh [[IP_PSC]  [Admin PWD]]
#
# Arguments are optional when LIGHTWAVE_AD / ADMIN_PASSWORD variables are set
#
#
# First argument must be IP address of PSC
if [ -n "$1" ]; then
  LIGHTWAVE_AD="$1"
  shift
fi

# Second argument must be Administrator password
if [ -n "$1" ]; then
  ADMIN_PASSWORD="$1"
  shift
fi

#
#
SCRIPT_DIR=`readlink -f "$0"`
TOOLS_DIR=`dirname $SCRIPT_DIR`
#
#
#
# 0 Sanity tests to validate this script can work at all
if [ -z "$LIGHTWAVE_AD" ]; then
  echo "ERROR: no DC IP address configured"
  exit 1
fi

if [ -z "$ADMIN_PASSWORD" ]; then
  echo "ERROR: Administrator account password not set"
  exit 1
fi

id=`ssh root@$LIGHTWAVE_AD hostname -i`
if [ $? -ne 0 ]; then
  echo "ERROR: 'ssh root@$LIGHTWAVE_AD' failed; fix IP or pub/priv keys"
  exit 1
fi

if [ $LIGHTWAVE_AD != $id ]; then
  echo "ERROR: LIGHTWAVE_AD IP address '$LIGHTWAVE_AD' does not match configuration '$id'"
  exit 1
fi

if [ ! -f $LIKEWISE_BASE/release/package/rpm/likewise-open/likewise-open-6.2.11-2.x86_64.rpm  ]; then
  echo "ERROR: file '$LIKEWISE_BASE/likewise-open-6.2.11-2.x86_64.rpm' does not exist"
  exit 1
fi

if [ `ls -1 $LIGHTWAVE_BASE/build/rpmbuild/RPMS/x86_64/*.rpm | wc -l` -eq 0 ]; then
  echo "ERROR '$LIGHTWAVE_BASE/build/rpmbuild/RPMS/x86_64/*.rpm' not found"
  exit 1
fi

# Save Administrator password to a file on AD system
echo -n "$ADMIN_PASSWORD" | ssh root@$LIGHTWAVE_AD 'cat > /tmp/promote-pwd.txt'
if [ $? -ne 0 ]; then
  echo "ERROR: Failed setting promote-pwd.txt file on '$LIGHTWAVE_AD' system"
  exit 1
fi

# 1 Copy custom build of Likewise-Open
scp $LIKEWISE_BASE/release/package/rpm/likewise-open/likewise-open-6.2.11-2.x86_64.rpm $LIGHTWAVE_AD:/tmp

# 2 Copy Lightwave --enable-winjoin build
scp $LIGHTWAVE_BASE/build/rpmbuild/RPMS/x86_64/*.rpm $LIGHTWAVE_AD:/tmp

# 3 Install Likewise-Open
ssh root@$LIGHTWAVE_AD rpm -ivh /tmp/likewise-open-6.2.11-2.x86_64.rpm

# 4 Install Lightwave RPMs
ssh root@$LIGHTWAVE_AD rpm -ivh /tmp/lightwave-1*.rpm /tmp/lightwave-client-1*.rpm

# 5 Modify /etc/resolv.conf to point to self for DNS
scp $TOOLS_DIR/resolv-config.sh  $LIGHTWAVE_AD:/tmp
ssh root@$LIGHTWAVE_AD  /tmp/resolv-config.sh


# 6 Promote Lightwave PSC
ssh root@$LIGHTWAVE_AD '/opt/vmware/bin/configure-lightwave-server \
  --domain lightwave.local --password `cat /tmp/promote-pwd.txt`'

# 7 Start Likewise SMB services
ssh root@$LIGHTWAVE_AD '( /opt/likewise/bin/lwsm start npfs && /opt/likewise/bin/lwsm start pvfs && 
    /opt/likewise/bin/lwsm start rdr && /opt/likewise/bin/lwsm start srv )'

# 8 Modify IP tables entries
ssh root@$LIGHTWAVE_AD '( iptables -I INPUT --proto icmp -j ACCEPT &&
    iptables -I INPUT --proto udp --dport 389 -j ACCEPT &&
    iptables -I INPUT --proto tcp --dport 445 -j ACCEPT &&
    iptables -I INPUT --proto tcp --dport 139 -j ACCEPT &&
    iptables -I INPUT --proto tcp --dport 389 -j ACCEPT &&
    iptables -I OUTPUT --proto tcp --dport 389 -j ACCEPT )'

# 9 Add DNS forwarder
ssh root@$LIGHTWAVE_AD '/opt/vmware/bin/vmdns-cli add-forwarder 10.155.23.1 \
    --server localhost --username Administrator --domain lightwave.local --password `cat /tmp/promote-pwd.txt`'

# 10 Additional DNS SRV records:
ssh root@$LIGHTWAVE_AD \
  '/opt/vmware/bin/vmdns-cli add-record --zone lightwave.local \
   --type SRV \
   --service kerberos-master \
   --protocol tcp \
   --target photon-102-test \
   --priority 1 \
   --weight 1 \
   --port 88 \
   --server localhost \
   --password `cat /tmp/promote-pwd.txt`'

ssh root@$LIGHTWAVE_AD \
  '/opt/vmware/bin/vmdns-cli add-record --zone lightwave.local \
   --type SRV \
   --service kerberos-master \
   --protocol udp \
   --target photon-102-test \
   --priority 1 \
   --weight 1 \
   --port 88 \
   --server localhost \
   --password `cat /tmp/promote-pwd.txt`'

# 11  Add additional cifs entries to krb5.keytab
scp $TOOLS_DIR/add-keytab.sh $LIGHTWAVE_AD:/tmp
ssh root@$LIGHTWAVE_AD \
  /tmp/add-keytab.sh

# 12 Add gss-spnego plugin
echo "Installing CyrusSASL gssspnego plugin"
scp /home/abernstein/workspaces/wrk/cyrus-sasl-2.1.26/plugins/.libs/libgssspnego.so.3.0.0 abernstein@$LIGHTWAVE_AD:/var/tmp
ssh root@$LIGHTWAVE_AD \
  '( mv /var/tmp/libgssspnego.so.3.0.0 /usr/lib/sasl2 && ln -s /usr/lib/sasl2/libgssspnego.so.3.0.0 /usr/lib/sasl2/libgssspnego.so )'

# 13 Replace default GSSAPI plugin (built with GSS-SPNEGO disabled)
echo "Configuring patched libgssapiv2.so plugin that disables gss-spnego"
scp /home/abernstein/workspaces/wrk/cyrus-sasl-2.1.26/plugins/.libs/libgssapiv2.so.3.0.0 abernstein@$LIGHTWAVE_AD:/var/tmp
ssh root@$LIGHTWAVE_AD \
 '( mv -i /usr/lib/sasl2/libgssapiv2.so.3.0.0   /usr/lib/sasl2/libgssapiv2.so.3.0.0.rpmorig &&
    mv /var/tmp/libgssapiv2.so.3.0.0   /usr/lib/sasl2/libgssapiv2.so.3.0.0 )'

# 14 Replace the default CyrusSASL library
scp /home/abernstein/workspaces/wrk/cyrus-sasl-2.1.26/./lib/.libs/libsasl2.so.3.0.0 abernstein@$LIGHTWAVE_AD:/var/tmp
ssh root@$LIGHTWAVE_AD \
    '( mv /usr/lib/libsasl2.so.3.0.0  /usr/lib/libsasl2.so.3.0.0.rpmorig &&
       mv /var/tmp/libsasl2.so.3.0.0 /usr/lib/libsasl2.so.3.0.0 )'

# 15 Modified Likewise components:
# This should be a no-op, as this script installs built likewise-open RPM,
# which contains these changes.
#
#/opt/likewise/lib64/liblwbase.so.0.0.0.rpmorig
#/opt/likewise/lib64/liblsa_srv.so.rpmorig
#/opt/likewise/lib64/libsamr_srv.so.rpmorig

# 15 Add partitions-containers to vmdird dseRoot
echo "Update CN=photon-102-test,CN=Partitions,cn=configuration,dc=lightwave,dc=local"
scp $TOOLS_DIR/partitions-vmdir.sh abernstein@$LIGHTWAVE_AD:/var/tmp
ssh root@$LIGHTWAVE_AD \
    /var/tmp/partitions-vmdir.sh 

# 16 Restart all lightwave services
ssh root@$LIGHTWAVE_AD \
    /opt/likewise/bin/lwsm restart lwreg

# 17 Ready for domainjoin-cli/Windows to join this system
echo "Lightwave DC is now configured: IP: $LIGHTWAVE_AD"
