AC_PREREQ([2.65])

AC_INIT([clutterrific],
        [0.0.0],
        [huaw@mcmaster.ca],
        [clutterrific],
        [http://attente.ca/])

AM_INIT_AUTOMAKE

AC_CONFIG_SRCDIR([src/clutterrific.h])
AM_CONFIG_HEADER([src/config.h])

AC_PROG_CC
AC_PROG_INSTALL

AC_SUBST([LIBS], [-lm])

PKG_CHECK_MODULES([ODE], [ode >= 0.11.1])
AC_SUBST([ODE_CFLAGS])
AC_SUBST([ODE_LIBS])

PKG_CHECK_MODULES([RSVG], [librsvg-2.0 >= 2.26])
AC_SUBST([RSVG_CFLAGS])
AC_SUBST([RSVG_LIBS])

PKG_CHECK_MODULES([CLUTTER], [clutter-1.0 >= 1.3])
AC_SUBST([CLUTTER_CFLAGS])
AC_SUBST([CLUTTER_LIBS])

PKG_CHECK_MODULES([GNOME_SCREENSAVER], [gnome-screensaver >= 2.30])
AC_SUBST([GNOME_SCREENSAVER_PRIVLIBEXECDIR],
         [$(pkg-config --variable=privlibexecdir gnome-screensaver)])
AC_SUBST([GNOME_SCREENSAVER_THEMESDIR],
         [$(pkg-config --variable=themesdir gnome-screensaver)])

# checks for header files
# checks for types
# checks for structures
# checks for compiler characteristics
# checks for library functions
# checks for system services

AC_CONFIG_FILES([Makefile data/Makefile src/Makefile])
AC_OUTPUT
