PRIVATE_INCLUDE_PATH = @abs_top_srcdir@/include
PUBLIC_INCLUDE_PATH = @abs_top_srcdir@/include/public
LIBTAPSTRUCT=@top_build_prefix@/tapstruct/libtapstruct.la

AM_CFLAGS = -Wall -Werror
AM_CPPFLAGS = -I $(PRIVATE_INCLUDE_PATH) -I $(PUBLIC_INCLUDE_PATH)

lib_LTLIBRARIES = libtapio.la
libtapio_la_SOURCES = tap_parse.c tap_pipe.c tap_print.c
libtapio_la_LIBADD = $(LIBTAPSTRUCT)
