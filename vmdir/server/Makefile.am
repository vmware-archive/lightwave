#
# Copyright (c) VMware Inc.  All rights Reserved.
# Module Name: top level Makefile
# Abstract: VMware Cloud Directory Platform.
# Created on: Oct 4, 2011
# Author: Sanjay Jain (sanjain@vmware.com)
#         Sriram Nambakam (snambakam@vmware.com)
#

STOREDIRS = mdb-store

if REPLICATION_V2
    REPLDIR = replication2
else
    REPLDIR = replication
endif

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
    api \
    rest-head \
    middle-layer \
    $(REPLDIR) \
    clusterstate \
    saslvmdirdb \
    schema \
    acl \
    vmkdc \
    vmdir \
    vmkdc_mit_tools
