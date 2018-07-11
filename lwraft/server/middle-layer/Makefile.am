#
# Copyright (c) VMware Inc.  All rights Reserved.
# Module Name: middle-layer Makefile
# Abstract: VMware Cloud Directory Platform.
# Created on: Oct 4, 2011
# Author: Sanjay Jain (sanjain@vmware.com)
#         Sriram Nambakam (snambakam@vmware.com)
#

noinst_LTLIBRARIES = libmiddle-layer.la

libmiddle_layer_la_SOURCES = \
    add.c \
    bind.c \
    computedattribute.c \
    delete.c \
    dn.c \
    globals.c \
    group.c \
    index.c \
    krb.c \
    lockoutpolicy.c \
    modify.c \
    password.c \
    plugin.c \
    pscache.c \
    sasl.c \
    saslsockbuf.c \
    schema.c \
    search.c \
    specialsearch.c \
    srputil.c \
    libmain.c \
    util.c

libmiddle_layer_la_CPPFLAGS = \
    -I$(top_srcdir)/lwraft/include \
    -I$(top_srcdir)/lwraft/include/public \
    -I$(top_srcdir)/lwraft/thirdparty/openldap/include \
    -I$(top_srcdir)/lwraft/thirdparty/openldap/libraries/liblber \
    -I$(top_srcdir)/lwraft/thirdparty/openldap/libraries/libldap \
    -I$(top_srcdir)/lwraft/server/include \
    -I$(top_srcdir)/vmcommon/include/public \
    @OPENSSL_INCLUDES@ \
    @LW_INCLUDES@

libmiddle_layer_la_LDFLAGS = \
    -static
