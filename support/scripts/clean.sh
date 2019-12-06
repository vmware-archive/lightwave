#!/bin/bash

PROJECT_ROOT=$(pwd)

if [ -f $PROJECT_ROOT/build/Makefile ]; then
    cd $PROJECT_ROOT/build && make distclean
fi

/bin/rm -rf `find $PROJECT_ROOT -name Makefile.in`

cd $PROJECT_ROOT/build && \
    /bin/rm -rf config \
                docker \
                include \
                lwraft \
                rpmbuild \
                stage \
                vmafd \
                vmca \
                vmdir \
                vmdns \
                vmevent \
                vmidentity \
                sts \
                vmmetrics
