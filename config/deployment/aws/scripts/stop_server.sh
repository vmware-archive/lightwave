#!/bin/bash

/opt/likewise/bin/lwsm stop vmdns
/opt/likewise/bin/lwsm stop vmdir
/opt/likewise/bin/lwsm stop vmca
/opt/likewise/bin/lwsm stop vmafd
echo "ignoring errors"
