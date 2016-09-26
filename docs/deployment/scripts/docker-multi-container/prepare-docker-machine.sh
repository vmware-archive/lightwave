#!/bin/sh -xe

docker-machine ls -q | xargs -I {} docker-machine scp ./prepare-docker-machine-helper.sh {}:/tmp/
docker-machine ls -q | xargs -I {} docker-machine ssh {} /tmp/prepare-docker-machine-helper.sh
