lib_LTLIBRARIES = libcommonunittests.la

libcommonunittests_la_CPPFLAGS = \
    -I$(top_srcdir)/include \
    -I$(top_srcdir)/include/public \
    @OPENSSL_INCLUDES@ \
    @LW_INCLUDES@

libcommonunittests_la_SOURCES = \
    circularbuffer.c \
    deque.c \
    main.c \
    parseargs.c \
    registry.c \
    stringlist.c \
    VmDirGetVmDirLogPath.c \
    VmDirAllocateStringA.c \
    VmDirAllocateStringOfLenA.c \
    VmDirAllocateStringPrintf.c \
    VmDirStringCpyA.c \
    VmDirStringNCpyA.c \
    VmDirStringCatA.c \
    VmDirStringNCatA.c \
    VmDirReadString.c \
    VmDirStringToTokenList.c

libcommonunittests_la_LIBADD = \
    @top_builddir@/testing/test_lib/libvmdirtesting.la \
    @DCERPC_LIBS@ \
    @LWIO_LIBS@ \
    @SCHANNEL_LIBS@ \
    @LWMSG_LIBS@ \
    @LWRSUTILS_LIBS@ \
    @LWREG_LIBS@ \
    @LWBASE_LIBS@ \
    @GSSAPI_LIBS@ \
    @UUID_LIBS@ \
    @LDAP_LIBS@ \
    @LBER_LIBS@ \
    @CRYPTO_LIBS@ \
    @PTHREAD_LIBS@

libcommonunittests_la_LDFLAGS = \
    -export-symbols @top_srcdir@/testing/unittests/libcommon/libcommonunittests.exp \
    @DCERPC_LDFLAGS@ \
    @OPENSSL_LDFLAGS@ \
    @LW_LDFLAGS@
