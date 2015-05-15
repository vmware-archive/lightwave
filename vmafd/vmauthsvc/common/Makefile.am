#
# Copyright (c) VMware Inc.  All rights Reserved.
# Module Name: common Makefile
# Abstract: VMware Authsvc Service.
# Created on: April 15, 2013
# Author: Sanjay Jain (sanjain@vmware.com)
#         Sriram Nambakam (snambakam@vmware.com)
#

noinst_LTLIBRARIES = libvmauthsvccommon.la

libvmauthsvccommon_la_SOURCES = \
    logging.c \
    misc.c    \
    memory.c  \
    string.c  \
    utils.c

libvmauthsvccommon_la_CPPFLAGS = \
    -I$(top_srcdir)/vmauthsvc/include \
    -I$(top_srcdir)/vmauthsvc/include/public \
    @LW_INCLUDES@ \
    @OPENSSL_INCLUDES@

libvmauthsvccommon_la_LDFLAGS = \
    -static
