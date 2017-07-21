#!/bin/sh

autoreconf -vif .. \
  && \
../configure \
    CFLAGS="-Wall -Werror -Wno-unused-but-set-variable -Wno-pointer-sign -Wno-implicit-function-declaration -Wno-address -Wno-enum-compare" \
    --prefix=/opt/vmware \
    --libdir=/opt/vmware/lib64 \
    --localstatedir=/var/lib/vmware \
    --with-config=./config \
    --enable-rest=no \
    --with-maven=/usr/share/maven \
    --with-ant=/usr/share/ant \
  && \
 make \
  && \
 make package
