#!/bin/bash
ldapsearch -o ldif-wrap=no -h localhost -p 389 \
-x -D $VMDIR_BIND_DN -w $VMDIR_PWD \
-s sub  'objectclass=user*' -LLL
