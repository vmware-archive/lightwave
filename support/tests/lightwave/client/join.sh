#!/bin/sh

prepare_sts_cert()
{
  CERT_PATH=/etc/vmware/ssl
  mkdir -p $CERT_PATH

  /opt/vmware/bin/vecs-cli entry getkey \
    --store MACHINE_SSL_CERT \
    --alias __MACHINE_CERT \
    --output $CERT_PATH/machine-ssl.key.tmp

  sed '/^\s*$/d' $CERT_PATH/machine-ssl.key.tmp > $CERT_PATH/machine-ssl.key
  chmod 0400 $CERT_PATH/machine-ssl.key
  rm -f $CERT_PATH/machine-ssl.key.tmp

  /opt/vmware/bin/vecs-cli entry getcert \
    --store MACHINE_SSL_CERT \
    --alias __MACHINE_CERT \
    --output $CERT_PATH/machine-ssl.crt

  cert_alias=$(/opt/vmware/bin/vecs-cli entry list --store TRUSTED_ROOTS | \
                                                            grep "Alias" | \
                                                            cut -d: -f2)

  for cert in $cert_alias
  do
    /opt/vmware/bin/vecs-cli entry getcert \
      --store TRUSTED_ROOTS \
      --alias $cert \
      --output $CERT_PATH/cert.tmp

    cat $CERT_PATH/cert.tmp >> $CERT_PATH/trusted-ssl.crt

  done

  rm -f $CERT_PATH/cert.tmp
}

install_sts()
{

  ADMIN=administrator
  SITE=Default-First-Site
  STS_ENDPOINT=`echo $HOSTNAME`:443

  # should this be done in rpm or by stssetup?
  mkdir -p /var/lwlogs
  echo > /var/lwlogs/stslog.log

  echo "running stssetup install ....."
  ADMINUPN="$ADMIN@$LIGHTWAVE_DOMAIN"

  /opt/vmware/bin/stssetup install \
    -admin-pwd=$LIGHTWAVE_PASS \
    -admin-uname=$ADMINUPN \
    -cfg-file='/etc/vmware/stssrv.conf' \
    -dir-certs-file='/etc/vmware/ssl/trusted-ssl.crt' \
    -log-file=/var/lwlogs/stslog.log \
    -log-level=4 \
    -port=443 \
    -primary \
    -sts-endpoint=$STS_ENDPOINT \
    -site=$SITE \
    -system-tenant=$LIGHTWAVE_DOMAIN \
    -tls-cert-file='/etc/vmware/ssl/machine-ssl.crt' \
    -tls-key-file='/etc/vmware/ssl/machine-ssl.key' \
    -enabled-heads='oidc,rest' \
    -daemon-group=lightwave \
    -daemon-user=lightwave

    if [ $? -ne 0 ]; then
        echo "failed to setup sts: log file=/var/lwlogs/stslog.log"
        exit 2
    fi
}

#check environment vars
if [ -z "$LIGHTWAVE_DOMAIN" -o -z "$LIGHTWAVE_PASS" ]; then
  echo "Please set LIGHTWAVE_DOMAIN and LIGHTWAVE_PASS in .env file"
  exit 1
fi

#prepare by installing rpms built in this build
rpm -Uvh --nodeps buildrpms/x86_64/lightwave-client*.rpm
rpm -Uvh --nodeps buildrpms/x86_64/lightwave-sts*.rpm

primary=lightwave_lightwave-server_1

#wait for server to promote
response=1
while [ $response -ne 0 ]; do
  netcat -v -z $primary 636 
  response=$?
  echo "waiting for $primary, response=$response"
  sleep 5
done

#join
/opt/vmware/bin/ic-join --domain-controller $primary --domain $LIGHTWAVE_DOMAIN --password $LIGHTWAVE_PASS > /tmp/join.out 2>&1

#verify
/opt/vmware/bin/dir-cli nodes list --login Administrator@$LIGHTWAVE_DOMAIN --password $LIGHTWAVE_PASS --server-name $primary

echo "preparing sts certs ..."
prepare_sts_cert

echo "installing sts ..."
install_sts

echo "starting sts ..."
# start sts
su -c '/opt/vmware/sbin/stssrv -config=/etc/vmware/stssrv.conf' -s /bin/sh lightwave > /tmp/join.out 2>&1

#
/bin/bash
