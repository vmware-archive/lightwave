#
# Copyright (c) VMware Inc.  All rights Reserved.
# Module Name: bdb-store Makefile
# Abstract: VMware Cloud Directory Platform.
# Created on: Oct 4, 2011
# Author: Sanjay Jain (sanjain@vmware.com)
#         Sriram Nambakam (snambakam@vmware.com)
#

noinst_LTLIBRARIES = libbdb-store.la

libbdb_store_la_SOURCES = \
    attrindexing.c  \
    config.c        \
    entry.c         \
    indexer.c       \
    init.c          \
    globals.c       \
    logmgmt.c       \
    txn.c

libbdb_store_la_CPPFLAGS = \
    -I$(top_srcdir)/include \
    -I$(top_srcdir)/include/public \
    -I$(top_srcdir)/thirdparty/openldap/include \
    -I$(top_srcdir)/thirdparty/openldap/libraries/liblber \
    -I$(top_srcdir)/thirdparty/openldap/libraries/libldap \
    -I$(top_srcdir)/server/include \
    @BDB_INCLUDES@ \
    @LW_INCLUDES@

libbdb_store_la_LDFLAGS = \
    -static \
    @BDB_LDFLAGS@ \
    @LW_LDFLAGS@ \
    @BDB_LIBS@

