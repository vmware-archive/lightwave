#
# Copyright (c) 2017 VMware Inc.  All rights Reserved.
# Module Name: common Makefile
# Abstract: VMware Authentication Framework Service.
# Created on: Jun 1, 2017
# Author: Aishu Raghavan (araghavan@vmware.com)
#

noinst_LTLIBRARIES = libvmnetevent.la

libvmnetevent_la_SOURCES = \
    connection.c \
    globals.c \
    linux_api.c \
    vmnetevent.c

libvmnetevent_la_CPPFLAGS = \
    -Wno-error=deprecated-declarations \
    -I$(top_srcdir)/include \
    -I$(top_srcdir)/include/public \
    @LW_INCLUDES@

libvmnetevent_la_LDFLAGS = \
    -static
