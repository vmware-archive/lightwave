#!/bin/sh

# Defaults: should be empty strings by default
#
#
#force_debug="-g -O0"
#enable_winjoin="--enable-winjoin=yes"

usage()
{
  if [ -n "$1" ]; then
    echo "$1"
  fi
  echo "usage: $0 [--force-debug] [--enable-winjoin]"
  exit 1
}

while [ `echo "$1" | grep -c '^-'` -gt 0 ]; do
  if [ \( " $1" = " --help" \) -o \( " $1" = " -h" \) ]; then
    usage "" 
  elif [ " $1" = " --force-debug" ]; then
    force_debug="-g -O0"
    shift
  elif [ " $1" = " --enable-winjoin" ]; then
    enable_winjoin="--enable-winjoin=yes"
    shift
  else
    usage "ERROR: unknown option $1" 
  fi
done

if [ \( -n "$force_debug" \) -o \( -n "$enable_winjoin" \) ]; then
  if [ -f ".build_photon_opts" ]; then
    echo "NOTICE: Overriding saved build options from '.build_photon_opts' file"
    rm -f .build_photon_opts
  fi
  echo "force_debug=\"$force_debug\""       >> .build_photon_opts
  echo "enable_winjoin=\"$enable_winjoin\"" >> .build_photon_opts
elif [ -f ".build_photon_opts" ]; then
  echo "NOTICE: Using Overriding saved build options from '.build_photon_opts' file"
  . "./.build_photon_opts"
  echo force_debug=$force_debug
  echo enable_winjoin=$enable_winjoin
fi

# Determine if system is photon 1 or 2
photon_ver_string=`cat /etc/issue | awk '{print $4}'`
if [ "$photon_ver_string" = "2.0" ]; then
  photon_ver=2
elif [ "$photon_ver_string" = "1.0" ]; then
  photon_ver=1
else
  echo "ERROR: unable to determine photon OS build type"
  exit 1
fi

# Fix up lightwave.spec file based on photon version
echo "NOTICE: building photon version '$photon_ver'"
if [ $photon_ver -eq 2 ]; then
  if [ ! -s "package/rpm/lightwave.spec.orig" ]; then
    echo "NOTICE: patching lightwave.spec file for photon version '$photon_ver'"
    mv "package/rpm/lightwave.spec" "package/rpm/lightwave.spec.orig"
    cat package/rpm/lightwave.spec.orig | \
      sed -e 's|^BuildRequires: openjdk|BuildRequires: openjdk8|' \
          -e 's|1\.60\.0|1.63.0|' >  package/rpm/lightwave.spec
  fi
fi

IFS=%

autoreconf -vif .. \
  && \
../configure \
    CFLAGS="$force_debug -Wall -Werror -Wno-unused-but-set-variable -Wno-pointer-sign -Wno-implicit-function-declaration -Wno-address -Wno-enum-compare" \
    LDFLAGS=-ldl \
    --prefix=/opt/vmware \
    --libdir=/opt/vmware/lib64 \
    --localstatedir=/var/lib/vmware \
    --with-config=./config \
    $enable_winjoin \
  && \
 make \
  && \
 make package

if [ $# -eq 1 ];then
    if [ $1 = "--with-ui" ];then
       make -C ../ui
       cp ../ui/lwraft-ui/stage/RPMS/x86_64/*.rpm rpmbuild/RPMS/x86_64/
       cp ../ui/lightwave-ui/stage/RPMS/x86_64/*.rpm rpmbuild/RPMS/x86_64/
    fi
fi
