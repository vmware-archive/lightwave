#
# Copyright (c) VMware Inc.  All rights Reserved.
# Module Name: middle-layer Makefile
# Abstract: VMware Cloud Directory Platform.
# Created on: Feb 28, 2019
# Author: Prakash Arumugam (arumugamprak@vmware.com)
#

noinst_LTLIBRARIES = libdb-integritychk.la

libdb_integritychk_la_SOURCES = \
    dbintegritychk.c \
    dbintegritychkjob.c \
    dbintegritychkthread.c

libdb_integritychk_la_CPPFLAGS = \
    -I$(top_srcdir)/vmdir/include \
    -I$(top_srcdir)/vmdir/include/public \
    -I$(top_srcdir)/vmdir/thirdparty/openldap/include \
    -I$(top_srcdir)/vmdir/thirdparty/openldap/libraries/liblber \
    -I$(top_srcdir)/vmdir/thirdparty/openldap/libraries/libldap \
    -I$(top_srcdir)/vmdir/server/include \
    -I$(top_srcdir)/vmcommon/include/public \
    @OPENSSL_INCLUDES@ \
    @LW_INCLUDES@

libdb_integritychk_la_LDFLAGS = \
    -static

