#!/bin/sh

# Avoid errors by disallowing expansions of unset variables
set -u

# Constants
YES='yes'

# Autoconf aux and m4 will always go in <project-root>/build
AUX_DIR="build/autotools/aux"
M4_DIR="build/autotools/m4"
PROJECT_ROOT=${0%autogen.sh}
CACHE=.autogen_cache

# autogen arguments
BUILD_DIR=
INSTALL=
VERBOSE=
CHECK=
CLEAN=
QUIET=
HELP=

# Cache arguments
CACHED_BUILD_DIR=

# Global state
PREFIX=/usr/local
STDOUT=1

abs_path() (
    IN_PATH=$1

    if [ "${IN_PATH#/}" = "$IN_PATH" ]; then
        IN_PATH="$PROJECT_ROOT/$IN_PATH"
    fi

    DIR=$(dirname "$IN_PATH")
    BASE=$(basename "$IN_PATH")

    # Resolve a path even if the end dir doesn't exist
    cd "$DIR"
    printf '%s/%s' "$(pwd)" "$BASE"
)

help() {
    cat <<eof
usage: autogen.sh [-h|OPTION]... [builddir]
Build uniTesTap

positional arguments:
  builddir                   The root of the build tree. (default: <project-root>/build)

options:
  -h, --help                 show this help message and exit
  -q, --quiet                suppress all normal output
  -v, --verbose              print steps of execution
  --check                    run unit tests
  --clean                    re-build everything
  --install [PREFIX]         Install library in PREFIX (default: /usr/local)
eof
}

argparse() {
    N_POSITIONAL_ARGS=0
    OPTION_ARG=

    while [ $# -gt 0 ]; do
        case "$1" in
            -h | --help)
                HELP=$YES
                ;;
            -v | --verbose)
                VERBOSE=$YES
                ;;
            -q | --quiet)
                QUIET=$YES
                ;;
            --check)
                CHECK=$YES
                ;;
            --clean)
                CLEAN=$YES
                ;;
            --install)
                OPTION_ARG=--install
                INSTALL=$YES
                ;;
            -*)
                printf 'Unknown option %s\n' "$1" >&2
                return 1
                ;;
            *)
                if [ -n "$OPTION_ARG" ]; then
                    case "$OPTION_ARG" in
                        --install)
                            PREFIX="$1"
                            ;;
                        *)
                            printf 'Unknown option arg %s\n' "$OPTION_ARG" >&2
                            return 1
                            ;;
                    esac
                    OPTION_ARG=
                elif [ $N_POSITIONAL_ARGS -eq 0 ]; then
                    BUILD_DIR="$1"
                else
                    printf 'Unknown positional argument: %s' "$1" >&2
                fi
                N_POSITIONAL_ARGS=$((N_POSITIONAL_ARGS + 1))
                ;;
        esac
        shift
    done

    [ $N_POSITIONAL_ARGS -le 1 ]
}

read_confirmation() (
    read -r ANSWER
    case "$ANSWER" in
        [yY] | [yY][eE][sS]) ;;
        *) return 1 ;;
    esac
    return 0
)

cacheparse() {
    while read -r key value; do
        # Ignore any empty values
        if [ -z "$value" ]; then
            continue
        fi

        case "$key" in
            builddir)
                CACHED_BUILD_DIR=$value
                ;;
            *)
                printf 'Unknown key-pair in cache: (%s, %s)' "$key" "$value" >&2
                ;;
        esac
    done <"$CACHE"
}

cachewrite() {
    cat <<eof >"$CACHE"
builddir $(abs_path "$BUILD_DIR")
eof
}

get_autotools_var() {
    printf '@%s@' "$1" | (cd "$BUILD_DIR" && ./config.status --file=-)
}

make_() {
    CC=$(get_autotools_var "CC")
    # Try to generate compiler_commands.json for editors
    intercept-build \
        --cdb "$BUILD_DIR/compile_commands.json" \
        --use-cc "$CC" \
        --append \
        make ${VERBOSE+-d}
}

build() (
    # All build steps must be successful
    set -e

    mkdir -p "$BUILD_DIR"

    # Must be run from source directory
    cd "$PROJECT_ROOT"
    ABS_PROJECT_ROOT=$(pwd)

    mkdir -p "$AUX_DIR" "$M4_DIR"

    # Go looking for the tap-driver if it is missing
    if ! [ -e "$AUX_DIR/tap-driver.sh" ]; then
        if ! AUTOMAKE_LIBDIR=$(automake --print-libdir); then
            printf 'automake error: cannot find tap-driver.sh' >&2
        fi
        cp "$AUTOMAKE_LIBDIR/tap-driver.sh" "$AUX_DIR"
    fi

    autoreconf -i ${CLEAN+-f} ${VERBOSE+-v} 2>&1
    cd "$BUILD_DIR"

    # Tidy up files created by make and configure
    if [ "$CLEAN" = "$YES" ]; then
        make distclean || true # Makefile may not exist
    fi

    "$ABS_PROJECT_ROOT"/configure --prefix="$PREFIX"
    make_
)

check() (
    cd "$BUILD_DIR"
    make check
)

install() (
    cd "$BUILD_DIR"
    make install
)

if ! argparse "$@"; then
    IFS="$OLD_IFS"
    exit 1
fi

if [ "$QUIET" = "$YES" ]; then
    # Redirect all stdout if quiet,
    # keep stdout open on fd 3 for prompting
    exec 3<&1
    exec 1>/dev/null
    STDOUT=3
fi

if [ -e "$CACHE" ]; then
    cacheparse
fi

# Resolve BUILD_DIR to compare to CACHED_BUILD_DIR
if [ -n "$BUILD_DIR" ]; then
    BUILD_DIR=$(abs_path "$BUILD_DIR")
fi

# Check if the cached builddir is being overridden
if [ -n "$CACHED_BUILD_DIR" ] && [ "$BUILD_DIR" != "$CACHED_BUILD_DIR" ]; then
    if [ -n "$BUILD_DIR" ] && [ -e "$CACHED_BUILD_DIR" ]; then
        # shellcheck disable=SC3021
        printf 'builddir exists created at "%s".\n' "$CACHED_BUILD_DIR" >&$STDOUT
        # shellcheck disable=SC3021
        printf 'Create new builddir at "%s"?\n' "$BUILD_DIR" >&$STDOUT
        if ! read_confirmation; then
            exit 1
        fi
    fi
    BUILD_DIR=$CACHED_BUILD_DIR
fi

# Default value if no cmdline or cached builddir
if [ -z "$BUILD_DIR" ]; then
    BUILD_DIR=build
fi

# Write variables that need to persist between runs
cachewrite

if [ "$HELP" = "$YES" ]; then
    help
    exit 1
fi

if [ "$VERBOSE" = "$YES" ]; then
    set -x
fi

if ! build; then
    exit 1
fi

if [ "$CHECK" = "$YES" ]; then
    check
fi

if [ "$INSTALL" = "$YES" ]; then
    install
fi
