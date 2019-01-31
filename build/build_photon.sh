#!/bin/sh

DISTRO=`cat /etc/os-release | grep VERSION_ID | cut -d= -f2`
ARG=$1

if [ $ARG == "--only-ui" ]; then
    make -C ../ui
    mkdir -p rpmbuild/RPMS/x86_64
    cp ../ui/stage/RPMS/x86_64/lightwave-ui-*.rpm rpmbuild/RPMS/x86_64/
    cp ../ui/stage/RPMS/x86_64/lwraft-ui-*.rpm rpmbuild/RPMS/x86_64/
    exit 0
fi

if [ $DISTRO == "1.0" ]; then
    DIST="%{nil}"
else
    DIST=".lwph2"
fi

./bootstrap.sh \
 && \
make \
 && \
make package DIST=$DIST \
 && \
make check
