noinst_LTLIBRARIES = libvmkrb5.la

libvmkrb5_la_SOURCES = \
    authenticator.c    \
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
    -I$(top_srcdir)/include \
    -I$(top_srcdir)/include/public \
    -I$(top_srcdir)/server/include \
    -I$(top_srcdir)/server \
    -I$(top_srcdir)/thirdparty/heimdal \
    -I$(top_srcdir)/thirdparty/heimdal/krb5-crypto \
    -I$(top_srcdir)/thirdparty/heimdal/asn1 \
    @LW_INCLUDES@ \
    @OPENSSL_INCLUDES@

libvmkrb5_la_LDFLAGS = \
    -static
