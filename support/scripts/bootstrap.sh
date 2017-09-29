#!/bin/bash

PROJECT_ROOT=$(pwd)

cd $PROJECT_ROOT/build && \
    autoreconf -fi .. && \
    ../configure \
    CFLAGS="-Wall -Werror -Wno-unused-but-set-variable -Wno-pointer-sign -Wno-implicit-function-declaration -Wno-address -Wno-enum-compare" \
    --prefix=/opt/vmware \
    --libdir=/opt/vmware/lib64 \
    --localstatedir=/var/lib/vmware \
    --with-config=./config
