INCLUDE_PATH = @abs_top_srcdir@/include

AM_CFLAGS = -Wall -Werror
AM_CPPFLAGS = -I $(INCLUDE_PATH)

include_HEADERS = $(INCLUDE_PATH)/tap.h

lib_LTLIBRARIES = libtap.la
libtap_la_SOURCES = tap.c

check_PROGRAMS = tap.test
tap_test_SOURCES = test_tap.c
tap_test_LDADD = libtap.la

TEST_LOG_DRIVER = \
    env AM_TAP_AWK='@AWK@' @SHELL@ \
    @abs_top_srcdir@/build/autotools/aux/tap-driver.sh

TESTS = $(check_PROGRAMS)
EXTRA_DIST = $(check_PROGRAMS)
