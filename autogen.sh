#!/bin/sh

BUILD_DIR=build
AUX_DIR="$BUILD_DIR/autotools/aux"
M4_DIR="$BUILD_DIR/autotools/m4"

set -xe

(
    # Must be run from source directory
    cd "${0%autogen.sh}"

    mkdir -p "$AUX_DIR" "$M4_DIR"

    autoreconf -ivf

    cd build
    ../configure
    make
)
