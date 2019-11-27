#!/bin/bash

DISTRO=`cat /etc/os-release | grep VERSION_ID | cut -d= -f2`

if [ $DISTRO == "1.0" ]; then
    DIST="%{nil}"
elif [ $DISTRO == "2.0" ]; then
    DIST=".lwph2"
else
    DIST=".lwph3"
fi

PROJECT_ROOT=$(pwd)

cd $PROJECT_ROOT/build && \
    make package DIST=$DIST
