#!/bin/bash
#usage:
#   $1: hostname
#   $2: newuser<count> from which delete has to start
#   $3: Number of users to delete
#Output
#   Perform LDAP delete on the request entires.
#Usage:
#   ./LdapAddModify.sh localhost 1 1
#   above specified input will delete cn=newuser1,cn=users,dc=lightwave,dc=local

#Input variables
HOST=$1
start=$2
count=$3

function deleteUsers
{
    i=0
    x=$start
    while [[ $i -lt $count ]]
    do
        dn="cn=newuser"$x",cn=users,"$DOMAIN
        ldapdelete -c -v -h $HOST -p $PORT -Y "SRP" -U "$ADMIN_UPN" -w $PASSWD $dn
        i=$((i+1))
        x=$((x+1))
    done
}

#perform ldap delete operation
deleteUsers
