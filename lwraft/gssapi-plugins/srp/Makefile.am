lib_LTLIBRARIES = libgssapi_srp.la

libgssapi_srp_la_CPPFLAGS = \
   -D_MIT_KRB5_1_11 \
   -D_MIT_KRB5_1_12 \
   -I. \
   -I$(top_srcdir)/gssapi-plugins/srp \
   -I$(top_srcdir)/include \
   -I$(top_srcdir) \
   -I$(top_srcdir)/include/public \
   -I$(top_srcdir)/thirdparty \
   -I$(top_builddir)/client \
    @OPENSSL_INCLUDES@ \
    @LW_INCLUDES@

libgssapi_srp_la_SOURCES = \
        gssapi_alloc.c \
	srp_accept_sec_ctx.c \
	srp_acquire_cred.c \
	srp_release_cred.c \
	srp_disp_name.c \
	srp_encrypt.c \
	srp_init_sec_ctx.c \
	srp_del_sec_ctx.c \
	srp_mech.c \
	srp_mech_desc_srp10.c \
	srp_wrap_iov.c \
	srp_unwrap_iov.c \
	srp_util.c \
        srpregutils.c \
        srpreg.c \
        srprpc.c

libgssapi_srp_la_LIBADD = \
    $(top_builddir)/client/liblwraftclient.la \
    $(top_builddir)/common/libcommon.la \
    $(top_builddir)/thirdparty/csrp/libcsrp.la \
    $(top_builddir)/client/liblwraftclient_la-srp_verifier_cstub.lo \
    @DCERPC_LIBS@ \
    @GSSAPI_LIBS@ \
    @LDAP_LIBS@ \
    @OPENSSL_LDFLAGS@ \
    @PTHREAD_LIBS@

libgssapi_srp_la_LDFLAGS = \
    @LW_LDFLAGS@
