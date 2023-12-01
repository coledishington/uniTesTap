AC_INIT([uniTesTap],[0.0.1],[coledishington@gmail.com])
AC_CONFIG_SRCDIR([src/tap.c])
AC_CONFIG_AUX_DIR([build/autotools/aux])
AC_CONFIG_MACRO_DIR([build/autotools/m4])
AC_CONFIG_MACRO_DIRS([build/autotools/m4])
AC_CONFIG_HEADERS([include/config.h])

# Check for compiler characteristics
AM_PROG_AR
AC_PROG_CC

AM_INIT_AUTOMAKE([-Wall -Werror foreign])
LT_INIT

AC_CONFIG_FILES([
    Makefile
    src/Makefile
])
AC_OUTPUT
