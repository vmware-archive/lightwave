lib_LTLIBRARIES = libgssapi_ntlm.la

libgssapi_ntlm_la_CPPFLAGS = \
   -D_MIT_KRB5_1_11 \
   -D_MIT_KRB5_1_12 \
   -I$(top_srcdir)/gssapi-plugins/ntlm \
   -I$(top_srcdir)/include \
   -I$(top_srcdir)/include/public \
   -I$(top_srcdir)/thirdparty \
    @OPENSSL_INCLUDES@ \
    @LW_INCLUDES@

libgssapi_ntlm_la_SOURCES = \
    gssapi_alloc.c \
    ntlm_accept_sec_ctx.c \
    ntlm_acquire_cred.c \
    ntlm_release_cred.c \
    ntlm_disp_name.c \
    ntlm_encrypt.c \
    ntlm_init_sec_ctx.c \
    ntlm_del_sec_ctx.c \
    ntlm_mech.c \
    ntlm_wrap_iov.c \
    ntlm_unwrap_iov.c \
    ntlm_util.c

libgssapi_ntlm_la_LIBADD = \
    $(top_builddir)/client/libvmdirclient.la \
    $(top_builddir)/thirdparty/csrp/libcsrp.la \
    @DCERPC_LIBS@ \
    @GSSAPI_LIBS@ \
    @LDAP_LIBS@ \
    @OPENSSL_LDFLAGS@ \
    @PTHREAD_LIBS@

libgssapi_ntlm_la_LDFLAGS = \
    @LW_LDFLAGS@
