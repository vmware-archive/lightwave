#
# Copyright (c) VMware Inc.  All rights Reserved.
#

lib_LTLIBRARIES = libvmafcfgapi.la

#remove after top level configure.ac sets the desired flags by default
AM_CFLAGS=$(CFLAGS_DESIRED)

libvmafcfgapi_la_SOURCES = \
    config.c \
    globals.c \
    libmain.c

libvmafcfgapi_la_CPPFLAGS = \
    -I$(top_srcdir)/vmafd/include \
    -I$(top_srcdir)/vmafd/include/public \
    -I$(top_srcdir)/vmafd/vmafcfg/include \
    @LW_INCLUDES@ \
    @OPENSSL_INCLUDES@

libvmafcfgapi_la_LDFLAGS = \
    @LW_LDFLAGS@

libvmafcfgapi_la_LIBADD = \
    $(top_builddir)/vmafd/vmafcfg/posix/libvmafposixcfg.la \
    @LWBASE_LIBS@
