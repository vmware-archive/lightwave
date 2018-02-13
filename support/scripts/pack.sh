#!/bin/bash

DISTRO=`cat /etc/os-release | grep VERSION_ID | cut -d= -f2`

if [ $DISTRO == "1.0" ]; then
    DIST="%{nil}"
else
    DIST=".lwph2"
fi

PROJECT_ROOT=$(pwd)

cd $PROJECT_ROOT/build && \
    make package DIST=$DIST
