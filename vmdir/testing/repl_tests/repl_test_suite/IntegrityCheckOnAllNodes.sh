#!/bin/bash
#Input:
#   $1 - Node to Skip
#   $2 - ExecCount (for better logging)
#Functionality:
#   1) Trigger integrity check for all the nodes in the federation.
#Output:
#   - Write the result to FinalResult file
#Usage:
#   ./IntegrityCheckOnAllNodes.sh "" "IntegrityCheck/AfterIO/"
#   Some of the variables are exported from BasicReplTests.sh/BasicJoinAndLeaveTests.sh

SKIP_NODE=$1
INTEGRITY_CHECK=$2

#FINAL_RESULT="BasicReplTestResults"
FAIL=false

mkdir -p $INTEGRITY_CHECK

function writeToLogAndResult
{
    CONTENT=$1
    echo "$CONTENT" >> $FINAL_RESULT
}

function IntegrityCheckOnAllNodes
{
    writeToLogAndResult ""
    writeToLogAndResult "Starting IntegrityCheck on all nodes in the federation"

    #launch integrity check on one node after the other and wait for it to complete.
    while IFS='' read -r line || [[ -n "$line" ]]; do
        if [[ "$line" != "$SKIP_NODE" ]]; then
            ./IntegrityCheck.sh $line $INTEGRITY_CHECK
        fi
    done < "$HOST_FILE"

    #Check the missing and digest mismatch entry count from integrity check output,
    #any value greater than '0' will be considered as failure
    while IFS='' read -r line || [[ -n "$line" ]]; do
        if [[ "$line" != "$SKIP_NODE" ]]; then
            RESULTS=($(awk -F "[()]" '{ for (i=2; i<NF; i+=2) print $i }' $INTEGRITY_CHECK$line ))
            RESULTS=( "${RESULTS[@]/objectclass=*}" )
            RESULTS[0]="0"
            for i in "${RESULTS[@]}"
            do
                if [[ "$i" != "0" ]]; then
                    writeToLogAndResult "    Failure: Integrity check on $line failed"
                    FAIL=true
                    break
                fi
            done

            if [ "$FAIL" = true ]; then
                break
            fi
        fi
    done < "$HOST_FILE"

    if [ "$FAIL" = false ]; then
        writeToLogAndResult "Success: Integrity check did not report any mismatch in the federation "
    fi

    writeToLogAndResult "Integrity check on all nodes completed successfully"
}

IntegrityCheckOnAllNodes
