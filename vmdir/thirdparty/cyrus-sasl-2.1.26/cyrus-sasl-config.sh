#!/bin/sh

usage()
{
  if [ -n "$1" ]; then
    echo "$1"
  fi
  echo "usage: $0 [--force-debug]"
  exit 1
}

top=$(cd `dirname $0` && pwd)
cd $top

while [ `echo "$1" | grep -c '^-'` -gt 0 ]; do
  if [ \( " $1" = " --help" \) -o \( " $1" = " -h" \) ]; then
    usage ""
  elif [ " $1" = " --force-debug" ]; then
    CDEBUGFLAGS='-g -O0 -fPIC'
    shift
  else
    usage "ERROR: unknown option $1"
  fi
done

autoreconf -i -f \
  -I$top/cmulocal \
  -I$top/config \
  -I$top/saslauthd/config


CFLAGS="$CDEBUGFLAGS"  ./configure --prefix=/ \
    --bindir=/usr/bin \
    --libdir=/usr/lib \
    --sysconfdir=/etc \
    --with-plugindir=/usr/lib/sasl2 \
    --without-dblib \
    --with-saslauthd=/run/saslauthd \
    --without-authdaemond \
    --disable-macos-framework \
    --disable-sample \
    --disable-digest \
    --disable-otp \
    --disable-plain \
    --disable-anon \
    --enable-srp \
    --enable-gss_mutexes \
    --disable-static \
    --enable-shared \
    --enable-fast-install \
    --enable-krb4 \
  &&
make


cp /usr/bin/libtool $top
