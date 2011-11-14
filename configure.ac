#httpdbusd
AC_PREREQ(2.53)
AC_INIT([httpdbusd], [0.1])
AC_CONFIG_SRCDIR([src])

HTTPDBUSD_DEPS="glib-2.0 gio-2.0 libsoup-2.4"

AM_INIT_AUTOMAKE([foreign -Wno-portability dist-bzip2])
AC_CONFIG_HEADERS([src/config.h])

AC_CONFIG_MACRO_DIR([m4])
AC_PROG_CC

AS_COMPILER_FLAGS([HTTPDBUSD_CFLAGS], [-Wall])

PKG_CHECK_MODULES([HTTPDBUSD_DEPS], [$HTTPDBUSD_DEPS])

AC_CONFIG_FILES([
  Makefile
  src/Makefile
])
AC_OUTPUT
