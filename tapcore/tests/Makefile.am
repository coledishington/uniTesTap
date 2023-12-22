PUBLIC_INCLUDE_PATH = @abs_top_srcdir@/include/public

# Each test imports internal.h that contains static re-usable functions but
# all functions will be defined for every test. Allow unused-function for only
# these tests.
AM_CFLAGS = -Wall -Werror -Wno-error=unused-function
AM_CPPFLAGS = -I $(PUBLIC_INCLUDE_PATH)

TESTPLAN_TESTS = \
    test_testplan_fail \
    test_testplan_pass \
    test_testplan_pass_fail \
    test_testplan_pass_pass \
    test_testplan_100

check_PROGRAMS = \
    $(TESTPLAN_TESTS) \
    test_early_exit \
    test_cmd \
    test_metadata

LDADD = ../libtap.la

TEST_LOG_DRIVER = \
    env AM_TAP_AWK='@AWK@' @SHELL@ \
    @abs_top_srcdir@/build/autotools/aux/tap-driver.sh

TESTS = test_tap.test$(EXEEXT)
EXTRA_DIST = $(check_PROGRAMS)

# Custom make targets
test_tap = test_tap.test
sh_subst = \
	@SED@ -e 's,[@]SHELL[@],@SHELL@,g' \
          -e 's,[@]abs_srcdir[@],@abs_srcdir@,g' \
          -e 's,[@]LIBTOOL[@],@LIBTOOL@,g' \
          -e 's,[@]SED[@],@SED@,g' \
          -e 's,[@]VALGRIND[@],@VALGRIND@,g'
$(test_tap)$(EXEEXT): $(test_tap).in Makefile
	$(sh_subst) < @abs_srcdir@/$(test_tap).in > $(test_tap)$(EXEEXT)
	chmod +x $(test_tap)$(EXEEXT)

check-local: $(test_tap)$(EXEEXT)
