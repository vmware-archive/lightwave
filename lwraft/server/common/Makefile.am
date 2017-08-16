#
# Copyright (c) VMware Inc.  All rights Reserved.
# Module Name: common Makefile
# Abstract: VMware Cloud Directory Platform.
# Created on: Oct 4, 2011
# Author: Sanjay Jain (sanjain@vmware.com)
#         Sriram Nambakam (snambakam@vmware.com)
#

noinst_LTLIBRARIES = libsrvcommon.la

libsrvcommon_la_SOURCES = \
    candidates.c \
    entryencodedecode.c \
    krbsrvutil.c \
    oprequestutil.c \
    security-sd.c \
    srp.c \
    util.c \
    vmdirentry.c \
    vmafdlib.c \
    vmdirtoldaperror.c

libsrvcommon_la_CPPFLAGS = \
    -I$(top_srcdir)/lwraft/include \
    -I$(top_srcdir)/lwraft/include/public \
    -I$(top_srcdir)/lwraft/thirdparty/openldap/include \
    -I$(top_srcdir)/lwraft/thirdparty/openldap/libraries/liblber \
    -I$(top_srcdir)/lwraft/thirdparty/openldap/libraries/libldap \
    -I$(top_srcdir)/lwraft/thirdparty/heimdal \
    -I$(top_srcdir)/lwraft/thirdparty/heimdal/krb5-crypto \
    -I$(top_srcdir)/lwraft/thirdparty/heimdal/asn1 \
    -I$(top_srcdir)/lwraft/thirdparty \
    -I$(top_srcdir)/lwraft/server/include \
    -I$(top_srcdir)/vmmetrics/include/public \
    @OPENSSL_INCLUDES@ \
    @DCERPC_INCLUDES@ \
    @LW_INCLUDES@

libsrvcommon_la_LDFLAGS = \
    -static
