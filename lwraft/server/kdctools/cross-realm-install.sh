#!/bin/sh
#
# Script to install Lightwave components listed in "files". Afterwards
# promote the installed instance, and then establish a Kerberos
# cross-realm trust relationship with the system named on the command line.
#
# This assumes /etc/hosts on the local and trusted system are properly
# configured with correct FQDN (and consistent) values for the
# system's IP addresses.

# This cannot be discovered
DNS_NAME_SERVER="10.155.23.1"

files="
likewise-open-6.2.0-0.x86_64.rpm
likewise-open-devel-6.2.0-0.x86_64.rpm
vmware-directory-client-6.0.0-0.x86_64.rpm
vmware-directory-6.0.0-0.src.rpm
vmware-directory-6.0.0-0.x86_64.rpm
vmware-directory-client-devel-6.0.0-0.x86_64.rpm
vmware-afd-client-6.0.0-0.x86_64.rpm
vmware-afd-6.0.0-0.src.rpm
vmware-afd-6.0.0-0.x86_64.rpm
vmware-afd-client-devel-6.0.0-0.x86_64.rpm
vmware-ca-client-6.0.0-0.x86_64.rpm
vmware-ca-6.0.0-0.src.rpm
vmware-ca-6.0.0-0.x86_64.rpm
vmware-ca-client-devel-6.0.0-0.x86_64.rpm
vmware-ic-config-1.0.0-0.src.rpm
vmware-ic-config-1.0.0-0.x86_64.rpm
"

if [ `id -u` -ne 0 ]; then
  echo "ERROR: only root can run $0"
  exit 1
fi

# This must be manually configured (or get from command line)
TRUSTED_DOMAIN=""

if [ -n "$1" ]; then
  TRUSTED_FQDN="$1"

  # Prove provided TRUSTED_FQDN is a known hostname in hosts or DNS
  TEST_TRUSTED_FQDN=`grep "$TRUSTED_FQDN" /etc/hosts | \
    sed -e '/^#.*/d' \
        -e '/^127.0.0.1.*/d' | awk '{print $2}'`
  if [ -z "$TEST_TRUSTED_FQDN" ]; then
    TEST_TRUSTED_FQDN=`nslookup "$TRUSTED_FQDN" "$DNS_NAME_SERVER" | grep ^Name: | sed 's/Name:\t*//'`
  fi
  if [ -z "$TEST_TRUSTED_FQDN" ]; then
    echo "ERROR: Specified trusted FQDN cannot be resolved"
    exit 1
  fi

  TRUSTED_DOMAIN=`echo "$TRUSTED_FQDN" | sed 's/[^.][^.]*\.\(.*\)/\1/' | tr 'a-z' 'A-Z'`
  if [ -z "$TRUSTED_FQDN" -o "$TRUSTED_FQDN" = "$TRUSTED_DOMAIN" ]; then
    echo "ERROR: cannot determine trusted FQDN"
    exit 1
  fi
  shift
else
  echo "ERROR: No trusted domain (cross-realm peer) specified!"
  exit 1
fi

if [ -n "$1" ]; then
  MY_FQDN="$1"
  shift
fi

#Compute FQDN of this system
hostname=`hostname`
FQDN=`grep $hostname /etc/hosts | \
  sed -e '/^#.*/d' \
      -e '/^127.0.0.1.*/d' | awk '{print $2}'`

# Try DNS lookup
if [ -z "$FQDN" -a -z "$MY_FQDN" ]; then
  FQDN=`nslookup $hostname "$DNS_NAME_SERVER" | grep ^Name: | sed 's/Name:\t*//'`
fi

if [ -z "$FQDN" -a -n "$MY_FQDN" ]; then
  FQDN=`grep "$MY_FQDN" /etc/hosts | \
    awk '{print $2}'`
fi

# Give up here if FQDN is still unknown
if [ -z "$FQDN"  -o "$FQDN" = "" ]; then
  echo "ERROR: Cannot determine domain of this system"
  exit 1
fi

DOMAIN=`echo $FQDN |
          sed -e "s/[^.][^.]*\.\(.*\)/\1/" | \
          tr 'a-z' 'A-Z'`
DOMAIN_LC=`echo "$DOMAIN" | tr 'A-Z' 'a-z'`

echo "DOMAIN='$DOMAIN'"
echo "TRUSTED_DOMAIN='$TRUSTED_DOMAIN'"

