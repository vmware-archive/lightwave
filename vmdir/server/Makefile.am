#
# Copyright (c) VMware Inc.  All rights Reserved.
# Module Name: top level Makefile
# Abstract: VMware Cloud Directory Platform.
# Created on: Oct 4, 2011
# Author: Sanjay Jain (sanjain@vmware.com)
#         Sriram Nambakam (snambakam@vmware.com)
#

STOREDIRS =

if HAVE_MDB_STORE
    STOREDIRS += mdb-store
endif

SUBDIRS = \
    kdcsrvcommon \
    kdckrb5 \
    kdctools \
    common \
    backend \
    $(STOREDIRS) \
    indexcfg \
    ldap-head \
    middle-layer \
    replication \
    saslvmdirdb \
    schema \
    acl \
    vmkdc \
    vmdir \
    vmkdc_mit_tools
