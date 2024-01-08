AC_INIT([uniTesTap],[0.0.1],[coledishington@gmail.com])
AC_CONFIG_SRCDIR([tapcore/tap.c])
AC_CONFIG_AUX_DIR([build/autotools/aux])
AC_CONFIG_MACRO_DIR([build/autotools/m4])
AC_CONFIG_MACRO_DIRS([build/autotools/m4])
AC_CONFIG_HEADERS([include/config.h])

# TAP driver
AC_REQUIRE_AUX_FILE([tap-driver.sh])

# Check for compiler characteristics
AM_PROG_AR
AC_PROG_AWK
AC_PROG_CC
AC_PROG_SED

OPT_WITH_VALGRIND=yes
AC_ARG_WITH(
    [valgrind],
    [AS_HELP_STRING([--with-valgrind], [Test with valgrind toolset (default is yes)])],
    [OPT_WITH_VALGRIND=$withval])
AS_IF([test "$OPT_WITH_VALGRIND" = 'yes'], [
    AC_CHECK_PROG([VALGRIND], [valgrind], [valgrind], [])
])

AC_DEFINE([_POSIX_C_SOURCE], [200809L], [Support newer posix definitions with glibc])

AM_INIT_AUTOMAKE([-Wall -Werror foreign])
LT_INIT

AC_CONFIG_FILES([
    Makefile
    tapstruct/Makefile
    tapio/Makefile
    tapcore/Makefile
    tapcore/tests/Makefile
])
AC_OUTPUT
