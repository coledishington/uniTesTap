PRIVATE_INCLUDE_PATH = @abs_top_srcdir@/include
PUBLIC_INCLUDE_PATH = @abs_top_srcdir@/include/public
LIBTAPSTRUCT=@top_build_prefix@/tapstruct/libtapstruct.la

AM_CFLAGS = -Wall -Werror
AM_CPPFLAGS = -I $(PRIVATE_INCLUDE_PATH) -I $(PUBLIC_INCLUDE_PATH)

noinst_LTLIBRARIES = libtapio.la
libtapio_la_SOURCES = tap_parse.c tap_pipe.c tap_print.c

check_PROGRAMS = tap_parse.test

tap_parse_test_SOURCES = test_tap_parse.c
tap_parse_test_LDADD = libtapio.la $(LIBTAPSTRUCT)

TEST_LOG_DRIVER = \
    env AM_TAP_AWK='@AWK@' @SHELL@ \
    @abs_top_srcdir@/build/autotools/aux/tap-driver.sh

TESTS = $(check_PROGRAMS)
EXTRA_DIST = $(check_PROGRAMS)
