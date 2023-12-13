#!/bin/sh

# Autoconf aux and m4 will always go in <project-root>/build
AUX_DIR="build/autotools/aux"
M4_DIR="build/autotools/m4"
PROJECT_ROOT=${0%autogen.sh}
BUILD_DIR=build

help() {
    cat <<eof
usage: autogen.sh [-h|OPTION]... [builddir]
Build uniTesTap

positional arguments:
  builddir                   The root of the build tree. (default: <project-root>/build)

options:
  -h, --help                 show this help message and exit
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

build() (
    # All build steps must be successful
    set -xe

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

    autoreconf -ivf
    cd "$BUILD_DIR"
    "$ABS_PROJECT_ROOT"/configure
    make
)

if ! argparse "$@"; then
    exit 1
fi

mkdir -p "$BUILD_DIR"

if [ "$HELP" = 'yes' ]; then
    help
    exit 1
fi

build
