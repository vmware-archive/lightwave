#
# Copyright (c) VMware Inc.  All rights Reserved.
# Module Name: mdb-store Makefile
# Abstract: VMware Cloud Directory Platform.
# Created on: Oct 4, 2011
# Author: Sanjay Jain (sanjain@vmware.com)
#         Sriram Nambakam (snambakam@vmware.com)
#
#    -L$(top_srcdir)/lwraft/thirdparty/openldap/libraries/libmdb
#    -lmdb

noinst_LTLIBRARIES = libmdb-store.la

if MDB_0_9_22
    MDB_DIR = vmdir/thirdparty/openldap/libraries/liblmdb
    MDB_LIB = libvmdirmdb.la
else
    MDB_DIR = lwraft/thirdparty/openldap/libraries/mdb
    MDB_LIB = liblwraftmdb.la
endif

libmdb_store_la_SOURCES = \
    config.c \
    dbstate.c \
    entry.c \
    generic.c \
    globals.c \
    index.c \
    indexdesc.c \
    indexreadutil.c \
    indexwriteutil.c \
    init.c \
    iterate.c \
    txn.c

libmdb_store_la_CPPFLAGS = \
    -I$(top_srcdir)/lwraft/include \
    -I$(top_srcdir)/lwraft/include/public \
    -I$(top_srcdir)/lwraft/thirdparty/openldap/include \
    -I$(top_srcdir)/lwraft/thirdparty/openldap/libraries/liblber \
    -I$(top_srcdir)/lwraft/thirdparty/openldap/libraries/libldap \
    -I$(top_srcdir)/$(MDB_DIR) \
    -I$(top_srcdir)/lwraft/server/include \
    -I$(top_srcdir)/vmcommon/include/public \
    @LW_INCLUDES@ \
    @OPENSSL_INCLUDES@

libmdb_store_la_LDFLAGS = \
    -static \
    $(top_builddir)/$(MDB_DIR)/$(MDB_LIB) \
    @LW_LDFLAGS@
