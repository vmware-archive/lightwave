SUBDIRS = \
    regcfg

bin_PROGRAMS = lw-cli

lw_cli_SOURCES = \
    main.c

lw_cli_CPPFLAGS = \
    -I$(top_srcdir)/vmcommon/include/public \
    -I$(top_srcdir)/vmcommon/include \
    @LW_INCLUDES@

lw_cli_LDADD = \
    @top_builddir@/vmcommon/libvmcommon.la \
    @top_builddir@/vmcommon/tools/lw-cli/regcfg/libregcfgcli.la \
    @PTHREAD_LIBS@

lw_cli_LDFLAGS = \
    @LW_LDFLAGS@ \
    @OPENSSL_LDFLAGS@
