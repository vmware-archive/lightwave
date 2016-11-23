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
    dnsutils.c            \
    forwarder.c           \
    globals.c             \
    hashtable.c           \
    ldap.c                \
    lru.c                 \
    nameEntry.c           \
    recordlist.c          \
    recordobject.c        \
    securityutils.c       \
    dnsprotocol.c         \
    serviceapi.c          \
    sockinterface.c       \
    store.c               \
    util.c                \
    zone.c                \
    zonelist.c

libsrvcommon_la_CPPFLAGS = \
    -I$(top_srcdir)/include \
    -I$(top_srcdir)/include/public \
    -I$(top_srcdir)/server/include \
    @DCERPC_INCLUDES@ \
    @LW_INCLUDES@

libsrvcommon_la_LIBADD = \
		$(top_builddir)/vmsock/api/libvmsock.la

libsrvcommon_la_LDFLAGS = \
    -static
