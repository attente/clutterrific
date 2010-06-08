#!/bin/sh

CRAP="aclocal.m4
      autom4te.cache
      config.log
      config.status
      configure
      depcomp
      INSTALL
      install-sh
      Makefile
      Makefile.in
      missing
      src/.deps
      src/config.h
      src/config.h.in
      src/Makefile
      src/Makefile.in
      src/stamp-h1"

if [ "$1" = "clean" ]; then
  rm -rf ${CRAP}
  exit
fi

aclocal
autoconf
autoheader
automake --add-missing

./configure "$@"
