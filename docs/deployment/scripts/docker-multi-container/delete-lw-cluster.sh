#!/bin/sh

docker ps -qa --filter "name=lightwave" | xargs docker kill
docker ps -qa --filter "name=lightwave" | xargs docker rm
docker network rm lightwave
