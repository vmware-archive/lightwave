#!/bin/bash
#this is a continuation of run_hmake_sanity so the test container is the same
#Assumptions:
#1. env file is available at $LIGHTWAVE_ENV_FILE
#2. test setup with a primary and partner is available
#3. client rpms are installed

#source env variables
source $LIGHTWAVE_ENV_FILE

#check environment vars
if [ -z "$LIGHTWAVE_DOMAIN" -o -z "$LIGHTWAVE_PASS" ]; then
  echo "Please set LIGHTWAVE_DOMAIN and LIGHTWAVE_PASS in .env file"
  exit 1
fi

/opt/likewise/sbin/lwsmd --start-as-daemon

#before joining, set the flag to use vmdirrest
#this test is going to exercise a join with rest paths
#enabled.
/opt/likewise/bin/lwregshell set_value \
  '[HKEY_THIS_MACHINE\Services\vmafd\Parameters]' \
  UseVmDirREST 0x1

primary=server.$LIGHTWAVE_DOMAIN
vmdir_rest_api=$primary:7479/v1/vmdir/api

/opt/likewise/bin/lwsm autostart
sleep 1

#download certs from server so the cert refresh path can succeed
#get a token to exercise rest apis
#note the insecure flag till a certs api is invoked
#and local cert installed
TOK=$(curl -k \
     "https://$primary/openidconnect/token/$LIGHTWAVE_DOMAIN" \
     -H 'content-type: application/x-www-form-urlencoded' \
     -d 'grant_type=password' \
     -d "username=administrator@$LIGHTWAVE_DOMAIN" \
     --data-urlencode "password=$LIGHTWAVE_PASS" \
     -d 'scope=openid rs_vmdir' \
     | cut -f1 -d, | cut -f2 -d: | tr -d '"')

#install server cert in local cache. without this, the very first
#cert refresh call (which is going to be over rest) will fail
CERTSJSON=$(curl -k \
"https://$vmdir_rest_api/certs/rootcerts?detail=true" \
-H "Authorization: Bearer ${TOK}")

echo $CERTSJSON | jq '.certs[0].cert' | cut -d '"' -f2 | sed 's/\\n/\
/g' > /etc/ssl/certs/$primary.cert

CERTHASH=`openssl x509 -hash -noout -in /etc/ssl/certs/$primary.cert`
mv /etc/ssl/certs/$primary.cert /etc/ssl/certs/$CERTHASH.0


#this join is going to use ldap to communicate to directory
#for the join process but it will use rest for
#cert refresh and pass refresh.
/opt/vmware/bin/ic-join \
  --domain-controller $primary \
  --domain $LIGHTWAVE_DOMAIN \
  --password $LIGHTWAVE_PASS

#verify
/opt/vmware/bin/dir-cli nodes list \
--login Administrator@$LIGHTWAVE_DOMAIN \
--password $LIGHTWAVE_PASS \
--server-name $primary

mkdir -p /etc/vmware/vmware-vmafd

#run a force refresh
/opt/vmware/bin/vecs-cli force-refresh
