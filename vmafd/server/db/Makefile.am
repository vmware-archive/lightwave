
lib_LTLIBRARIES = libvecsdb.la

libvecsdb_la_CPPFLAGS = \
    -DLDAP_DEPRECATED \
    -I$(top_srcdir)/include \
    -I$(top_srcdir)/include/public \
    -I$(top_srcdir)/server/include \
    @SQLITE_INCLUDES@ \
    @LW_INCLUDES@ \
    @OPENSSL_INCLUDES@

libvecsdb_la_SOURCES = \
    cdcdbutil.c   \
		authdbutil.c   \
    database.c     \
    dbcontext.c    \
    globals.c      \
    libmain.c		\
    regdbutil.c  \
	vecsdbutil.c

libvecsdb_la_LIBADD = \
    @top_builddir@/common/libcommon.la \
    @SQLITE_LIBS@ \
    @LWBASE_LIBS@ \
    @PTHREAD_LIBS@ \
    @LDAP_LIBS@ \
    @LBER_LIBS@


libvecsdb_la_LDFLAGS = \
    -DLDAP_DEPRECATED \
    @SQLITE_LDFLAGS@ \
    @LW_LDFLAGS@ \
    -static \
    @LDAP_LDFLAGS@

