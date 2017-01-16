thirdparty_srcdir = $(top_srcdir)/thirdparty
thirdparty_builddir = $(top_builddir)/thirdparty

noinst_PROGRAMS = \
    parsedb \
    ktdump \
    testcrypto

#noinst_PROGRAMS = \
#    parsedb \
#    ktdump \
#    testcrypto \
#    krb5keys-test

MIT_TOOLS_INCLUDES = \
    -I$(top_srcdir)/server/vmkdc \
    -I$(top_srcdir)/server/kdctools \
    -I$(top_srcdir)/include \
    -I$(top_srcdir)/include/public \
    -I$(top_srcdir)/server \
    -I$(top_srcdir)/server/tools \
    -I$(top_srcdir)/server/include \
    -I$(top_srcdir)/server/vmkrb5 \
    -I$(thirdparty_srcdir)/heimdal \
    -I$(thirdparty_srcdir)/heimdal/krb5-crypto \
    -I$(thirdparty_srcdir)/heimdal/asn1 \
    -I$(top_srcdir)/server/vmkdc \
    @LW_INCLUDES@ \
    @OPENSSL_INCLUDES@

MIT_TOOLS_LDADD = \
    $(top_builddir)/server/common/libsrvcommon.la \
    $(top_builddir)/kdccommon/libkdccommon.la \
    $(top_builddir)/server/kdctools/libvmkdctools.la \
    $(top_builddir)/server/kdckrb5/libvmkrb5.la \
    $(top_builddir)/kdccommon/libkdccommon.la \
    $(top_builddir)/common/libcommon.la \
    $(thirdparty_builddir)/heimdal/krb5-crypto/libkrb5crypto.la \
    $(thirdparty_builddir)/heimdal/asn1/libasn1db.la \
    $(thirdparty_builddir)/heimdal/asn1/libasn1.la \
    @CRYPTO_LIBS@ \
    @CRYPT_LIBS@ \
    @LWBASE_LIBS@ \
    @PTHREAD_LIBS@

MIT_TOOLS_LD_FLAGS = \
    @LW_LDFLAGS@ \
    @OPENSSL_LDFLAGS@

# # ---- krb5keys-test ----
# krb5keys_test_SOURCES = krb5keys-test.c
# krb5keys_test_CPPFLAGS = $(MIT_TOOLS_INCLUDES)
# krb5keys_test_LDADD = $(MIT_TOOLS_LDADD)
# krb5keys_test_LDFLAGS = $(MIT_TOOLS_LD_FLAGS)
# 
# ---- parsedb ----
parsedb_SOURCES = parsedb.c
parsedb_CPPFLAGS = $(MIT_TOOLS_INCLUDES)
parsedb_LDADD = $(MIT_TOOLS_LDADD)
parsedb_LDFLAGS = $(MIT_TOOLS_LD_FLAGS)

# ---- ktdump ----
ktdump_SOURCES = ktdump.c
ktdump_CPPFLAGS = $(MIT_TOOLS_INCLUDES)
ktdump_LDADD = $(MIT_TOOLS_LDADD)
ktdump_LDFLAGS = $(MIT_TOOLS_LD_FLAGS)

# ---- testcrypto ----
testcrypto_SOURCES = test-crypto.c
testcrypto_LDADD = $(MIT_TOOLS_LDADD)
testcrypto_CPPFLAGS = \
    -DHEIMDAL_SMALLER \
    $(MIT_TOOLS_INCLUDES)
testcrypto_LDFLAGS = $(MIT_TOOLS_LD_FLAGS)
