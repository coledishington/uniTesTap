#!/bin/sh

set -xe

# Must be run from source directory
cd "${0%autogen.sh}"

mkdir -p build/autotools/aux build/autotools/m4

autoreconf -ivf

(
    cd build
    ../configure
    make
)
