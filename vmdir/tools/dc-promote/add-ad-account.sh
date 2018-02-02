#!/bin/sh
#
# Create a new account for the local domain with additional
# LDAP attributes required for Likewise-Open to lookup this account
#

DOMAIN="LIGHTWAVE.LOCAL"
LIGHTWAVE_AD=10.118.96.62
DC_DOMAIN="lightwave.local"
DC_NAME="photon-addc-test10"
ADMIN_PASSWORD="VMware123@"
PRIV_USER="root"
LDAP_LOGIN_DATA="-D CN=Administrator,CN=Users,DC=lightwave,DC=local -H ldap://10.118.96.62 -w VMware123@"

DC_DOMAIN_UC=`echo $DC_DOMAIN | tr 'a-z' 'A-Z'`


if [ -z "$1" ]; then
  echo "usage: $0 user_name"
  exit 1
fi
user="$1"
shift


echo -n "$user password: "
read -s pass
/opt/vmware/bin/vmkdc_admin addprinc -p $pass $user
DISTINGUISHED_NAME=`ldapsearch $LDAP_LOGIN_DATA -b "dc=lightwave,dc=local" '(objectClass=*)'  - + '*' | \
                    grep "^dn: cn=$user@$DC_DOMAIN_UC" | sed 's/^dn: cn=/cn=/'`

echo debug DISTINGUISHED_NAME=$DISTINGUISHED_NAME

#accountExpires: 0
#userAccountControl: 66048
#primaryGroupID: 513
#distinguishedName: cn=test1@LIGHTWAVE.LOCAL,cn=Users,dc=lightwave,dc=local

echo "=========== Adding new ldap attributes for $DISTINGUISHED_NAME ==============="

cat <<NNNN> /var/tmp/add-ad-user.ldif
version: 1
dn: $DISTINGUISHED_NAME
changetype: modify
add: distinguishedName
distinguishedName: $DISTINGUISHED_NAME
-
add: userAccountControl
userAccountControl: 66048
-
add: primaryGroupID
primaryGroupID: 513
-
add: accountExpires
accountExpires: 0
NNNN

ldapmodify $LDAP_LOGIN_DATA -w `cat /var/tmp/promote-pwd.txt` -f /var/tmp/add-ad-user.ldif
