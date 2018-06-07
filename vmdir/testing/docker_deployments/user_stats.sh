#!/bin/bash

function user_stats() {
    docker ps -f id=$1
    docker exec -t $1 /vmdir_user_stats.sh
}

docker ps -qaf network=vmdir_test_net | xargs -n1 -I{} bash -c "$(declare -f user_stats) ; user_stats {} ;"
