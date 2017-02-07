#!/bin/bash -x
rm -rf /root/*
# Zero out empty space on the disk, so that unused,
# non-zero data does not end up in the compressed VMDK
dd if=/dev/zero of=/zero bs=1M oflag=direct; rm -f /zero
