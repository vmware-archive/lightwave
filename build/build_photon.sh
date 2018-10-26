#!/bin/sh

DISTRO=`cat /etc/os-release | grep VERSION_ID | cut -d= -f2`
ARG=$1

if [ $DISTRO == "1.0" ]; then
    DIST="%{nil}"
else
    DIST=".lwph2"
fi

if [[ $ARG == "" || $ARG == "--with-ui" ]]; then
 ./bootstrap.sh \
  && \
 make \
  && \
 make package DIST=$DIST \
  && \
 make check
fi

if [[ $ARG == "--with-ui" || $ARG == "--only-ui" ]]; then
    make -C ../ui
    mkdir -p rpmbuild/RPMS/x86_64
    cp ../ui/stage/RPMS/x86_64/lightwave-ui-*.rpm rpmbuild/RPMS/x86_64/
    cp ../ui/stage/RPMS/x86_64/lwraft-ui-*.rpm rpmbuild/RPMS/x86_64/
fi
