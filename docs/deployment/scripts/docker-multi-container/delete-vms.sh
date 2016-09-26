#!/bin/sh

docker-machine ls -q | xargs docker-machine rm -y -f
