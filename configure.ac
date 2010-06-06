AC_PREREQ([2.65])

AC_INIT([clutterrific],
        [0.0.0],
        [huaw@mcmaster.ca],
        [clutterrific],
        [http://attente.ca/])

AC_CONFIG_SRCDIR([src/clutterrific.h])
AM_INIT_AUTOMAKE
AM_CONFIG_HEADER([src/config.h])

AC_PROG_CC
AC_PROG_INSTALL

PKG_CHECK_MODULES([CLUTTER], [clutter-1.0 >= 1.3])
AC_SUBST(CLUTTER_CFLAGS)
AC_SUBST(CLUTTER_LIBS)

# checks for header files
# checks for types
# checks for structures
# checks for compiler characteristics
# checks for library functions
# checks for system services

AC_CONFIG_FILES([Makefile src/Makefile])
AC_OUTPUT
