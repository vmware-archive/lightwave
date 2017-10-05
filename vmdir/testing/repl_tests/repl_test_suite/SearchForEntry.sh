#!/bin/bash
#Input:
#   $1 - DN to search
#   $2 - key word to grep for
#   $3 - Search whether entry exists or does not exist
#   $4 - Final result log file path
#Functionality:
#   1) #Check whether a specified dn exists or does not exist in the federation
#Output:
#   - Write the result to FinalResult file
#Usage:
#   ./SearchForEntry.sh "$USER_DN" "$USER" "DoesNotExists" $FINAL_RESULT)
#   Some of the variables are exported from BasicReplTests.sh/BasicJoinAndLeaveTests.sh


SEARCH_DN=$1
SEARCH_STRING=$2
SEARCH_TYPE=$3
FINAL_RESULT=$4
NODE_LEAVE_FED=$5

MAX_RETRY=12

function writeToLogAndResult
{
    CONTENT=$1
    echo "$CONTENT" >> $FINAL_RESULT
}


function SearchForEntry
{
    B_SEARCH_FAILURE=false
    while IFS='' read -r line || [[ -n "$line" ]];
    do
        CURR_COUNT=0

        # Script is performing Leave federation tests - hence skip the nodes not part of federation
        if [[ "$line" == "$NODE_LEAVE_FED" ]]; then
            writeToLogAndResult "    Search $Search_DN skipping $line "
            continue
        fi

        # Based on the search type - search the entry - time out 2 mins
        while [ $CURR_COUNT -lt $MAX_RETRY ]
        do
            if [[ "$SEARCH_TYPE" == "Exists" ]]; then
                if [ ! -z "$(ldapsearch -h "$line" -p $PORT -LLL -Y "SRP" -U "$ADMIN_UPN" -w $PASSWD  -b "$SEARCH_DN" | grep $SEARCH_STRING)" ]; then
                    writeToLogAndResult "    Success: Grep for $SEARCH_STRING in $SEARCH_DN sub-tree found in node: $line "
                    break
                fi
            else
                if [ -z "$(ldapsearch -h "$line" -p $PORT -LLL -Y "SRP" -U "$ADMIN_UPN" -w $PASSWD  -b "$SEARCH_DN" | grep $SEARCH_STRING)" ]; then
                    writeToLogAndResult "    Success: Grep for $SEARCH_STRING in $SEARCH_DN sub-tree not found in node: $line "
                    break
                fi
            fi
            writeToLogAndResult "    Search_for_entry: sleep for 10 seconds, before retrying"
            sleep 10
            CURR_COUNT=$((CURR_COUNT+1))
        done

        if [ $CURR_COUNT -eq $MAX_RETRY ]; then
            B_SEARCH_FAILURE=true
            if [[ "$SEARCH_TYPE" == "Exists" ]]; then
                writeToLogAndResult "    Failure: Grep for $SEARCH_STRING in $SEARCH_DN sub-tree not found in node: $line  "
            else
                writeToLogAndResult "    Failure: Grep for $SEARCH_STRING in $SEARCH_DN sub-tree found in node: $line "
            fi
            break
        fi
    done < "$HOST_FILE"

    echo $B_SEARCH_FAILURE
}

#perform search
SearchForEntry
