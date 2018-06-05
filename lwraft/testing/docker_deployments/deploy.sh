#!/bin/bash
set -x
#usage: first parameter is the number of containers to deploy
#       call without parameters assume number to be 1
#       deploy is a no-op if there are already n containers
#       launched.
#usage: second parameter is to turn log split on or off
#       set to 0 to start with combined log and state db
#       set to 1 to start log in a log db.
#
docker_net="post_test_net"
docker_image="docker_posttest"

container_count=1
log_split=false

if [[ $1 ]]; then
  container_count=$1
fi

if [ "$2" == 1 ]; then
  log_split=true
fi

#check if this network exists
docker network ls -f name="$docker_net" | grep "$docker_net"
if [ $? -ne 0 ]; then
  docker network create $docker_net
fi
#create n containers with names like post1, post2 etc.
for i in `seq 1 $container_count`;
do
  partner='none'
  if [ $i -gt 1 ]; then
    partner='partner'
  fi
  hostname=post$i.post.test
  docker ps -f name="$hostname" | grep "$hostname"
  if [ $? -ne 0 ]; then
    docker run --net=$docker_net --name $hostname -h $hostname -itd $docker_image
    $(docker exec -t $hostname /post_setup.sh $partner $log_split)
    sleep 1s
  fi
done
#
