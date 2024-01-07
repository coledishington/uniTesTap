BUILDDIR_INCLUDE_PATH = @top_build_prefix@/include
PRIVATE_INCLUDE_PATH = @abs_top_srcdir@/include
PUBLIC_INCLUDE_PATH = @abs_top_srcdir@/include/public

AM_CFLAGS = -Wall -Werror
AM_CPPFLAGS = -I $(BUILDDIR_INCLUDE_PATH) \
              -I $(PRIVATE_INCLUDE_PATH) \
              -I $(PUBLIC_INCLUDE_PATH)

noinst_LTLIBRARIES = libtapstruct.la
libtapstruct_la_SOURCES = tap_cmd.c tap_string.c
