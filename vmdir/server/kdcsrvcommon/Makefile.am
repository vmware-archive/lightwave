#
# Copyright (c) VMware Inc.  All rights Reserved.
# Module Name: common Makefile
# Abstract: VMware Kdc Service.
# Created on: July 10, 2012
# Author: Sanjay Jain (sanjain@vmware.com)
#         Sriram Nambakam (snambakam@vmware.com)
#

noinst_LTLIBRARIES = libkdcsrvcommon.la

idl_srcdir=$(top_srcdir)/idl

PAC_SOURCES =

if VMDIR_ENABLE_PAC
PAC_SOURCES += vmdir_pac_cstub.c
PAC_SOURCES += pacops.c

BUILT_SOURCES = vmdir_pac_h.h

# Required because the vmdir_pac_cstub.c is not going to pick up the definition from config.h
VMDIR_ENABLE_PAC_DEFINE=-DVMDIR_ENABLE_PAC

CLEANFILES = \
    vmdir_pac_h.h \
    vmdir_pac_cstub.c \
    vmdir_pac_sstub.c

vmdir_pac_cstub.c vmdir_pac_h.h: $(idl_srcdir)/vmdir_pac.idl $(idl_srcdir)/vmdir_pac.acf
	$(IDL) $(IDLFLAGS) -DVMDIR_ENABLE_PAC -keep c_source -header vmdir_pac_h.h -I$(idl_srcdir) -I$(top_srcdir)/include/public $<
endif

libkdcsrvcommon_la_SOURCES = \
    util.c \
    $(PAC_SOURCES)


libkdcsrvcommon_la_CPPFLAGS = \
    $(VMDIR_ENABLE_PAC_DEFINE) \
    -I$(top_srcdir)/include \
    -I$(top_srcdir)/include/public \
    -I$(top_srcdir)/server/include \
    -I. \
    -I$(top_builddir)/server/kdcsrvcommon \
    @DCERPC_INCLUDES@ \
    @LW_INCLUDES@ \
    @OPENSSL_INCLUDES@

libkdcsrvcommon_la_LDFLAGS = \
    -static
