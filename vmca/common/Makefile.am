#
# Copyright (c) VMware Inc.  All rights Reserved.
# Module Name: common Makefile
# Abstract: VMware Certificate Server Platform.
#

noinst_LTLIBRARIES = libcommon.la

libcommon_la_SOURCES = \
    certutils.c \
    config.c \
    config-posix.c \
    fsutils.c   \
    globals.c   \
    memory.c    \
    pkcs_openssl.c \
    ldap.c \
    string.c \
    misc.c \
    pkcs_crl.c \
    logging.c

libcommon_la_CPPFLAGS = \
    -DLDAP_DEPRECATED \
    -I$(top_srcdir)/include \
    -I$(top_srcdir)/include/public \
    @LW_INCLUDES@ \
    @VMAFD_INCLUDES@ \
    @OPENSSL_INCLUDES@

libcommon_la_LIBADD = \
    @LDAP_LIBS@ \
    @LBER_LIBS@ \
    @VMAFD_LIBS@ \
    @LWREG_LIBS@ \
    @LWRSUTILS_LIBS@

libcommon_la_LDFLAGS = \
    -DLDAP_DEPRECATED \
    -static \
    @VMAFD_LDFLAGS@ \
    @OPENSSL_LDFLAGS@
