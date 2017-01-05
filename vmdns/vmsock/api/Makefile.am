#
# Copyright (c) VMware Inc.  All rights Reserved.
#

lib_LTLIBRARIES = libvmsock.la

libvmsock_la_SOURCES = \
    api.c     \
    globals.c \
    libmain.c

libvmsock_la_CPPFLAGS = \
    -I$(top_srcdir)/include \
    -I$(top_srcdir)/include/public \
    -I$(top_srcdir)/vmsock/include \
    -DHAVE_ARPA_INET_H \
    @LW_INCLUDES@

libvmsock_la_LDFLAGS = \
    -static \
    @LW_LDFLAGS@

libvmsock_la_LIBADD = \
    $(top_builddir)/vmsock/posix/libvmsockposix.la \
    @LWBASE_LIBS@

