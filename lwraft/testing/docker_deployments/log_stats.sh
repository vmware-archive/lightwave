#!/bin/bash

function log_stats() {
    docker ps -f id=$1
    docker exec -t $1 /post_log_stat.sh
}

docker ps -qf name=post.test | xargs -n1 -I{} bash -c "$(declare -f log_stats) ; log_stats {} ;"
