AC_PREREQ([2.69])
AC_INIT([lightwave], [1.3.0])
AC_CONFIG_AUX_DIR([.])
AM_INIT_AUTOMAKE([-Wall -Werror foreign subdir-objects])
AM_PROG_AR
LT_INIT
AC_PROG_CC
AC_CONFIG_FILES([Makefile])

AC_CONFIG_SUBDIRS([vmevent
                   vmdir
                   lwraft
                   vmdns
                   vmafd
                   vmca
                   vmidentity
                   config
                  ])

AC_SUBST(STAGEDIR)

AC_OUTPUT
