INCLUDE_PATH=@top_build_prefix@/include

AM_CFLAGS = -Wall -Werror
AM_CPPFLAGS = -I $(INCLUDE_PATH)

include_HEADERS = $(INCLUDE_PATH)/tap.h

lib_LTLIBRARIES = libtap.la
libtap_la_SOURCES = tap.c

bin_PROGRAMS = tap.test
tap_test_SOURCES = test_tap.c
tap_test_LDADD = libtap.la
