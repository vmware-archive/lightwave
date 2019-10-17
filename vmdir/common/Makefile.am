#
# Copyright (c) VMware Inc.  All rights Reserved.
# Module Name: common Makefile
# Abstract: VMware Cloud Directory Platform.
# Created on: Oct 4, 2011
# Author: Sanjay Jain (sanjain@vmware.com)
#         Sriram Nambakam (snambakam@vmware.com)
#

noinst_LTLIBRARIES = libcommon.la

libcommon_la_SOURCES = \
    circularbuffer.c \
    deque.c \
    krbmisc.c \
    krbutil.c \
    ldapbind.c \
    ldapcontrol.c \
    ldaputil.c \
    linkedlist.c \
    logging.c \
    marshalutil.c \
    memory.c  \
    mergesort.c \
    networkutil.c \
    networkutiluniximpl.c \
    opensslclient.c  \
    opensslutil.c \
    parsearguments.c \
    queue.c \
    regutil.c  \
    rpcmemory.c  \
    securityutil.c \
    securityutiluniximpl.c \
    sortedlinkedlist.c \
    string.c  \
    stringlist.c \
    superlogutil.c \
    synchronizing.c  \
    threadcontext.c  \
    threading.c  \
    tsstack.c \
    util.c \
    replstate/schemareplstate.c \
    schema/compat.c \
    schema/copy.c \
    schema/def.c \
    schema/deflist.c \
    schema/diff.c \
    schema/file.c \
    schema/globals.c \
    schema/load.c \
    schema/merge.c \
    schema/parse.c \
    schema/patch.c \
    schema/resolve.c \
    schema/schema.c \
    schema/util.c \
    schema/verify.c

#TODO: Remove -DLDAP_DEPRECATED after fixing ldap_value_free call
libcommon_la_CPPFLAGS = \
    -DOPENSSL_NO_KRB5 \
    -DLDAP_DEPRECATED \
    -I$(top_srcdir)/vmcommon/include/public \
    -I$(top_srcdir)/vmdir/include \
    -I$(top_srcdir)/vmdir/include/public \
    -I$(top_srcdir)/vmdir/thirdparty/heimdal \
    -I$(top_srcdir)/vmdir/thirdparty/heimdal/krb5-crypto \
    -I$(top_srcdir)/vmdir/thirdparty/heimdal/asn1 \
    @OPENSSL_INCLUDES@ \
    @LW_INCLUDES@

libcommon_la_LDFLAGS = \
    -static \
    $(top_builddir)/vmcommon/libvmcommon.la \
    @OPENSSL_LDFLAGS@
