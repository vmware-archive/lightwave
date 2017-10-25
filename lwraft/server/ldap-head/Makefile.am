#
# Copyright (c) VMware Inc.  All rights Reserved.
# Module Name: ldap-head Makefile
# Abstract: VMware Cloud Directory Platform.
# Created on: Oct 4, 2011
# Author: Sanjay Jain (sanjain@vmware.com)
#         Sriram Nambakam (snambakam@vmware.com)
#

noinst_LTLIBRARIES = libldap-head.la

libldap_head_la_SOURCES = \
    add.c           \
    bind.c          \
    connection.c    \
    controls.c      \
    delete.c        \
    filter.c        \
    globals.c       \
    metricsinit.c   \
    modify.c        \
    openssl.c       \
    operation.c     \
    opstatistic.c   \
    rename.c        \
    result.c        \
    search.c        \
    vecs.c          \
    unbind.c

libldap_head_la_CPPFLAGS = \
    -I$(top_srcdir)/lwraft/include \
    -I$(top_srcdir)/lwraft/include/public \
    -I$(top_srcdir)/lwraft/thirdparty/openldap/include \
    -I$(top_srcdir)/lwraft/thirdparty/openldap/libraries/liblber \
    -I$(top_srcdir)/lwraft/thirdparty/openldap/libraries/libldap \
    -I$(top_srcdir)/lwraft/server/include \
    -I$(top_srcdir)/vmmetrics/include/public \
    -I$(top_srcdir)/vmafd/include/public \
    @OPENSSL_INCLUDES@ \
    @LW_INCLUDES@

libldap_head_la_LDFLAGS = \
    -static
