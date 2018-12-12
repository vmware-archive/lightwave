#
# Copyright (c) VMware Inc.  All rights Reserved.
# Module Name: common Makefile
# Abstract: VMware Domain Name Service.
# Created on: Sep 18, 2012
# Author: Sanjay Jain (sanjain@vmware.com)
#         Sriram Nambakam (snambakam@vmware.com)
#

noinst_LTLIBRARIES = libsrvcommon.la

libsrvcommon_la_SOURCES = \
    cache.c               \
    dnsparser.c           \
    dnsprotocol.c         \
    dnsutils.c            \
    forwarder.c           \
    globals.c             \
    hashtable.c           \
    ldap.c                \
    lru.c                 \
    metricsenums.c        \
    nameEntry.c           \
    packetlist.c          \
    propertylist.c        \
    propertyobject.c      \
    recordlist.c          \
    recordobject.c        \
    registry.c            \
    securityutils.c       \
    serviceapi.c          \
    sockinterface.c       \
    store.c               \
    util.c                \
    vmafdlib.c            \
    zone.c                \
    zonelist.c

libsrvcommon_la_CPPFLAGS = \
    -I$(top_srcdir)/vmdns/include \
    -I$(top_srcdir)/vmdns/include/public \
    -I$(top_srcdir)/vmdns/server/include \
    -I$(top_srcdir)/vmcommon/include/public \
    @DCERPC_INCLUDES@ \
    @LW_INCLUDES@

libsrvcommon_la_LIBADD = \
    $(top_builddir)/vmdns/vmsock/api/libvmsock.la \
    $(top_builddir)/vmcommon/libvmcommon.la

libsrvcommon_la_LDFLAGS = \
    -static
