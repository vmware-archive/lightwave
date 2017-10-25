#!/bin/bash

PROJECT_ROOT=$(pwd)

cd $PROJECT_ROOT/build && \
        make distclean

/bin/rm -rf `find $PROJECT_ROOT -name Makefile.in`

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
            vmmetrics
