#
# Copyright (c) VMware Inc.  All rights Reserved.
# Module Name: top level Makefile
# Abstract: VMware Cloud Directory Platform.
# Created on: Oct 4, 2011
# Author: Sanjay Jain (sanjain@vmware.com)
#         Sriram Nambakam (snambakam@vmware.com)
#

STOREDIRS = mdb-store

SUBDIRS = \
    kdcsrvcommon \
    kdckrb5 \
    kdctools \
    common \
    backend \
    $(STOREDIRS) \
    indexcfg \
    metrics \
    ldap-head \
    rest-head \
    middle-layer \
    replication \
    saslvmdirdb \
    schema \
    acl \
    watchnotify \
    vmdir

if BUILD_GRPC
    SUBDIRS += grpc-head
endif