if [ "$DOMAIN" = "$TRUSTED_DOMAIN" ]; then
  echo "ERROR: You cannot trust yourself."
  echo "       Specify TRUSTED_DOMAIN which is not the local DOMAIN"
  exit 1
fi

# /etc/resolv.conf fixup
if [ ! -f /etc/resolv.conf.orig ]; then
  # Eliminate symlink /etc/resolv.conf, making a real file, which is unmanaged
  mv /etc/resolv.conf /etc/resolv.conf.orig
  cat /etc/resolv.conf.orig | \
    sed -e "s|nameserver.*|nameserver $DNS_NAME_SERVER|" \
        -e "/nameserver.*/i\
search $DOMAIN_LC" > /etc/resolv.conf
fi

# Assume if ic-config package is installed, then this system was already
# installed/promoted. When true, only configure cross-realm trust with the
# named trusted domain.
#
if [ `rpm -qa | grep -c vmware-ic-config | grep -v grep` -eq 0 ]; then
  echo '+++ Install RPMs +++'
  # Install RPMs
  for i in $files; do
    rpm -ivh $i
    sleep 1
  done

  echo '+++ Configure registry +++'
  # Configure registry
  /opt/likewise/bin/lwregshell set_value '[HKEY_THIS_MACHINE\Services\lsass\Parameters\Providers]' LoadOrder ActiveDirectory VmDir Local
  /opt/likewise/bin/lwsm refresh
  /opt/likewise/bin/lwsm restart lsass
  /opt/likewise/bin/lwsm start vmdir
  /opt/likewise/bin/lwsm start vmafd
  /opt/likewise/bin/lwsm start vmca


  echo '+++ Promote vmdir +++'
  # Promote vmdir instance
  /opt/vmware/bin/ic-promote --domain $DOMAIN --password 'VMware123@'

  echo '+++ Enable nsswitch and pam +++'
  /opt/likewise/bin/domainjoin-cli configure --enable nsswitch
  /opt/likewise/bin/domainjoin-cli configure --enable pam

  echo '+++ Configure Kerberos +++'
  # Modify /etc/krb5.conf
  echo 'include /etc/krb5.lotus.conf' >> /etc/krb5.conf
  /opt/vmware/bin/vmkdc_admin addprinc -p VMware123@ abernstein@$DOMAIN
  /opt/vmware/bin/vmkdc_admin addprinc -p VMware123@ adam@$DOMAIN
  /opt/vmware/bin/vmkdc_admin addprinc -p Testing123@ test1@$DOMAIN
fi

# Create Kerberos cross-realm relationship with trusted domain
/opt/vmware/bin/vmkdc_admin \
  addprinc -p "Testing123@" \
  "krbtgt/${DOMAIN}@${TRUSTED_DOMAIN}"

/opt/vmware/bin/vmkdc_admin \
  addprinc -p "Testing123@" \
  "krbtgt/${TRUSTED_DOMAIN}@${DOMAIN}"

# Add rules similar to these:
# auth_to_local = RULE:[1:$1@$0](.*@TESTLAB12.COM)s/@.*//
# auth_to_local = DEFAULT
#
if [ ! -f /etc/krb5.lotus.conf.orig ]; then
  cp /etc/krb5.lotus.conf /etc/krb5.lotus.conf.orig 
fi
cat /etc/krb5.lotus.conf | \
  sed -e "/$DOMAIN  *=  *{/,/}/{
/}/i\
\\\t    auth_to_local = RULE:[1:\$1@\$0](.*@$TRUSTED_DOMAIN)s/@.*//\n\\t    auth_to_local = DEFAULT
}" > /etc/krb5.lotus.conf.sed
mv /etc/krb5.lotus.conf.sed /etc/krb5.lotus.conf

# Fix /etc/pam.d/system-auth smartcard entry
#
if [ ! -f /etc/pam.d/system-auth.orig ]; then
  cp /etc/pam.d/system-auth /etc/pam.d/system-auth.orig
fi
SMARTCARD_CONFIG=`grep smartcard /etc/pam.d/system-auth | head -n 1 | sed 's/requisite/sufficient/'`
cat /etc/pam.d/system-auth | sed '/smartcard/d' | sed "/^auth.*sufficient.*pam_lsass\.so.*/a\
$SMARTCARD_CONFIG" > /etc/pam.d/system-auth.sed
mv /etc/pam.d/system-auth.sed /etc/pam.d/system-auth


