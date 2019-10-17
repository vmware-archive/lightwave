#
# Copyright (c) VMware Inc.  All rights Reserved.
#

noinst_LTLIBRARIES = libvmafposixcfg.la

libvmafposixcfg_la_SOURCES = \
    config.c \
    globals.c \
    libmain.c

libvmafposixcfg_la_CPPFLAGS = \
    -I$(top_srcdir)/vmcommon/include/public \
    -I$(top_srcdir)/vmafd/include \
    -I$(top_srcdir)/vmafd/include/public \
    -I$(top_srcdir)/vmafd/vmafcfg/include \
    @LW_INCLUDES@ \
    @OPENSSL_INCLUDES@

libvmafposixcfg_la_LDFLAGS = \
    -static \
    -I$(top_builddir)/vmcommon/libvmcommon.la \
    @LWRSUTILS_LIBS@ \
    @LWREG_LIBS@ \
    @LWBASE_LIBS@ \
    @PTHREAD_LIBS@
