noinst_LTLIBRARIES = libvmkrb5.la

PACSOURCES =
if VMDIR_ENABLE_PAC
PACSOURCES += authzdata.c
endif

libvmkrb5_la_SOURCES = \
    authenticator.c    \
    $(PACSOURCES)      \
    asreq.c            \
    apreq.c            \
    checksum.c         \
    crypto.c           \
    data.c             \
    encdata.c          \
    enctypes.c         \
    kdcreq.c           \
    kdcrep.c           \
    key.c              \
    keyset.c           \
    krberror.c         \
    padata.c           \
    paenctsenc.c       \
    etypeinfo2.c       \
    principal.c        \
    tgsreq.c           \
    ticket.c

libvmkrb5_la_CPPFLAGS = \
    -DOPENSSL_NO_KRB5 \
    -I$(top_srcdir)/vmdir/include \
    -I$(top_srcdir)/vmdir/include/public \
    -I$(top_srcdir)/vmdir/server/include \
    -I$(top_srcdir)/vmdir/server \
    -I$(top_srcdir)/vmdir/thirdparty/heimdal \
    -I$(top_srcdir)/vmdir/thirdparty/heimdal/krb5-crypto \
    -I$(top_srcdir)/vmdir/thirdparty/heimdal/asn1 \
    -I$(top_srcdir)/vmcommon/include/public \
    @LW_INCLUDES@ \
    @OPENSSL_INCLUDES@

libvmkrb5_la_LDFLAGS = \
    -static
