#!/bin/bash
postdir=/var/lib/vmware/post
#environment
mdb_stat -e $postdir
#free list
mdb_stat -ff $postdir
