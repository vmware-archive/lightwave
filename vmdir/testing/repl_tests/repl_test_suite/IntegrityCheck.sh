#!/bin/bash
#Input:
#   $1 - hostname
#   $2 - File path to save the IntegrityCheck file
#   $3 - debug (optional), to print debug messages
#Functionality:
#   1) Trigger integrity check for the specified node
#   2) Result of the IntegrityCheck will be written to the file path
#Usage:
#   ./IntegrityCheck.sh localhost "./IntegrityCheck/"
#   Some of the variables are exported from BasicReplTests.sh/BasicJoinAndLeaveTests.sh
#Output:
#   - Wait for Integritycheck to complete and summarize the entries processed in BasicReplTestResults
#Note:
#Integrity logs generted by the Integritycheck tool will be in /var/log/lightwave on corresponding nodes
#In the generated log file, I - Digest mismatch and M - Missed Entry

#host instance
HOST=$1
FILE_PATH=$2

function writeToLogAndResult
{
    CONTENT=$1
    echo "$CONTENT" >> $FINAL_RESULT
}

#enable debug flag
if [[ $3 == "debug" ]]; then
    debug=true
fi

#declarations
ENTRIES_PROCESSED=0
PREV_ENTRIES_PROCESSED=0
declare -i RETRY=0

function IntegrityCheck
{
    #base dn for integritycheckstatus
    INTEGRITY_CN="cn=integritycheckstatus"

    #common filter for search
    FILTER="(objectclass=*)"
    OPTION="ldif-wrap=no"

    #specific filter
    START_CHECK="start"

    #start Integrity check on HOST
    ldapsearch -h $HOST -p $PORT -o $OPTION -Y "SRP" -U "$ADMIN_UPN" -w $PASSWD  -b $INTEGRITY_CN -s base $FILTER $START_CHECK

    #specific filter
    STATUS_CHECK="vmwServerRunTimeStatus"

    #Below logic is to determine whether Integrity check is making progress
    #If integrity check is making progress wait for it to complete, if integrity check entries processed count
    #is the same for 10 retries consider as done and bail out.
    while [ $RETRY -lt 10 ]
    do
        sleep 1
        ldapsearch -h $HOST -p $PORT -o $OPTION -Y "SRP" -U "$ADMIN_UPN" -w $PASSWD  -b $INTEGRITY_CN -s one $FILTER $STATUS_CHECK > $FILE_PATH$HOST
        ENTRIES_PROCESSED=$(awk '/entries processed/' $FILE_PATH$HOST)
        ENTRIES_PROCESSED=$(echo $ENTRIES_PROCESSED | sed 's/[^0-9]//g')
        if [[ "$PREV_ENTRIES_PROCESSED" -eq "$ENTRIES_PROCESSED" ]]; then
           (( RETRY++ ))
        else
           RETRY=0
        fi

        if [ $debug ]; then
            echo ""
            echo "ENTRIES PROCESSED: $ENTRIES_PROCESSED"
           echo "PREV_ENTRIES PROCESSED: $PREV_ENTRIES_PROCESSED"
       fi
       PREV_ENTRIES_PROCESSED=$ENTRIES_PROCESSED
    done

    #Result
    writeToLogAndResult "    $HOST: Done, Entries Processed Count: $ENTRIES_PROCESSED"
}

#Perform Integriy check
IntegrityCheck
