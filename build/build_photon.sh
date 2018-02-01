#!/bin/sh

autoreconf -vif .. \
  && \
../configure \
    CFLAGS="-Wall -Werror -Wno-unused-but-set-variable -Wno-pointer-sign -Wno-implicit-function-declaration -Wno-address -Wno-enum-compare" \
    --prefix=/opt/vmware \
    --libdir=/opt/vmware/lib64 \
    --localstatedir=/var/lib/vmware \
    --with-config=./config \
  && \
 make \
  && \
 make package

if [ $# -eq 1 ];then
    if [ $1 = "--with-ui" ];then
       make -C ../ui
       cp ../ui/lwraft-ui/stage/RPMS/x86_64/*.rpm rpmbuild/RPMS/x86_64/
       cp ../ui/lightwave-ui/stage/RPMS/x86_64/*.rpm rpmbuild/RPMS/x86_64/
    fi
fi
