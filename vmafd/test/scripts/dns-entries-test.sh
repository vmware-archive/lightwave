#!/bin/bash

###
# This script verifies that DNS records are properly created for a federation
# of promoted DCs and joined clients.
# This script assumes that no other DNS records are created except that of the
# DCs and of the clients promoted/joined to the federation.
###

DCIP=$1
DOMAIN=$2
PASSWORD=$3
NUMDCS=$4
NUMCLIENT=$5
SSHUSER=$6
SSHFLAGS=$7

# Each DC needs 4 SRV Records:
#     - 1 LDAP
#     - 1 Kerberos
#     - 1 LDAP MSDCS
#     - 1 Kerberos MSDCS
NUMSRVPERDC=4

if [[ "$#" -lt "6" ]] ; then
    echo "[ERROR] invalid arguments"
    echo -e "Usage:"
    echo -e "\t$0 DC_IP DOMAIN PASSWORD NUM_DCS NUM_CLIENTS SSH_USER [SSH_FLAGS]"
    exit 1
fi

## 1. Calculate expected number of records

# There should only be one SOA record per zone/domain
NUMEXPECTEDSOARRS=1
NUMEXPECTEDARRS=$((${NUMCLIENT} + ${NUMDCS}))
NUMEXPECTEDNSRRS=$((${NUMDCS}))
NUMEXPECTEDSRVRRS=$((${NUMDCS} * ${NUMSRVPERDC}))
NUMEXPECTEDRRS=$((${NUMEXPECTEDSOARRS} +
            ${NUMEXPECTEDARRS} +
            ${NUMEXPECTEDNSRRS} +
            ${NUMEXPECTEDSRVRRS}))

## 2. Get List-Records output from DC and calculate actual number of records

LISTRECORDS=$(ssh ${SSHFLAGS} ${SSHUSER}@${DCIP} \
              "/opt/vmware/bin/vmdns-cli list-record --zone ${DOMAIN} --password ${PASSWORD}")
NUMRRS=$(echo "${LISTRECORDS}" \
         | grep 'Total number' \
         | cut -d: -f2 \
         | tr -d ' ')
NUMSOARRS=$(echo "${LISTRECORDS}" \
            | grep "Type:" \
            | grep "\<SOA\>" \
            | wc -l \
            | tr -d ' ')
NUMARRS=$(echo "${LISTRECORDS}" \
          | grep "\<A\>" \
          | wc -l \
          | tr -d ' ')
NUMNSRRS=$(echo "${LISTRECORDS}" \
           | grep "\<NS\>" \
           | wc -l \
           | tr -d ' ')
NUMSRVRRS=$(echo "${LISTRECORDS}" \
            | grep "\<SRV\>" \
            | wc -l \
            | tr -d ' ')

## 3. Verify that the number of records matches our expected values

if [[ "${NUMSOARRS}" != "${NUMEXPECTEDSOARRS}" ]] ; then
    echo "[ERROR] invalid number of SOA records"
    echo -e "\nFAILURE :("
    exit 1
fi

if [[ "${NUMARRS}" != "${NUMEXPECTEDARRS}" ]] ; then
    echo "[ERROR] invalid number of A records"
    echo -e "\nFAILURE :("
    exit 1
fi

if [[ "${NUMNSRRS}" != "${NUMEXPECTEDNSRRS}" ]] ; then
    echo "[ERROR] invalid number of NS records"
    echo -e "\nFAILURE :("
    exit 1
fi

if [[ "${NUMSRVRRS}" != "${NUMEXPECTEDSRVRRS}" ]] ; then
    echo "[ERROR] invalid number of SRV records"
    echo -e "\nFAILURE :("
    exit 1
fi

if [[ "${NUMRRS}" != "${NUMEXPECTEDRRS}" ]] ; then
    echo "[ERROR] invalid total number of records"
    echo -e "\nFAILURE :("
    exit 1
fi

echo -e "\nSUCCESS :)"
