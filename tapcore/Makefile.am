BUILDDIR_INCLUDE_PATH = @top_build_prefix@/include
PRIVATE_INCLUDE_PATH = @abs_top_srcdir@/include
PUBLIC_INCLUDE_PATH = @abs_top_srcdir@/include/public
LIBTAPSTRUCT=@top_build_prefix@/tapstruct/libtapstruct.la
LIBTAPIO=@top_build_prefix@/tapio/libtapio.la -lm

AM_CFLAGS = -Wall -Werror
AM_CPPFLAGS = -I $(BUILDDIR_INCLUDE_PATH) \
              -I $(PRIVATE_INCLUDE_PATH) \
              -I $(PUBLIC_INCLUDE_PATH)

include_HEADERS = $(PUBLIC_INCLUDE_PATH)/tap.h

lib_LTLIBRARIES = libuniTesTap.la
libuniTesTap_la_SOURCES = tap.c testrun.c
libuniTesTap_la_LIBADD = $(LIBTAPSTRUCT) $(LIBTAPIO)

SUBDIRS = tests
