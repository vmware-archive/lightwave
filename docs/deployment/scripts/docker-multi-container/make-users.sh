#!/bin/sh -xe

docker cp ./make-users-helper.sh lightwave-0:/

docker exec -t lightwave-0 /make-users-helper.sh
