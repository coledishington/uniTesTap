#!/bin/sh

# Autoconf aux and m4 will always go in <project-root>/build
AUX_DIR="build/autotools/aux"
M4_DIR="build/autotools/m4"
PROJECT_ROOT=${0%autogen.sh}
CACHE=.autogen_cache

# autogen arguments
BUILD_DIR=
VERBOSE=
HELP=

# Cache arguments
CACHED_BUILD_DIR=

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
  -v, --verbose              print steps of execution
eof
}

argparse() {
    N_POSITIONAL_ARGS=0

    while [ $# -gt 0 ]; do
        case "$1" in
            -h | --help)
                HELP='yes'
                shift
                ;;
            -v | --verbose)
                VERBOSE='yes'
                shift
                ;;
            -*)
                printf 'Unknown option %s\n' "$1" >&2
                return 1
                ;;
            *)
                if [ $N_POSITIONAL_ARGS -eq 0 ]; then
                    BUILD_DIR="$1"
                else
                    printf 'Unknown positional argument: %s' "$1" >&2
                fi
                N_POSITIONAL_ARGS=$((N_POSITIONAL_ARGS + 1))
                shift
                ;;
        esac
    done

    [ $N_POSITIONAL_ARGS -le 1 ]
}

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

build() (
    # All build steps must be successful
    set -e

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

    autoreconf -if ${VERBOSE+-v}
    cd "$BUILD_DIR"
    "$ABS_PROJECT_ROOT"/configure
    make ${VERBOSE+-d}
)

if ! argparse "$@"; then
    exit 1
fi

if [ -e "$CACHE" ]; then
    cacheparse
fi

# Resolve BUILD_DIR to compare to CACHED_BUILD_DIR
if [ -n "$BUILD_DIR" ]; then
    BUILD_DIR=$(abs_path "$BUILD_DIR")
fi

# Check if the cached builddir is being overridden
if [ -n "$CACHED_BUILD_DIR" ]; then
    if [ -n "$BUILD_DIR" ] && [ "$BUILD_DIR" != "$CACHED_BUILD_DIR" ] && [ -e "$CACHED_BUILD_DIR" ]; then
        printf 'builddir exists created at "%s".\n' "$CACHED_BUILD_DIR"
        printf 'Create new builddir at "%s"?\n' "$BUILD_DIR"
        read -r ANSWER
        case "$ANSWER" in
            [yY] | [yY][eE][sS]) ;;
            *) exit 1 ;;
        esac
    fi
    BUILD_DIR=$CACHED_BUILD_DIR
fi

# Default value if no cmdline or cached builddir
if [ -z "$BUILD_DIR" ]; then
    BUILD_DIR=build
fi

# Write variables that need to persist between runs
cachewrite

if [ "$HELP" = 'yes' ]; then
    help
    exit 1
fi

if [ "$VERBOSE" = 'yes' ]; then
    set -x
fi

mkdir "$BUILD_DIR"

build
