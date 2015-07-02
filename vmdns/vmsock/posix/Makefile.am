#
# Copyright (c) VMware Inc.  All rights Reserved.
#

noinst_LTLIBRARIES = libvmsockposix.la

libvmsockposix_la_SOURCES = \
    globals.c \
    libmain.c \
    socket.c

libvmsockposix_la_CPPFLAGS = \
    -I$(top_srcdir)/include \
    -I$(top_srcdir)/include/public \
    -I$(top_srcdir)/vmsock/include \
    -DHAVE_SYS_EPOLL_H \
    -DHAVE_FCNTL_H \
    -DHAVE_ARPA_INET_H \
    @LW_INCLUDES@

libvmsockposix_la_LDFLAGS = \
    -static \
    @LWBASE_LIBS@ \
    @PTHREAD_LIBS@
