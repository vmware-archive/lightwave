bin_PROGRAMS = vdcbackup

vdcbackup_SOURCES = \
    main.c

vdcbackup_CPPFLAGS = \
    -I$(top_srcdir)/include \
    -I$(top_srcdir)/include/public \
    @DCERPC_INCLUDES@ \
    @LW_INCLUDES@ \
    @OPENSSL_INCLUDES@

vdcbackup_LDADD = \
    $(top_builddir)/common/libcommon.la \
    $(top_builddir)/client/liblwraftclient.la

vdcbackup_LDFLAGS = \
    @LW_LDFLAGS@
