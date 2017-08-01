#
# Copyright (c) VMware Inc.  All rights Reserved.
# Module Name: backend Makefile
# Abstract: VMware Cloud Directory Platform.
# Created on: Oct 4, 2011
# Author: Sanjay Jain (sanjain@vmware.com)
#         Sriram Nambakam (snambakam@vmware.com)
#

noinst_LTLIBRARIES = libbackend.la

libbackend_la_SOURCES = \
    backend.c           \
    globals.c           \
    util.c

libbackend_la_CPPFLAGS = \
    -I$(top_srcdir)/lwraft/include \
    -I$(top_srcdir)/lwraft/include/public \
    -I$(top_srcdir)/lwraft/thirdparty/openldap/include \
    -I$(top_srcdir)/lwraft/thirdparty/openldap/libraries/liblber \
    -I$(top_srcdir)/lwraft/thirdparty/openldap/libraries/libldap \
    -I$(top_srcdir)/lwraft/server/include \
    @LW_INCLUDES@ \
    @OPENSSL_INCLUDES@

libbackend_la_LDFLAGS = \
    -static
