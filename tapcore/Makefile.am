PRIVATE_INCLUDE_PATH = @abs_top_srcdir@/include
PUBLIC_INCLUDE_PATH = @abs_top_srcdir@/include/public
LIBTAPSTRUCT=@top_build_prefix@/tapstruct/libtapstruct.la
LIBTAPIO=@top_build_prefix@/tapio/libtapio.la

AM_CFLAGS = -Wall -Werror
AM_CPPFLAGS = -I $(PRIVATE_INCLUDE_PATH) -I $(PUBLIC_INCLUDE_PATH)

include_HEADERS = $(PUBLIC_INCLUDE_PATH)/tap.h

lib_LTLIBRARIES = libtap.la
libtap_la_SOURCES = tap.c
libtap_la_LIBADD = $(LIBTAPSTRUCT) $(LIBTAPIO)

check_PROGRAMS = tap.test
tap_test_SOURCES = test_tap.c
tap_test_LDADD = libtap.la

TEST_LOG_DRIVER = \
    env AM_TAP_AWK='@AWK@' @SHELL@ \
    @abs_top_srcdir@/build/autotools/aux/tap-driver.sh

TESTS = $(check_PROGRAMS)
EXTRA_DIST = $(check_PROGRAMS)