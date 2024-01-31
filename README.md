# uniTesTap

uniTesTap's (unit-test-tap) purpose is a standalone Test Anything Protocol (TAP) producer library for simplifying writing unit tests for automake.

* Test isolation via per process tests
* Concurrent test runs
* Plain c syntax for test functions
* Standalone library for reduced integration complexity

# Basic Usage

uniTesTap test functions can be written without calling any custom functions, only using uniTesTap functions for the test rigging, for example using the standard library <code>assert()</code> function:

    #include <assert.h>
    #include <tap.h>

    int test_plus() {
        assert(1 + 2 == 2);
        return 0;
    }

    int test_subtract() {
        assert(2 - 1 == 1);
        return 0;
    }

    int main(void) {
        tap_easy_register(test_plus, "Test adding works");
        tap_easy_register(test_subtract, "Test subtracting works");
        tap_easy_runall_and_cleanup();
    }

Oops, notice the error in <code>test_plus()</code>, this check will fail and terminate the test, via <code>SIGABRT</code>. Regardless due to uniTesTap's test isolation <code>test_subtract()</code> will continue running just fine, producing the output:

    1..2
    # test 1: example.test: <file.c>:5: test_plus: Assertion `1 + 2 == 2' failed.
    # test 1: terminated via Aborted(6)
    not ok 1 - Test adding works (95.6ms)
    ok 2 - Test subtracting works (90.6ms)

# Building uniTesTap

For quickstart and most usecases, executing

    ./autogen.sh

will build the project outside of the source tree in <code>./build</code>. For building and installing, execute

    ./autogen.sh --install

For further options check <code>./autogen --help</code>. If contributing to uniTesTap make sure to use the <code>--clean</code> and <code>--check</code> options of <code>autogen.sh</code>. For more complex use-cases, use the autotools toolset (e.g. <code>autoreconf</code>, <code>./configure</code>, and <code>make</code>).

# Contributors

Before submitting any patches, please run <code>./scripts/checkpatch.sh</code> and <code>./autogen.sh --check</code> over each commit. Feel free to open a discussion to communicate any feature ideas if you're unsure how to implement or fit it into the surrounding code.

# License

uniTesTap is licensed under [LGPLv3](COPYING.LESSER).
