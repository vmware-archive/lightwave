#!/bin/sh
rm -f /tmp/*.keytab
/opt/vmware/bin/vmkdc_admin addprinc -p VMware123@ cifs/`hostname -f`@LIGHTWAVE.LOCAL
/opt/vmware/bin/vmkdc_admin ktadd -k /tmp/krb5.keytab cifs/`hostname -f`@LIGHTWAVE.LOCAL
echo "rkt /tmp/krb5.keytab
rkt /etc/krb5.keytab
wkt /tmp/new.keytab
quit" | ktutil

cp /tmp/new.keytab /etc/krb5.keytab
