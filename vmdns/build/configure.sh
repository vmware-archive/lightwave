#!/bin/bash

#
# Expected: BUILDLOG_DIR will be defined in the environment
#

# include our custom script wrappers
export PATH=`pwd`:${PATH}

# verbose
#export V=3

echo BUILDLOG_DIR: $BUILDLOG_DIR

../configure $@
rc=$?

if [ "x${BUILDLOG_DIR}x" != "xx" ] && [ -d $BUILDLOG_DIR ]; then
    cp -pf config.log $BUILDLOG_DIR
fi


exit $rc
