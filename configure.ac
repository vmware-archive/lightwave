AC_PREREQ([2.69])
AC_INIT([lightwave], [1.3.0])
AC_CONFIG_AUX_DIR([.])
AM_INIT_AUTOMAKE([-Wall -Werror foreign subdir-objects])
AM_PROG_AR
AC_PROG_CC
AC_CONFIG_FILES([Makefile])

#
# append to ac_configure_args
#
ac_configure_args="$ac_configure_args --enable-server=yes"

ac_cv_header_vmdns_h = yes
ac_cv_lib_vmdnsclient_VmDnsOpenServerA = yes

AC_CONFIG_SUBDIRS([vmevent
                   vmdir
                   config
                   lwraft
                   vmdns
                   vmafd
                   vmca
                   vmidentity
                  ])
AC_OUTPUT
