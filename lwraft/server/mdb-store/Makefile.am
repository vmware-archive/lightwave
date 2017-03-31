#
# Copyright (c) VMware Inc.  All rights Reserved.
# Module Name: mdb-store Makefile
# Abstract: VMware Cloud Directory Platform.
# Created on: Oct 4, 2011
# Author: Sanjay Jain (sanjain@vmware.com)
#         Sriram Nambakam (snambakam@vmware.com)
#
#    -L$(top_srcdir)/thirdparty/openldap/libraries/libmdb
#    -lmdb

noinst_LTLIBRARIES = libmdb-store.la

libmdb_store_la_SOURCES = \
    config.c \
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
    -I$(top_srcdir)/include \
    -I$(top_srcdir)/include/public \
    -I$(top_srcdir)/thirdparty/openldap/include \
    -I$(top_srcdir)/thirdparty/openldap/libraries/liblber \
    -I$(top_srcdir)/thirdparty/openldap/libraries/libldap \
    -I$(top_srcdir)/thirdparty/openldap/libraries/mdb \
    -I$(top_srcdir)/server/include \
    @LW_INCLUDES@ \
    @OPENSSL_INCLUDES@

libmdb_store_la_LDFLAGS = \
    -static \
    $(top_builddir)/thirdparty/openldap/libraries/mdb/liblwraftmdb.la \
    @LW_LDFLAGS@
