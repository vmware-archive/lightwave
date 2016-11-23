#
# Copyright (c) VMware Inc.  All rights Reserved.
# Module Name: common Makefile
# Abstract: VMware Domain Name Service.
# Created on: Sep 18, 2012
# Author: Sanjay Jain (sanjain@vmware.com)
#         Sriram Nambakam (snambakam@vmware.com)
#

noinst_LTLIBRARIES = libcommon.la

libcommon_la_SOURCES = \
    config.c \
    posixconfig.c \
    buffer.c  \
    globals.c  \
    logging.c \
    memory.c  \
    record.c  \
    record-a.c  \
    record-aaaa.c  \
    record-cname.c  \
    record-ns.c  \
    record-ptr.c  \
    record-soa.c  \
    record-srv.c  \
    record-tkey.c  \
    record-tsig.c  \
    rpcmemory.c  \
    string.c \
    threading.c \
    utils.c

libcommon_la_CPPFLAGS = \
    -I$(top_srcdir)/include \
    -I$(top_srcdir)/include/public \
    @DCERPC_INCLUDES@ \
    @LW_INCLUDES@

libcommon_la_LDFLAGS = \
    -static
