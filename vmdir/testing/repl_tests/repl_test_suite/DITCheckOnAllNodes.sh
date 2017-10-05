#!/bin/bash
#Input:
#   $1 - Node to Skip DIT check
#   $2 - ExecCount (for better logging)
#Functionality:
#   1) For all the Nodes in the federation, dump all the DN's present under DIT.
#   2) V1: Verify whether entries(dn) count is matching across all the nodes in the federation.
#   3) V2: Verify whether all the entries (dn) are matching across all the nodes in the federation.
#   4) Perform V1 and V2, if it does not find any mismatch - success
#   5) Final result will be writen to file specified by FinalResult
#Output:
#   - Logs all the results in BasicReplTestResults file
#Usage:
#   ./DITCheckOnAllNodes.sh "" "0"
#   Some of the variables are exported from BasicReplTests.sh/BasicJoinAndLeaveTests.sh

SKIP_NODE=$1
EXEC_COUNT=$2

DIT_CONTENT="./DIT/DITContents_$EXEC_COUNT/"
DIT_RESULT="./DIT/DITResults_$EXEC_COUNT/"

#12 * 10 seconds = 120 seconds
MAX_RETRY=12

#Create Necessary directories to store results and contents
mkdir -p DIT
mkdir -p $DIT_CONTENT
mkdir -p $DIT_RESULT

declare -a DIT_ENTRY_COUNT
declare -a HOST_ARR
declare -i COUNT=0


function writeToLogAndResult
{
    CONTENT=$1
    echo "$CONTENT" >> $FINAL_RESULT
}

function performDITCheck
{
    CONTENT_PATH=$1
    RESULT_PATH=$2

    COUNT=0
    #Retrieve DIT (all the dn's) Contents from all the Nodes in the federation
    while IFS='' read -r line || [[ -n "$line" ]]; do
        if [[ "$SKIP_NODE" != "$line" ]]; then
            DIT_ENTRY_COUNT[COUNT]=$(./DITcheck.sh $line $CONTENT_PATH "dn")
            COUNT=$(( $COUNT + 1 ))
        fi
    done < "$HOST_FILE"

    #As a first step of verification - perform DIT entry count across all the nodes
    #Even if there is a mismatch with one of the node, fail the test
    for ((i=0; i < $COUNT-1; i++)); do
        if [[ ${DIT_ENTRY_COUNT[$i]} -ne ${DIT_ENTRY_COUNT[$i+1]} ]]; then
            writeToLogAndResult "    Failure: DIT entry count for ${HOST_ARR[$i]} is ${DIT_ENTRY_COUNT[$i]} not matching with the ${HOST_ARR[$i+1]} which is ${DIT_ENTRY_COUNT[$i+1]}"
            break
        fi
    done

    writeToLogAndResult "    Success: DIT entry count on all the nodes in the federation match"

    #Perform a Diff between first node's dn list with all other nodes one by one
    #If there is a mismatch, fail the test.
    for ((i=0; i < $COUNT-1; i++)); do
        DIFF_FILE="${RESULT_PATH}Diff_${HOST_ARR[$i]}_with_${HOST_ARR[$i+1]}"
        diff "$CONTENT_PATH${HOST_ARR[$i]}" "$CONTENT_PATH${HOST_ARR[$i+1]}" > $DIFF_FILE
        if [ -s $DIFF_FILE ]; then
            writeToLogAndResult "$DIFF_FILE is not empty"
            writeToLogAndResult "    Failure: $DIFF_FILE is not empty"
            break
          else
              writeToLogAndResult "    Success: ${HOST_ARR[$i]} DIT entries match with ${HOST_ARR[$i+1]}"
          fi
    done
}

function DITCheckOnAllNodes
{
     #Extract the HOST file to HOST_ARR
    while IFS='' read -r line || [[ -n "$line" ]]; do
        HOST_ARR+=("$line")
    done < "$HOST_FILE"

    writeToLogAndResult "Starting DITCheck on all the Nodes in the Federation "

    #Perform DIT check without IO's
    performDITCheck $DIT_CONTENT $DIT_RESULT

    writeToLogAndResult "DIT check on all nodes completed successfully"
}

#perform DIT check on all nodes
DITCheckOnAllNodes
