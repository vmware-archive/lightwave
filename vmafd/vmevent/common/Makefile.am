#
# Copyright (c) VMware Inc.  All rights Reserved.
# Module Name: common Makefile
# Abstract: VMware Authentication Framework Service.
# Created on: Feb 13, 2013
# Author: Sanjay Jain (sanjain@vmware.com)
#         Sriram Nambakam (snambakam@vmware.com)
#

noinst_LTLIBRARIES = libvmeventcommon.la

libvmeventcommon_la_SOURCES = \
    globals.c \
    memory.c  \
    misc.c    \
    string.c  \
    utils.c

libvmeventcommon_la_CPPFLAGS = \
    -I$(top_srcdir)/vmevent/include \
    -I$(top_srcdir)/vmevent/include/public \
    @LW_INCLUDES@ \
    @OPENSSL_INCLUDES@


libvmeventcommon_la_LIBADD = \
    @LWADVAPI_LIBS@ \
    @LWBASE_LIBS@ \
    @LWRSUTILS_LIBS@ \
    @LWMSG_LIBS@ \
    @LWREG_LIBS@

libvmeventcommon_la_LDFLAGS = \
    -static \
    @LW_LDFLAGS@
