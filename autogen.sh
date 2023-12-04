#!/bin/sh

BUILD_DIR=build
AUX_DIR="$BUILD_DIR/autotools/aux"
M4_DIR="$BUILD_DIR/autotools/m4"

set -xe

(
    # Must be run from source directory
    cd "${0%autogen.sh}"

    mkdir -p "$AUX_DIR" "$M4_DIR"

    # Go looking for the tap-driver if it is missing
    if ! [ -e "$AUX_DIR/tap-driver.sh" ]; then
        if ! AUTOMAKE_LIBDIR=$(automake --print-libdir); then
            printf 'automake error: cannot find tap-driver.sh' >&2
        fi
        cp "$AUTOMAKE_LIBDIR/tap-driver.sh" "$AUX_DIR"
    fi

    autoreconf -ivf

    cd build
    ../configure
    make
)
