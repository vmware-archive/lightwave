#!/bin/bash
#Input:
#   $1 - hostname
#   $2 - File path to save the DIT content file
#Functionality:
#   1) Number of entries present in the specified domain
#   2) In addition to that generates a file which contains all the DN's present in specified domain
#Usage:
#   ./DITcheck.sh localhost "/DIT/DITContent/"
#   This script makes use of some exported variables, check BasicJoinAndLeaveTests.sh/BasicJoinReplTests.sh

#host instance
HOST=$1
#DIT file path
DIT_DIR=$2
#Requested FILTER
FILTER=$3

function performDITCheck
{
    #perform ldapsearch to obtain all DN's and redirect to a file (DIT_<hostname>)
    ldapsearch -h $HOST -p $PORT -o ldif-wrap=no -LLL -Y "SRP" -U "$ADMIN_UPN" -w $PASSWD  -b $DOMAIN -s sub $FILTER > $DIT_DIR$HOST

    #remove empty lines from the file
    sed -i '/^[[:space:]]*$/d' $DIT_DIR$HOST

    #extract entry count from the file
    ENTRY_COUNT=$(wc -l < $DIT_DIR$HOST)

    echo $ENTRY_COUNT
}

#perform DIT check
performDITCheck
