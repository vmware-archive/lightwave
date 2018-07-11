#
# Copyright (c) VMware Inc.  All rights Reserved.
# Module Name: mdb-store Makefile
# Abstract: VMware Cloud Directory Platform.
# Created on: Oct 4, 2011
# Author: Sanjay Jain (sanjain@vmware.com)
#         Sriram Nambakam (snambakam@vmware.com)
#
#    -L$(top_srcdir)/vmdir/thirdparty/openldap/libraries/libmdb
#    -lmdb

noinst_LTLIBRARIES = libmdb-store.la

libmdb_store_la_SOURCES = \
    config.c \
    entry.c \
    dbstate.c \
    generic.c \
    globals.c \
    index.c \
    indexdesc.c \
    indexreadutil.c \
    indexwriteutil.c \
    init.c \
    iterate.c \
    legacy.c \
    txn.c

libmdb_store_la_CPPFLAGS = \
    -I$(top_srcdir)/vmdir/include \
    -I$(top_srcdir)/vmdir/include/public \
    -I$(top_srcdir)/vmdir/thirdparty/openldap/include \
    -I$(top_srcdir)/vmdir/thirdparty/openldap/libraries/liblber \
    -I$(top_srcdir)/vmdir/thirdparty/openldap/libraries/libldap \
    -I$(top_srcdir)/vmdir/thirdparty/openldap/libraries/mdb \
    -I$(top_srcdir)/vmdir/server/include \
    -I$(top_srcdir)/vmcommon/include/public \
    @LW_INCLUDES@ \
    @OPENSSL_INCLUDES@ \
    -D MDB_USE_PWRITEV

libmdb_store_la_LDFLAGS = \
    -static \
    $(top_builddir)/vmdir/thirdparty/openldap/libraries/mdb/libvmdirmdb.la \
    @LW_LDFLAGS@
