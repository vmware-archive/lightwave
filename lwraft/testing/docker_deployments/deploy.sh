#!/bin/bash
set -x
#
docker_net="post_test_net"
docker_image="docker_posttest"
#check if this network exists
docker network ls -f name="$docker_net" | grep "$docker_net"
if [ $? -ne 0 ]; then
  docker network create $docker_net
fi
#create n containers with names like post1, post2 etc.
for i in `seq 1 $1`;
do
  partner='none'
  if [ $i -gt 1 ]; then
    partner='partner'
  fi
  hostname=post$i.post.test
  docker ps -f name="$hostname" | grep "$hostname"
  if [ $? -ne 0 ]; then
    docker run --net=$docker_net --name $hostname -h $hostname -itd $docker_image
    $(docker exec -t $hostname /post_setup.sh $partner)
    sleep 1s
  fi
done
#
