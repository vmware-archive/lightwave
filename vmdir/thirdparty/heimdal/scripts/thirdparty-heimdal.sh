#!/bin/sh
#
# Script to create minimal VMware distribution of lib/asn1 taken from
# Heimdal 1.5.2 distribution.
#
# This script is used to create the sources from a fresh Heimdal
# release, extract only the components needed to build lib/asn1, and
# apply appropriate patches needed to build asn1 and dependent components.
#
# This script was used to generate the sources that are now checked into 
# Perforce for lotus/main/vmkdc/thirdparty/heimdal-1.5.2.
#
# This can be used to recreate the VMware asn1 source tree from later
# Heimdal releases as needed.
#
# !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
# WARNING: autoreconf will fail if you don't have automake 1.10.3 installed
# !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
#
heimdal_src_root="."
heimdal_build_root="."
heimdal_vers="1.5.2"
options=0

if [ -n "$1" ]; then
  heimdal_src_root="$1"
  heimdal_build_root="$1" # not a typo!
  shift
  options=1
fi

if [ -n "$1" ]; then
  heimdal_build_root="$1"
  shift
  options=1
fi

heimdal_url="http://www.h5l.org/dist/src/heimdal-${heimdal_vers}.tar.gz"
heimdal_src="${heimdal_src_root}/heimdal-${heimdal_vers}.tar.gz"
heimdal_base="heimdal-${heimdal_vers}"
heimdal_root=$heimdal_build_root/$heimdal_base
pwd=`readlink -f $0 | sed 's|\(.*\)/.*|\1|'`
echo $pwd

if [ $options -eq 1 ]; then
  echo "heimdal_src=$heimdal_src"
  echo "heimdal_root=$heimdal_root"
  echo
  echo "Press <RETURN> to accept, otherwise ^C to quit"
  read line
fi

if [ ! -f "$heimdal_src" ]; then
  echo "Downloading $heimdal_src from '$heimdal_url'"
  echo wget $heimdal_url
  wget $heimdal_url
fi

tar_opt=zxf

FILES="Makefile.am
Makefile.am.common
config.guess
config.sub
configure.ac
depcomp
lib/Makefile.am
aclocal.m4
acinclude.m4
compile
missing
install-sh
ltmain.sh
ylwrap
NEWS
README
LICENSE
cf
include
lib/roken
lib/vers
lib/com_err
lib/asn1"

echo "Extracting components from $heimdal_src..."
(
cd $heimdal_build_root
for file in $FILES; do
  tar $tar_opt $heimdal_src ${heimdal_base}/$file
  [ $? -ne 0 ] && exit 1
done
)

(
cd $heimdal_build_root
patch -p2 ${heimdal_root}/Makefile.am < $pwd/patches/heimdal-root.patch
patch -p2 ${heimdal_root}/configure.ac < $pwd/patches/heimdal-root-configureac.patch
patch -p2 ${heimdal_root}/lib/Makefile.am < $pwd/patches/heimdal-lib.patch
patch -p2 ${heimdal_root}/include/Makefile.am < $pwd/patches/heimdal-include-makefileam.patch
)

echo "Running autoreconf..."
(cd ${heimdal_root} && autoreconf)

echo "Done."
echo "Build, run the following commands:"
echo
echo "cd $heimdal_root; ./configure; make"
