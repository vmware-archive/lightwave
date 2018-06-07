#!/bin/bash

function node_stats() {
    docker ps -f id=$1
    docker exec -t $1 /opt/vmware/bin/post-cli node state --server-name  localhost --password $POST_PWD
}

docker ps -qf network=post_test_net | xargs -n1 -I{} bash -c "$(declare -f node_stats) ; node_stats {} ;"
