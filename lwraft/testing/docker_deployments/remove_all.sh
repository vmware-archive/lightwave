#!/bin/bash

function remove_node() {
    docker ps -f id=$1
    echo 'Stopping..'
    docker stop $1
    echo 'Removing..'
    docker rm $1
}

docker ps -qf name=post.test | xargs -n1 -I{} bash -c "$(declare -f remove_node) ; remove_node {} ;"
