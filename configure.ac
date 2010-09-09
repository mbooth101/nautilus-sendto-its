dnl Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT(nautilus-sendto-trac, 0.1, http://www.matbooth.co.uk/trac/wiki/NautilusSendtoTracStart)


AC_CANONICAL_SYSTEM
AM_INIT_AUTOMAKE()

AM_DISABLE_STATIC
AC_PROG_CC
AC_PROG_LIBTOOL

NAUTILUS_SENDTO_REQUIRED=2.28.0
SOUP_REQUIRED=2.28.0

PKG_CHECK_MODULES(NAUTILUS_SENDTO, nautilus-sendto >= $NAUTILUS_SENDTO_REQUIRED)
PKG_CHECK_MODULES(SOUP, libsoup-2.4 >= $SOUP_REQUIRED)

AC_CONFIG_FILES(Makefile src/Makefile)
AC_OUTPUT

