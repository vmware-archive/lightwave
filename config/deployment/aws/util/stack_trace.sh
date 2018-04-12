#!/bin/bash -xe
#
# Script to get a stack traceback of a process

PROC=$1
PID=$(pidof $PROC)

# pstack is installed by the gdb package
tdnf install -y gdb-7.8.2

# get process info for all threads sorted by %CPU
top -p $PID -n 1 -b -H -o %CPU

# get the stack traceback
pstack $PID
