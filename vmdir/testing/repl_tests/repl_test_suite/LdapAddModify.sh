#!/bin/bash
#Input:
#    1) hostfile
#    2) start user numebr - newuser<start>
#    3) count - number of users to add newuser<start> to newuser<start+count>
#    4) modifyCount - Number of users to perform modify
#Functionality:
#    1) Perform Add and modify requested number of users
#Usage:
#    1) ./LdapAddModify.sh localhost 1 100 50
#    will add 100 users from newuser1 to newuser100 and perform modify of 50 users randomly

#Input variables with sanity check
HOST=$1

start=$2
if [ "$start" == "" ]
then
    echo "provide start"
    exit 1
fi

count=$3
if [ "$count" == "" ]
then
    echo "provide count"
    exit 1
fi

modifyCount=$4
if [[ "$modifyCount" == "" || $modifyCount -gt $count ]]
then
    echo "provide modify count or modify count provided is greater than number of entries we are tyring to add"
    exit 1
fi

function addUser
{
    for ((x = start; x < start+count; x++))
    do
        #Start of auto generate code
        userDn="dn: cn=newuser"
        userDn+=$x
        partialDN=",cn=users,"
        partialDN=$partialDN$DOMAIN

        Dn="$userDn$partialDN"

        userSn="sn: newuser"
        userSn+=$x

        userCn="cn: newuser"
        userCn+=$x
        #end of auto generate code

       #perform ldapadd
ldapadd -c -v -h $HOST -p $PORT -Y "SRP" -U "$ADMIN_UPN" -w $PASSWD <<EOM
$Dn
changetype: add
$userSn
$userCn
objectclass: top
objectclass: person
objectclass: organizationalPerson
objectclass: user
EOM
    done
}

function modifyUser
{
    i=0
    while [[ $i -lt $modifyCount ]]
    do

        r=$RANDOM
        x=$((r%count))
        x=$((start+x))

        #perform ldapmodify
ldapmodify -c -h $HOST -p $PORT -Y "SRP" -U "$ADMIN_UPN" -w $PASSWD  <<EOM
dn: cn=newuser$x,cn=users,$DOMAIN
changetype: modify
replace: description
description: $r
EOM
        i=$((i+1))
    done
}

#perform ldap operations
addUser
modifyUser
