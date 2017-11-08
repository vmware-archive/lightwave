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

IFS=%

autoreconf -vif .. \
  && \
../configure \
    CFLAGS="$force_debug -Wall -Werror -Wno-unused-but-set-variable -Wno-pointer-sign -Wno-implicit-function-declaration -Wno-address -Wno-enum-compare" \
    --prefix=/opt/vmware \
    --libdir=/opt/vmware/lib64 \
    --localstatedir=/var/lib/vmware \
    --with-config=./config \
    $enable_winjoin \
  && \
 make \
  && \
 make package
