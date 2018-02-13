#!/bin/sh

DISTRO=`cat /etc/os-release | grep VERSION_ID | cut -d= -f2`

if [ $DISTRO == "1.0" ]; then
    DIST="%{nil}"
else
    DIST=".lwph2"
fi

autoreconf -vif .. \
  && \
../configure \
    CFLAGS="-Wall -Werror -Wno-unused-but-set-variable -Wno-pointer-sign -Wno-implicit-function-declaration -Wno-address -Wno-enum-compare" \
    LDFLAGS=-ldl \
    --prefix=/opt/vmware \
    --libdir=/opt/vmware/lib64 \
    --localstatedir=/var/lib/vmware \
    --with-config=./config \
  && \
 make \
  && \
 make package DIST=$DIST

if [ $# -eq 1 ];then
    if [ $1 = "--with-ui" ];then
       make -C ../ui
       cp ../ui/lwraft-ui/stage/RPMS/x86_64/*.rpm rpmbuild/RPMS/x86_64/
       cp ../ui/lightwave-ui/stage/RPMS/x86_64/*.rpm rpmbuild/RPMS/x86_64/
    fi
fi
