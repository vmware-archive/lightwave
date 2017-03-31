#
# Copyright (c) VMware Inc.  All rights Reserved.
# Module Name: common Makefile
# Abstract: VMware Authentication Framework Service.
# Created on: Feb 13, 2013
# Author: Sanjay Jain (sanjain@vmware.com)
#         Sriram Nambakam (snambakam@vmware.com)
#

noinst_LTLIBRARIES = libcommon.la

libcommon_la_SOURCES = \
    acl.c \
    buffer.c \
    certutil.c \
    fsapi.c \
    globals.c \
    logging.c \
    marshalutil.c \
    memory.c  \
    misc.c    \
    networkutiluniximpl.c \
    networkutil.c    \
    security-sd.c \
    securityutil.c   \
    securityutiluniximpl.c  \
    string.c  \
    ssl.c  \
    utils.c

libcommon_la_CPPFLAGS = \
    -I$(top_srcdir)/include \
    -I$(top_srcdir)/include/public \
    @LW_INCLUDES@ \
    @VMDIR_INCLUDES@ \
    @OPENSSL_INCLUDES@

libcommon_la_LDFLAGS = \
    @LDAP_LIBS@ \
    -static
