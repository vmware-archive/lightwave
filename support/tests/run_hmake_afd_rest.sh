#!/bin/bash
#this is a continuation of run_hmake_sanity so the test container is the same
#1. env file is available at $LIGHTWAVE_ENV_FILE
#2. test setup with a primary and partner is available
#3. client rpms are installed

#source env variables
source $LIGHTWAVE_ENV_FILE

/opt/likewise/sbin/lwsmd --start-as-daemon

#before joining, set the flag to use vmdirrest
/opt/likewise/bin/lwregshell set_value \
  '[HKEY_THIS_MACHINE\Services\vmafd\Parameters]' \
  UseVmDirREST 0x1

primary=server.$LIGHTWAVE_DOMAIN

/opt/likewise/bin/lwsm autostart
sleep 1

#join
/opt/vmware/bin/ic-join \
  --domain-controller $primary \
  --domain $LIGHTWAVE_DOMAIN \
  --password $LIGHTWAVE_PASS

#verify
/opt/vmware/bin/dir-cli nodes list \
--login Administrator@$LIGHTWAVE_DOMAIN \
--password $LIGHTWAVE_PASS \
--server-name $primary

#get a token to exercise rest apis
TOK=$(curl --capath /etc/ssl/certs \
     "https://$primary/openidconnect/token/$LIGHTWAVE_DOMAIN" \
     -H 'content-type: application/x-www-form-urlencoded' \
     -d 'grant_type=password' \
     -d "username=administrator@$LIGHTWAVE_DOMAIN" \
     --data-urlencode "password=$LIGHTWAVE_PASS" \
     -d 'scope=openid rs_vmdir' \
     | cut -f1 -d, | cut -f2 -d: | tr -d '"')
echo $TOK

#run password refresh without force
curl --capath /etc/ssl/certs \
-X POST \
"https://$primary:7479/v1/vmdir/api/password/refresh?force=false" \
-H "Authorization: Bearer ${TOK}"
