#
# Copyright (c) VMware Inc.  All rights Reserved.
# Module Name: common Makefile
# Abstract: VMware Kdc Service.
# Created on: July 10, 2012
# Author: Sanjay Jain (sanjain@vmware.com)
#         Sriram Nambakam (snambakam@vmware.com)
#

noinst_LTLIBRARIES = libkdccommon.la

libkdccommon_la_SOURCES = \
    memory.c  \
    string.c  \
    security-sd.c

libkdccommon_la_CPPFLAGS = \
    -I$(top_srcdir)/include \
    -I$(top_srcdir)/include/public \
    @LW_INCLUDES@ \
    @OPENSSL_INCLUDES@

libkdccommon_la_LDFLAGS = \
    -static
