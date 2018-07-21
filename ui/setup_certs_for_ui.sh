CERT_PATH=/etc/vmware/ssl

mkdir -p $CERT_PATH

/opt/vmware/bin/vecs-cli entry getkey --store MACHINE_SSL_CERT \
                                --alias __MACHINE_CERT \
                                --output $CERT_PATH/machine-ssl.key.tmp

sed '/^\s*$/d' $CERT_PATH/machine-ssl.key.tmp > $CERT_PATH/machine-ssl.key

chmod 0400 $CERT_PATH/machine-ssl.key

rm -f $CERT_PATH/machine-ssl.key.tmp

/opt/vmware/bin/vecs-cli entry getcert --store MACHINE_SSL_CERT \
                                 --alias __MACHINE_CERT \
                                 --output $CERT_PATH/machine-ssl.crt

cert_alias=$(/opt/vmware/bin/vecs-cli entry list --store TRUSTED_ROOTS | \
                                                    grep "Alias" | \
                                                    cut -d: -f2)

for cert in $cert_alias
do
/opt/vmware/bin/vecs-cli entry getcert --store TRUSTED_ROOTS \
                                             --alias $cert \
                                             --output $CERT_PATH/cert.tmp

cat $CERT_PATH/cert.tmp >> $CERT_PATH/machine-ssl.crt

done

rm -f $CERT_PATH/cert.tmp
