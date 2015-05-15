#
# Copyright (c) VMware Inc.  All rights Reserved.
#

lib_LTLIBRARIES = libvmafcfgapi.la

libvmafcfgapi_la_SOURCES = \
    config.c  \
    globals.c \
    libmain.c

libvmafcfgapi_la_CPPFLAGS = \
    -I$(top_srcdir)/include \
    -I$(top_srcdir)/include/public \
    -I$(top_srcdir)/vmafcfg/include \
    @LW_INCLUDES@ \
    @OPENSSL_INCLUDES@

libvmafcfgapi_la_LDFLAGS = \
    @LW_LDFLAGS@

libvmafcfgapi_la_LIBADD = \
    $(top_builddir)/vmafcfg/posix/libvmafposixcfg.la \
    @LWBASE_LIBS@

