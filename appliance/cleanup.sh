#!/bin/bash -x
rm -rf /root/*
# Zero out empty space on the disk, so that unused,
# non-zero data does not end up in the compressed VMDK
cat /dev/zero > /zero; sync; rm -f /zero
